// Copyright (c) 2024, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "MatchFinder.h"

MatchFinder::MatchFinder(const uint8_t* pInput, uint16_t inputSize, const FormatLimits& limits)
{
    mInputPtr = pInput;
    mInputSize = inputSize;

    mMinMatchLength = limits.minMatchLength;
    mMaxMatchLength = limits.maxMatchLength;
    mMaxMatchOffset = limits.maxMatchOffset;

    // 0xFFFF is the end marker since no match can begin at the last byte.

    std::fill(std::begin(mHash), std::end(mHash), 0xFFFF);
    std::fill(std::begin(mList), std::end(mList), 0xFFFF);
}

inline uint16_t MatchFinder::GetMatchLength(uint16_t inputPos, uint16_t matchPos) const
{
    // We already know that the first two bytes match.

    uint16_t endPos = inputPos + std::min<uint16_t>(mInputSize - inputPos, mMaxMatchLength);
    uint16_t matchLength = 2;
    inputPos += 2;
    matchPos += 2;

    while (inputPos < endPos)
    {
        if (mInputPtr[inputPos] != mInputPtr[matchPos])
            break;

        matchLength++;
        inputPos++;
        matchPos++;
    }

    return matchLength;
}

void MatchFinder::FindMatches(std::vector<Match>& matches, uint16_t inputPos)
{
    uint16_t windowPos = inputPos - std::min<uint16_t>(inputPos, mMaxMatchOffset);
    uint16_t key = mInputPtr[inputPos] | (mInputPtr[inputPos + 1] << 8);
    uint16_t matchPos = mHash[key];

    while (matchPos != 0xFFFF && matchPos >= windowPos)
    {
        uint16_t matchLength = GetMatchLength(inputPos, matchPos);

        for (uint16_t length = mMinMatchLength; length <= matchLength; length++)
        {
            matches.emplace_back(inputPos - matchPos, length);
        }

        matchPos = mList[matchPos];
    }

    mList[inputPos] = mHash[key];
    mHash[key] = inputPos;
}
