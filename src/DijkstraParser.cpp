// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "DijkstraParser.h"
#include "MultiHeap.h"

#define COST_INDEX_64(cost, index) \
    ((static_cast<uint64_t>(cost) << 32) | static_cast<uint64_t>(index))

std::vector<ParseStep> DijkstraParser::Parse()
{
    uint32_t bestPathIndex = 0;
    std::vector<uint32_t> posCosts(mInputSize, 0xFFFFFFFF);

    BlockVector<PathNode> nodes;
    nodes.emplace_back(0, 0, 0, true);

    MultiHeap<uint64_t> heap;
    heap.Push(0);

    while (heap.Size())
    {
        uint64_t heapTop = heap.Pop();
        uint32_t cost = static_cast<uint32_t>(heapTop >> 32);
        uint32_t nodeIndex = static_cast<uint32_t>(heapTop);

        PathNode node = nodes[nodeIndex];
        uint16_t inputPos = node.GetInputPos();
        uint16_t offsetOrRep = node.GetOffsetOrRep();

        if (inputPos >= mInputSize)
        {
            // End of input data reached, exit loop.
            bestPathIndex = nodeIndex;
            break;
        }

        // Consider all repeat matches (only after a literal).

        if (node.IsLiteral())
        {
            for (const Match& match: mMatcher.FindMatches(inputPos, true))
            {
                if (match.offset != offsetOrRep)
                    continue;

                uint32_t newPos = inputPos + match.length;
                uint32_t newCost = cost + mFormat.GetRepMatchCost(match.length);

                if (ShouldEnqueue(newPos, offsetOrRep, newCost))
                {
                    heap.Push(COST_INDEX_64(newCost, nodes.size()));
                    nodes.emplace_back(nodeIndex, newPos, match.offset, true);
                }
            }
        }

        // Consider all available matches (if not already explored by a previous path).

        if (cost < posCosts[inputPos])
        {
            posCosts[inputPos] = cost;

            for (const Match& match: mMatcher.FindMatches(inputPos))
            {
                uint32_t newPos = inputPos + match.length;
                uint32_t newCost = cost + mFormat.GetMatchCost(match.length, match.offset);

                if (ShouldEnqueue(newPos, match.offset, newCost))
                {
                    heap.Push(COST_INDEX_64(newCost, nodes.size()));
                    nodes.emplace_back(nodeIndex, newPos, match.offset, true);
                }
            }
        }

        // Consider all available literals (only after a match).

        if (node.IsMatch())
        {
            uint16_t maxLength = std::min<uint16_t>(mInputSize - inputPos, mFormat.MaxLiteralLength());

            for (uint16_t length = 1; length <= maxLength; length++)
            {
                uint32_t newPos = inputPos + length;
                uint32_t newCost = cost + mFormat.GetLiteralCost(length);

                if (ShouldEnqueue(newPos, offsetOrRep, newCost))
                {
                    heap.Push(COST_INDEX_64(newCost, nodes.size()));
                    nodes.emplace_back(nodeIndex, newPos, offsetOrRep, false);
                }
            }
        }
    }

    // Backtrack and reconstruct the optimal parse sequence.

    std::vector<ParseStep> parse;

    while (bestPathIndex)
    {
        const PathNode& node = nodes[bestPathIndex];
        const PathNode& parentNode = nodes[node.parent];

        uint16_t length = node.GetInputPos() - parentNode.GetInputPos();
        uint16_t offset = node.IsMatch() ? node.GetOffsetOrRep() : 0;
        parse.emplace_back(length, offset);

        bestPathIndex = node.parent;
    }

    std::reverse(parse.begin(), parse.end());

    return parse;
}

inline bool DijkstraParser::ShouldEnqueue(uint16_t inputPos, uint16_t repOffset, uint32_t cost)
{
#ifdef USE_COST_TABLE

    if (cost < mPosRepCosts.Get(inputPos, repOffset))
    {
        mPosRepCosts.Set(inputPos, repOffset, cost);
        return true;
    }

    return false;

#else

    uint32_t key = (repOffset << 16) | inputPos;
    auto iHash = mPosRepCosts.find(key);

    if (iHash == mPosRepCosts.end())
    {
        mPosRepCosts[key] = cost;
        return true;
    }
    else if (cost < iHash->second)
    {
        iHash->second = cost;
        return true;
    }

    return false;

#endif
}
