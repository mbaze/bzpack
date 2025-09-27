// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "ExhaustiveParser.h"
#include "PrefixMatcher.h"

std::vector<ParseStep> ExhaustiveParser::Parse(const uint8_t* pInput, uint32_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    // Precompute matches.

    std::vector<Match> matches;
    PrefixMatcher matcher(pInput, inputSize, format.MinMatchLength(), format.MaxMatchLength(), format.MaxMatchOffset());

    // Allocate triangular DP table and corresponding row pointers.

    std::vector<PathNode*> rowPointers(inputSize + 1);
    size_t nodeCount = 0;

    for (uint32_t i = 0; i <= inputSize; i++)
    {
        nodeCount += GetRowWidth(i, format.MaxMatchOffset());
    }

    std::vector<PathNode> nodes(nodeCount);
    PathNode* p = nodes.data();

    for (uint32_t i = 0; i <= inputSize; i++)
    {
        rowPointers[i] = p;
        p += GetRowWidth(i, format.MaxMatchOffset());
    }

    // Kickstart the main loop and sweep over all coding paths.

    nodes[0] = PathNode{0, 1, 0};

    for (uint32_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        matches.clear();
        size_t regularMatchOffset = matcher.FindMatches(inputPos, true, matches);

        PathNode* rowPtr = rowPointers[inputPos];
        uint32_t rowWidth = GetRowWidth(inputPos, format.MaxMatchOffset());

        for (uint16_t repState = 0; repState < rowWidth; repState++)
        {
            const PathNode& node = rowPtr[repState];

            // Skip unreachable states.
            if (node.cost == 0xFFFFFFFF)
                continue;

            bool isMatch = node.matchLength;
            bool isLiteral = !isMatch;

            // Propagate repeat matches (only after a literal).

            if (isLiteral)
            {
                for (const Match& match: matches)
                {
                    if (match.offset != repState)
                        continue;

                    PathNode& nextNode = rowPointers[inputPos + match.length][repState];
                    uint32_t nextCost = node.cost + format.GetRepMatchCost(match.length);

                    if (nextCost < nextNode.cost)
                    {
                        nextNode = PathNode{nextCost, match.length, repState};
                    }
                }
            }

            // Propagate literals (only after a match).

            if (isMatch)
            {
                uint16_t maxLength = std::min<uint16_t>(inputSize - inputPos, format.MaxLiteralLength());

                for (uint16_t length = 1; length <= maxLength; length++)
                {
                    PathNode& nextNode = rowPointers[inputPos + length][repState];
                    uint32_t nextCost = node.cost + format.GetLiteralCost(length);

                    if (nextCost < nextNode.cost)
                    {
                        nextNode = PathNode{nextCost, 0, length};
                    }
                }
            }

            // Propagate regular matches.

            for (size_t i = regularMatchOffset; i < matches.size(); i++)
            {
                const Match& match = matches[i];

                PathNode& nextNode = rowPointers[inputPos + match.length][match.offset];
                uint32_t nextCost = node.cost + format.GetMatchCost(match.length, match.offset);

                if (nextCost < nextNode.cost)
                {
                    nextNode = PathNode{nextCost, match.length, repState};
                }
            }
        }
    }

    // Find the best final state at end of input.

    uint32_t bestRepState = 0;
    uint32_t bestCost = 0xFFFFFFFF;
    uint32_t lastRowWidth = GetRowWidth(inputSize, format.MaxMatchOffset());

    for (uint32_t repState = 0; repState < lastRowWidth; repState++)
    {
        const PathNode& node = rowPointers[inputSize][repState];

        if (node.cost < bestCost)
        {
            bestCost = node.cost;
            bestRepState = repState;
        }
    }

    // Backtrack to reconstruct the optimal parse sequence.

    std::vector<ParseStep> parse;

    while (inputSize)
    {
        const PathNode& node = rowPointers[inputSize][bestRepState];

        if (node.matchLength)
        {
            parse.emplace_back(node.matchLength, bestRepState);
            bestRepState = node.value;
            inputSize -= node.matchLength;
        }
        else
        {
            parse.emplace_back(node.value, 0);
            inputSize -= node.value;
        }
    }

    std::reverse(parse.begin(), parse.end());

    return parse;
}
