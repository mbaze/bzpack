// Copyright (c) 2024, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef MATCH_FINDER_H
#define MATCH_FINDER_H

#include "Formats.h"

struct Match
{
    Match(uint16_t o, uint16_t l): offset(o), length(l) {}

    uint16_t offset;
    uint16_t length;
};

class MatchFinder
{
public:

    MatchFinder() = delete;
    MatchFinder(const uint8_t* pInput, uint16_t inputSize, const FormatLimits& limits);
    void FindMatches(std::vector<Match>& matches, uint16_t inputPos);

private:

    uint16_t GetMatchLength(uint16_t inputPos, uint16_t matchPos) const;

    const uint8_t* mInputPtr;
    uint16_t mInputSize;

    uint16_t mMinMatchLength;
    uint16_t mMaxMatchLength;
    uint16_t mMaxMatchOffset;

    uint16_t mHash[65536];
    uint16_t mList[65536];
};

#endif // MATCH_FINDER_H
