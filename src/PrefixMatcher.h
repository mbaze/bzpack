// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef PREFIX_MATCHER_H
#define PREFIX_MATCHER_H

#include <vector>
#include "CommonTypes.h"

class PrefixMatcher
{
    struct MaxMatch
    {
        MaxMatch(uint32_t inputPos, uint16_t length = 0):
            inputPos{inputPos}, length{length}
        {}

        uint32_t inputPos;
        uint16_t length;
    };

public:

    PrefixMatcher() = delete;

    PrefixMatcher(
        const uint8_t* pInput,
        uint32_t inputSize,
        uint16_t minMatchLength,
        uint16_t maxMatchLength,
        uint16_t maxMatchOffset
    );

    size_t FindMatches(uint32_t inputPos, bool allowBytes, std::vector<Match>& matches) const;
    Match FindLongestMatch(uint32_t inputPos) const;

private:

    uint16_t GetMatchLength(uint32_t inputPos, uint32_t matchPos) const;

    const uint8_t* mInputPtr;
    const uint32_t mInputSize;

    const uint16_t mMinMatchLength;
    const uint16_t mMaxMatchLength;
    const uint16_t mMaxMatchOffset;

    std::vector<std::vector<uint32_t>> mBytePositions;
    std::vector<std::vector<MaxMatch>> mMaxMatches;
};

#endif // PREFIX_MATCHER_H
