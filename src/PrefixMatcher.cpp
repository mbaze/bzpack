// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "PrefixMatcher.h"

PrefixMatcher::PrefixMatcher(const uint8_t* pInput, uint32_t inputSize, uint16_t minMatchLength, uint16_t maxMatchLength, uint16_t maxMatchOffset):
    mInputPtr{pInput},
    mInputSize{inputSize},
    mMinMatchLength{minMatchLength},
    mMaxMatchLength{maxMatchLength},
    mMaxMatchOffset{maxMatchOffset},
    mByteMatches{inputSize},
    mMaxMatches{inputSize}
{
    if (inputSize < 2)
        return;

    // Gather byte positions and record matches of length 1 within the offset window.

    std::vector<std::vector<uint32_t>> bytePositions(256);

    for (uint32_t inputPos = 1; inputPos < inputSize; inputPos++)
    {
        uint8_t byte = pInput[inputPos];
        uint32_t windowPos = inputPos - std::min<uint32_t>(inputPos, maxMatchOffset);

        for (auto i = bytePositions[byte].rbegin(); i != bytePositions[byte].rend(); i++)
        {
            if (*i < windowPos)
                break;

            mByteMatches[inputPos].emplace_back(*i);
        }

        bytePositions[byte].emplace_back(inputPos);
    }

    // Gather 2-byte word positions and record maximum match lengths within the offset window.

    std::vector<std::vector<uint32_t>> wordPositions(65536);

    for (uint32_t inputPos = 1; inputPos < inputSize - 1; inputPos++)
    {
        uint16_t word = pInput[inputPos] | (pInput[inputPos + 1] << 8);
        uint32_t windowPos = inputPos - std::min<uint32_t>(inputPos, maxMatchOffset);

        for (auto i = wordPositions[word].rbegin(); i != wordPositions[word].rend(); i++)
        {
            if (*i < windowPos)
                break;

            uint16_t matchLength = GetMatchLength(inputPos, *i);

            if (matchLength >= mMinMatchLength)
            {
                mMaxMatches[inputPos].emplace_back(*i, matchLength);
            }
        }

        wordPositions[word].emplace_back(inputPos);
    }
}

size_t PrefixMatcher::FindMatches(std::vector<Match>& matches, uint32_t inputPos, bool allowBytes) const
{
    matches.clear();

    // Single-byte matches are cheap to encode and can establish useful repeat offsets.

    if (allowBytes)
    {
        for (uint32_t bytePos: mByteMatches[inputPos])
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

uint16_t PrefixMatcher::GetMatchLength(uint32_t inputPos, uint32_t matchPos) const
{
    uint32_t maxLength = std::min<uint32_t>(mInputSize - inputPos, mMaxMatchLength) - 2;
    uint16_t length = 2;
    inputPos += 2;
    matchPos += 2;
    
    while (maxLength-- && mInputPtr[inputPos++] == mInputPtr[matchPos++])
    {
        length++;
    }
    
    return length;
}
