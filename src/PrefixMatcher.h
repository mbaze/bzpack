// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef PREFIX_MATCHER_H
#define PREFIX_MATCHER_H

#include <vector>
#include "CommonTypes.h"

class PrefixMatcher
{
public:

    PrefixMatcher() = delete;

    PrefixMatcher(
        const uint8_t* pInput,
        uint16_t inputSize,
        uint16_t minMatchLength,
        uint16_t maxMatchLength,
        uint16_t maxMatchOffset
    );

    std::vector<Match> FindMatches(uint16_t inputPos, bool allowBytes = false) const;
    Match FindLongestMatch(uint16_t inputPos) const;

private:

    uint16_t GetMatchLength(uint16_t inputPos, uint16_t matchPos) const;

    const uint8_t* mInputPtr;
    const uint16_t mInputSize;

    const uint16_t mMinMatchLength;
    const uint16_t mMaxMatchLength;
    const uint16_t mMaxMatchOffset;

    std::vector<std::vector<uint16_t>> mBytePositions;
    std::vector<std::vector<Match>> mLongestMatches;
};

#endif // PREFIX_MATCHER_H
