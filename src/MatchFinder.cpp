// Copyright (c) 2024, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "MatchFinder.h"

MatchFinder::MatchFinder(const uint8_t* pInput, uint16_t inputSize, const FormatLimits& limits)
{
    mInputPtr = pInput;
    mInputSize = inputSize;
    mWindowSize = limits.maxMatchOffset;
    mMinMatchLength = limits.minMatchLength;
    mMaxMatchLength = limits.maxMatchLength;

    for (size_t i = 0; i < 65536; i++)
    {
        mHash[i] = -mWindowSize;
        mList[i] = -mWindowSize;
    }
}

uint16_t MatchFinder::GetMatchLength(uint16_t inputPos, uint16_t matchPos) const
{
    // We already know that the first two bytes match.

    uint16_t endPos = inputPos + std::min<uint16_t>(mInputSize - inputPos, mMaxMatchLength);
    inputPos += 2;
    matchPos += 2;
    uint16_t matchLength = 2;

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
    uint16_t key = mInputPtr[inputPos] | (mInputPtr[inputPos + 1] << 8);
    int32_t matchPos = mHash[key];
    int32_t minMatchPos = inputPos - mWindowSize;

    while (matchPos > minMatchPos)
    {
        uint16_t matchLength = GetMatchLength(inputPos, matchPos);

        for (uint16_t length = mMinMatchLength; length <= matchLength; length++)
        {
            Match match;
            match.offset = inputPos - matchPos;
            match.length = length;
            matches.push_back(match);
        }

        matchPos = mList[matchPos];
    }

    mList[inputPos] = mHash[key];
    mHash[key] = inputPos;
}
