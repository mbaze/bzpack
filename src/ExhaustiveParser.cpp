// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "ExhaustiveParser.h"
#include "PrefixMatcher.h"

std::vector<ParseStep> ExhaustiveParser::Parse(const uint8_t* pInput, uint32_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    // Precompute matches.

    PrefixMatcher matcher(pInput, inputSize, format.MinMatchLength(), format.MaxMatchLength(), format.MaxMatchOffset());
    std::vector<Match> matches;

    // Precompute row offsets for triangular DP table.

    mRowOffsets.resize(inputSize + 1);
    size_t elementCount = 0;

    for (uint32_t i = 0; i < mRowOffsets.size(); i++)
    {
        mRowOffsets[i] = elementCount;
        elementCount += GetRowWidth(i, format.MaxMatchOffset());
    }

    mNodes.resize(elementCount);
    mNodes[0] = {0, 1, 0};

    for (uint32_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        matches.clear();
        size_t byteMatchCount = matcher.FindMatches(inputPos, true, matches);
        uint32_t rowWidth = GetRowWidth(inputPos, format.MaxMatchOffset());

        for (uint16_t repState = 0; repState < rowWidth; repState++)
        {
            const PathNode& node = NodeAt(inputPos, repState);

            // Skip unreachable states.
            if (node.cost == 0xFFFFFFFF)
                continue;

            bool isMatch = node.matchLength;
            bool isLiteral = !isMatch;

            // Propagate repeat matches.

            if (isLiteral)
            {
                for (const Match& match: matches)
                {
                    if (match.offset != repState)
                        continue;

                    PathNode& nextNode = NodeAt(inputPos + match.length, repState);
                    uint32_t nextCost = node.cost + format.GetRepMatchCost(match.length);

                    if (nextCost < nextNode.cost)
                    {
                        nextNode = PathNode{nextCost, match.length, repState};
                    }
                }
            }

            // Propagate literals.

            if (isMatch)
            {
                uint16_t maxLength = std::min<uint16_t>(inputSize - inputPos, format.MaxLiteralLength());

                for (uint16_t length = 1; length <= maxLength; length++)
                {
                    PathNode& nextNode = NodeAt(inputPos + length, repState);
                    uint32_t nextCost = node.cost + format.GetLiteralCost(length);

                    if (nextCost < nextNode.cost)
                    {
                        nextNode = PathNode{nextCost, 0, length};
                    }
                }
            }

            // Propagate regular matches.

            for (size_t i = byteMatchCount; i < matches.size(); i++)
            {
                const Match& match = matches[i];

                PathNode& nextNode = NodeAt(inputPos + match.length, match.offset);
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
        const PathNode& node = NodeAt(inputSize, repState);
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
        const PathNode& node = NodeAt(inputSize, bestRepState);

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
