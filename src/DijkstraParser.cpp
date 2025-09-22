// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "DijkstraParser.h"
#include "MultiHeap.h"

std::vector<ParseStep> DijkstraParser::Parse()
{
    uint32_t bestPathIndex = 0;
    uint32_t greedyParseCost = ComputeGreedyParseCost();

    std::vector<Match> matches;
    std::vector<uint32_t> posCosts(mInputSize, 0xFFFFFFFF);

    BlockVector<PathNode> nodes;
    nodes.push_back(PathNode{0, 0, 0});

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
        bool isMatch = node.IsMatch();
        bool isLiteral = !isMatch;

        if (inputPos >= mInputSize)
        {
            // End of input data reached, exit loop.
            bestPathIndex = nodeIndex;
            break;
        }

        // Find matches after a literal or if this path improves position cost.

        matches.clear();
        size_t byteMatchCount = 0;

        if (isLiteral || cost < posCosts[inputPos])
        {
            byteMatchCount = mMatcher.FindMatches(inputPos, isLiteral, matches);
        }

        posCosts[inputPos] = std::min(posCosts[inputPos], cost);

        // Consider all repeat matches (only after a literal).

        if (isLiteral)
        {
            for (const Match& match: matches)
            {
                if (match.offset != offsetOrRep)
                    continue;

                uint16_t newPos = inputPos + match.length;
                uint32_t newCost = cost + mFormat.GetRepMatchCost(match.length);

                if (ShouldEnqueue(newPos, match.offset, newCost, greedyParseCost))
                {
                    heap.Push(SortableCostIndex(newCost, nodes.size()));
                    nodes.push_back(PathNode{newPos, match.offset, nodeIndex});
                }
            }
        }

        // Consider all matches if not already explored by a previous path.

        for (size_t i = byteMatchCount; i < matches.size(); i++)
        {
            const Match& match = matches[i];

            uint16_t newPos = inputPos + match.length;
            uint32_t newCost = cost + mFormat.GetMatchCost(match.length, match.offset);

            if (isMatch && match.offset == offsetOrRep)
            {
                // Don't enqueue if propagating a literal would be cheaper (very rare).
                if (cost + mFormat.GetLiteralCost(match.length) < newCost)
                    continue;
            }

            if (ShouldEnqueue(newPos, match.offset, newCost, greedyParseCost))
            {
                heap.Push(SortableCostIndex(newCost, nodes.size()));
                nodes.push_back(PathNode{newPos, match.offset, nodeIndex});
            }
        }

        // Consider all available literals (only after a match).

        if (isMatch)
        {
            uint16_t maxLength = std::min<uint16_t>(mInputSize - inputPos, mFormat.MaxLiteralLength());

            for (uint16_t length = 1; length <= maxLength; length++)
            {
                uint16_t newPos = inputPos + length;
                uint32_t newCost = cost + mFormat.GetLiteralCost(length);

                if (ShouldEnqueue(newPos, offsetOrRep, newCost, greedyParseCost))
                {
                    heap.Push(SortableCostIndex(newCost, nodes.size()));
                    nodes.push_back(PathNode{offsetOrRep, newPos, nodeIndex});
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

inline bool DijkstraParser::ShouldEnqueue(uint16_t inputPos, uint16_t repOffset, uint32_t cost, uint32_t greedyParseCost)
{
    if (cost >= mPosRepCosts.Get(inputPos, repOffset) || cost > greedyParseCost)
        return false;

    mPosRepCosts.Set(inputPos, repOffset, cost);
    return true;
}

uint32_t DijkstraParser::ComputeGreedyParseCost()
{
    uint32_t cost = 0;
    uint16_t inputPos = 0;
    uint16_t literalLength = 0;

    while (inputPos < mInputSize)
    {
        Match match = mMatcher.FindLongestMatch(inputPos);
        if (match.offset)
        {
            if (literalLength)
            {
                cost += mFormat.GetLiteralCost(literalLength);
                literalLength = 0;
            }

            cost += mFormat.GetMatchCost(match.length, match.offset);
            inputPos += match.length;
        }
        else
        {
            literalLength++;
            inputPos++;        
        }
    }

    if (literalLength)
    {
        cost += mFormat.GetLiteralCost(literalLength);
    }

    return cost;
}
