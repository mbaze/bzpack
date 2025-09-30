// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "PrefixMatcher.h"
#include <algorithm>

PrefixMatcher::PrefixMatcher(const uint8_t* pInput, uint32_t inputSize, uint16_t minMatchLength, uint16_t maxMatchLength, uint16_t maxMatchOffset):
    mInputPtr{pInput},
    mInputSize{inputSize},
    mMinMatchLength{minMatchLength},
    mMaxMatchLength{maxMatchLength},
    mMaxMatchOffset{maxMatchOffset}
{
    // Initialize lists that track past positions of single bytes and 2-byte sequences in the input.

    std::vector<std::vector<uint32_t>> byteOccurrences(256);
    mBytePositions.resize(inputSize);

    for (uint32_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        uint8_t byte = pInput[inputPos];
        uint32_t windowPos = inputPos - std::min<uint32_t>(inputPos, maxMatchOffset);

        for (auto i = byteOccurrences[byte].rbegin(); i != byteOccurrences[byte].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mBytePositions[inputPos].emplace_back(*i);
        }

        byteOccurrences[byte].emplace_back(inputPos);
    }

    std::vector<std::vector<uint32_t>> wordOccurrences(65536);
    mMaxMatches.resize(inputSize);

    for (uint32_t inputPos = 0; inputPos < inputSize - 1; inputPos++)
    {
        uint16_t word = (pInput[inputPos + 1] << 8) | pInput[inputPos];
        uint32_t windowPos = inputPos - std::min<uint32_t>(inputPos, maxMatchOffset);

        for (auto i = wordOccurrences[word].rbegin(); i != wordOccurrences[word].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mMaxMatches[inputPos].emplace_back(*i);
        }

        wordOccurrences[word].emplace_back(inputPos);
    }

    // Calculate the longest available match length at each position.

    for (uint32_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        for (MaxMatch& maxMatch: mMaxMatches[inputPos])
        {
            maxMatch.length = GetMatchLength(inputPos, maxMatch.inputPos);
        }
    }
}

size_t PrefixMatcher::FindMatches(std::vector<Match>& matches, uint32_t inputPos, bool allowBytes) const
{
    matches.clear();

    // Single-byte matches are cheap to encode and can establish useful repeat offsets.

    if (allowBytes)
    {
        for (uint32_t bytePos: mBytePositions[inputPos])
        {
            matches.emplace_back(1, inputPos - bytePos);
        }
    }

    size_t byteMatchCount = matches.size();

    for (const MaxMatch& maxMatch: mMaxMatches[inputPos])
    {
        uint16_t offset = inputPos - maxMatch.inputPos;

        for (uint16_t length = mMinMatchLength; length <= maxMatch.length; length++)
        {
            matches.emplace_back(length, offset);
        }
    }

    return byteMatchCount;
}

Match PrefixMatcher::FindLongestMatch(uint32_t inputPos) const
{
    uint16_t maxLength = mMinMatchLength - 1;
    uint16_t maxOffset = 0;

    for (const MaxMatch& maxMatch: mMaxMatches[inputPos])
    {
        if (maxMatch.length > maxLength)
        {
            maxLength = maxMatch.length;
            maxOffset = inputPos - maxMatch.inputPos;
        }
    }

    return {maxLength, maxOffset};
}

uint16_t PrefixMatcher::GetMatchLength(uint32_t inputPos, uint32_t matchPos) const
{
    const uint8_t* pInput = mInputPtr + inputPos;
    const uint8_t* pMatch = mInputPtr + matchPos;
    const uint8_t* pEnd = pInput + std::min<uint32_t>(mInputSize - inputPos, mMaxMatchLength);

    return static_cast<uint16_t>(std::mismatch(pInput, pEnd, pMatch).first - pInput);
}
