// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "PrefixMatcher.h"
#include <algorithm>

PrefixMatcher::PrefixMatcher(const uint8_t* pInput, uint16_t inputSize, uint16_t minMatchLength, uint16_t maxMatchLength, uint16_t maxMatchOffset):
    mInputPtr{pInput},
    mInputSize{inputSize},
    mMinMatchLength{minMatchLength},
    mMaxMatchLength{maxMatchLength},
    mMaxMatchOffset{maxMatchOffset}
{
    // Initialize lists that track past positions of single bytes and 2-byte sequences in the input.

    std::vector<std::vector<uint16_t>> byteOccurrences(256);
    mBytePositions.resize(inputSize);

    for (uint16_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        uint8_t byte = pInput[inputPos];
        uint16_t windowPos = inputPos - std::min(inputPos, maxMatchOffset);

        for (auto i = byteOccurrences[byte].rbegin(); i != byteOccurrences[byte].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mBytePositions[inputPos].emplace_back(*i);
        }

        byteOccurrences[byte].emplace_back(inputPos);
    }

    std::vector<std::vector<uint16_t>> matchOccurrences(65536);
    mLongestMatches.resize(inputSize);

    for (uint16_t inputPos = 0; inputPos < inputSize - 1; inputPos++)
    {
        uint16_t match = (pInput[inputPos + 1] << 8) | pInput[inputPos];
        uint16_t windowPos = inputPos - std::min(inputPos, maxMatchOffset);

        for (auto i = matchOccurrences[match].rbegin(); i != matchOccurrences[match].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mLongestMatches[inputPos].emplace_back(0, *i);
        }

        matchOccurrences[match].emplace_back(inputPos);
    }

    // Calculate the longest available match length at each position.

    for (uint16_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        for (Match& match: mLongestMatches[inputPos])
        {
            match.length = GetMatchLength(inputPos, match.offset);
        }
    }
}

size_t PrefixMatcher::FindMatches(uint16_t inputPos, bool allowBytes, std::vector<Match>& matches) const
{
    // Single-byte matches help set up useful repeat offsets.

    if (allowBytes)
    {
        for (uint16_t bytePos: mBytePositions[inputPos])
        {
            matches.emplace_back(1, inputPos - bytePos);
        }
    }

    size_t byteMatchCount = matches.size();

    // In this case, the precomputed match.offset is the absolute input position.

    for (const Match& match: mLongestMatches[inputPos])
    {
        uint16_t offset = inputPos - match.offset;

        for (uint16_t length = mMinMatchLength; length <= match.length; length++)
        {
            matches.emplace_back(length, offset);
        }
    }

    return byteMatchCount;
}

Match PrefixMatcher::FindLongestMatch(uint16_t inputPos) const
{
    uint16_t maxLength = mMinMatchLength - 1;
    uint16_t maxOffset = 0;

    for (const Match& match: mLongestMatches[inputPos])
    {
        if (match.length > maxLength)
        {
            maxLength = match.length;
            maxOffset = inputPos - match.offset;
        }
    }

    return {maxLength, maxOffset};
}

uint16_t PrefixMatcher::GetMatchLength(uint16_t inputPos, uint16_t matchPos) const
{
    const uint8_t* pInput = mInputPtr + inputPos;
    const uint8_t* pMatch = mInputPtr + matchPos;
    const uint8_t* pEnd = pInput + std::min<uint16_t>(mInputSize - inputPos, mMaxMatchLength);

    return static_cast<uint16_t>(std::mismatch(pInput, pEnd, pMatch).first - pInput);
}
