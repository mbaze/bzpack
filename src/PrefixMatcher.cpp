// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <cassert>
#include <algorithm>
#include "PrefixMatcher.h"

PrefixMatcher::PrefixMatcher(const uint8_t* pInput, uint16_t inputSize, uint16_t minMatchLength, uint16_t maxMatchLength, uint16_t maxMatchOffset):
    mInputPtr{pInput},
    mInputSize{inputSize},
    mMinMatchLength{minMatchLength},
    mMaxMatchLength{maxMatchLength},
    mMaxMatchOffset{maxMatchOffset}
{
    // Initialize lists that track past positions of single bytes and 2-byte sequences in the input.

    std::vector<std::vector<uint16_t>> byteOccurrences(256);
    std::vector<std::vector<uint16_t>> matchOccurrences(65536);

    mBytePositions = std::make_unique<std::vector<uint16_t>[]>(inputSize);
    mMatchPositions = std::make_unique<std::vector<uint16_t>[]>(inputSize);

    for (uint16_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        uint8_t byte = pInput[inputPos];
        uint16_t match = (pInput[inputPos + 1] << 8) | byte;
        uint16_t windowPos = inputPos - std::min<uint16_t>(inputPos, maxMatchOffset);

        for (auto i = byteOccurrences[byte].rbegin(); i != byteOccurrences[byte].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mBytePositions[inputPos].emplace_back(*i);
        }

        for (auto i = matchOccurrences[match].rbegin(); i != matchOccurrences[match].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mMatchPositions[inputPos].emplace_back(*i);
        }

        byteOccurrences[byte].emplace_back(inputPos);
        matchOccurrences[match].emplace_back(inputPos);
    }
}

std::vector<Match> PrefixMatcher::FindMatches(uint16_t inputPos, bool allowBytes) const
{
    std::vector<Match> matches;

    if (allowBytes)
    {
        // Single-byte matches help set up useful repeat offsets.

        for (uint16_t bytePos: mBytePositions[inputPos])
        {
            matches.emplace_back(1, inputPos - bytePos);
        }
    }

    for (uint16_t matchPos: mMatchPositions[inputPos])
    {
        uint16_t matchLength = GetMatchLength(inputPos, matchPos);
        for (uint16_t length = mMinMatchLength; length <= matchLength; length++)
        {
            matches.emplace_back(length, inputPos - matchPos);
        }
    }

    return matches;
}

uint16_t PrefixMatcher::GetMatchLength(uint16_t inputPos, uint16_t matchPos) const
{
    const uint8_t* pInput = mInputPtr + inputPos;
    const uint8_t* pMatch = mInputPtr + matchPos;
    const uint8_t* pEnd = pInput + std::min<uint16_t>(mInputSize - inputPos, mMaxMatchLength);

    return static_cast<uint16_t>(std::mismatch(pInput, pEnd, pMatch).first - pInput);
}
