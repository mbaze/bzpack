// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef DIJKSTRA_PARSER_H
#define DIJKSTRA_PARSER_H

#define USE_COST_TABLE

#include "CostTable.h"
#include "Formats.h"
#include "PrefixMatcher.h"

#ifndef USE_COST_TABLE
#include <unordered_map>
#endif

class DijkstraParser
{
    // Each node stores only two fields (besides the parent index) to save memory.
    // Matches (param1 >= param2): param1 = input position, param2 = offset.
    // Literals (param1 < param2): param1 = repeat offset, param2 = input position.

    struct PathNode
    {
        PathNode() = default;

        PathNode(uint32_t parent, uint16_t inputPos, uint16_t offsetOrRep, bool isMatch):
            parent{parent},
            param1{isMatch ? inputPos : offsetOrRep},
            param2{isMatch ? offsetOrRep : inputPos}
        {
        }

        bool IsMatch() const { return param1 >= param2; }
        bool IsLiteral() const { return param1 < param2; }

        uint16_t GetInputPos() const { return IsMatch() ? param1 : param2; }
        uint16_t GetOffsetOrRep() const { return IsMatch() ? param2 : param1; }

        uint32_t parent;
        uint16_t param1;
        uint16_t param2;
    };

    static constexpr uint64_t SortableCostIndex(uint32_t cost, size_t index)
    {
        return (static_cast<uint64_t>(cost) << 32) | index;
    }

public:

    DijkstraParser() = delete;

    DijkstraParser(const uint8_t* pInput, uint16_t inputSize, const Format& format):
#ifdef USE_COST_TABLE
        mPosRepCosts{inputSize, mFormat.MaxMatchOffset()},
#endif // USE_COST_TABLE
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
    {
    }

    std::vector<ParseStep> Parse();

private:

    bool ShouldEnqueue(uint16_t inputPos, uint16_t repOffset, uint32_t cost);

    const uint8_t* mInputPtr;
    const uint16_t mInputSize;
    const Format& mFormat;
    PrefixMatcher mMatcher;

#ifdef USE_COST_TABLE

    CostTable mPosRepCosts;

#else

    struct Hash
    {
        size_t operator () (uint32_t key) const { return (key >> 8) ^ key; }
    };

    std::unordered_map<uint32_t, uint32_t, Hash> mPosRepCosts;

#endif 
};

#endif // DIJKSTRA_PARSER_H
