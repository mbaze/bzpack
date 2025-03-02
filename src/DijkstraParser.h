// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef DIJKSTRA_PARSER_H
#define DIJKSTRA_PARSER_H

#include <unordered_map>
#include "Formats.h"
#include "PrefixMatcher.h"

class DijkstraParser
{
public:

    DijkstraParser() = delete;

    DijkstraParser(const uint8_t* pInput, uint16_t inputSize, const Format& format):
        mInputPtr{pInput},
        mInputSize{inputSize},
        mFormat{format},
        mMatcher{
            pInput,
            inputSize,
            format.MinMatchLength(),
            format.MaxMatchLength(),
            format.MaxMatchOffset()
        }
    {}

    std::vector<ParseStep> Parse();

private:

    bool ShouldEnqueue(uint16_t inputPos, uint16_t repOffset, uint32_t cost);

    const uint8_t* mInputPtr;
    const uint16_t mInputSize;
    const Format& mFormat;

    PrefixMatcher mMatcher;
    std::unordered_map<uint32_t, uint32_t> mPosRepCosts;

    struct PathNode
    {
        PathNode(uint32_t parent, uint16_t inputPos, uint16_t length, uint16_t offset, uint16_t repOffset):
            parent{parent}, inputPos{inputPos}, length{length}, offset{offset}, repOffset{repOffset} {}

        uint32_t parent;
        uint16_t inputPos;
        uint16_t length;
        uint16_t offset;
        uint16_t repOffset;
    };
};

#endif // DIJKSTRA_PARSER_H
