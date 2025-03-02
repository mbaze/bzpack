// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <queue>
#include "DijkstraParser.h"

#define COST_INDEX_64(cost, index) \
    ((static_cast<uint64_t>(cost) << 32) | static_cast<uint64_t>(index))

bool DijkstraParser::ShouldEnqueue(uint16_t inputPos, uint16_t repOffset, uint32_t cost)
{
    uint32_t key = (repOffset << 16) | inputPos;

    auto result = mPosRepCosts.try_emplace(key, cost);
    if (result.second)
        return true;

    // Prune if a new arrival to the same state doesn't reduce the cost.

    if (cost >= result.first->second)
        return false;

    result.first->second = cost;

    return true;
}

std::vector<ParseStep> DijkstraParser::Parse()
{
    uint32_t bestPathIndex = 0;
    std::vector<uint32_t> posCosts(mInputSize, 0xFFFFFFFF);

    std::vector<PathNode> nodes;
    nodes.emplace_back(0, 0, 0, 0xFFFFFFFF, 0);

    std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> queue;
    queue.emplace(0);

    while (!queue.empty())
    {
        uint64_t queueTop = queue.top();
        uint32_t nodeIndex = static_cast<uint32_t>(queueTop);
        uint32_t cost = static_cast<uint32_t>(queueTop >> 32);
        queue.pop();

        PathNode node = nodes[nodeIndex];

        if (node.inputPos >= mInputSize)
        {
            // End of input data reached, exit loop.
            bestPathIndex = nodeIndex;
            break;
        }

        // Consider all available matches (if not already explored by a previous path).

        if (cost < posCosts[node.inputPos])
        {
            posCosts[node.inputPos] = cost;

            for (const Match& match: mMatcher.FindMatches(node.inputPos))
            {
                uint32_t newPos = node.inputPos + match.length;
                uint32_t newCost = cost + mFormat.GetMatchCost(match.length, match.offset);

                if (ShouldEnqueue(newPos, match.offset, newCost))
                {
                    queue.emplace(COST_INDEX_64(newCost, nodes.size()));
                    nodes.emplace_back(nodeIndex, newPos, match.length, match.offset, match.offset);
                }
            }
        }

        // Consider all repeat matches (only after a literal).

        if (node.offset == 0)
        {
            for (const Match& match: mMatcher.FindMatches(node.inputPos, true))
            {
                if (match.offset != node.repOffset)
                    continue;

                uint32_t newPos = node.inputPos + match.length;
                uint32_t newCost = cost + mFormat.GetMatchCost(match.length, match.offset, node.repOffset);

                if (ShouldEnqueue(newPos, node.repOffset, newCost))
                {
                    queue.emplace(COST_INDEX_64(newCost, nodes.size()));
                    nodes.emplace_back(nodeIndex, newPos, match.length, match.offset, match.offset);
                }
            }
        }

        // Consider all available literals (only after a match).

        if (node.offset)
        {
            uint16_t maxLength = std::min<uint16_t>(mInputSize - node.inputPos, mFormat.MaxLiteralLength());

            for (uint16_t length = 1; length <= maxLength; length++)
            {
                uint32_t newPos = node.inputPos + length;
                uint32_t newCost = cost + mFormat.GetLiteralCost(length);

                if (ShouldEnqueue(newPos, node.repOffset, newCost))
                {
                    queue.emplace(COST_INDEX_64(newCost, nodes.size()));
                    nodes.emplace_back(nodeIndex, newPos, length, 0, node.repOffset);
                }
            }
        }
    }

    // Backtrack and reconstruct the optimal parse sequence.

    std::vector<ParseStep> parse;

    while (bestPathIndex)
    {
        parse.emplace_back(nodes[bestPathIndex].length, nodes[bestPathIndex].offset);
        bestPathIndex = nodes[bestPathIndex].parent;
    }

    std::reverse(parse.begin(), parse.end());

    return parse;
}
