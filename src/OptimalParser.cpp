// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "OptimalParser.h"
#include <algorithm>
#include "PrefixMatcher.h"

std::vector<ParseStep> OptimalParser::Parse(const uint8_t* pInput, uint32_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    // Precompute all available matches for each input position.

    std::vector<Match> matches;
    PrefixMatcher matcher(pInput, inputSize, format.MinMatchLength(), format.MaxMatchLength(), format.MaxMatchOffset());

    // Initialize the state and sweep over all coding paths at each input position.

    std::vector<PathNode> nodes(inputSize + 1);
    nodes[0].cost = 0;

    for (uint32_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        const PathNode& node = nodes[inputPos];

        // Propagate literals.

        uint16_t maxLength = std::min<uint16_t>(inputSize - inputPos, format.MaxLiteralLength());

        for (uint16_t length = 1; length <= maxLength; length++)
        {
            PathNode& nextNode = nodes[inputPos + length];
            uint32_t nextCost = node.cost + format.GetLiteralCost(length);

            if (nextCost < nextNode.cost)
            {
                nextNode = PathNode{nextCost, length, 0};
            }
        }

        // Propagate matches.

        matcher.GetMatches(matches, inputPos);

        for (const Match& match: matches)
        {
            PathNode& nextNode = nodes[inputPos + match.length];
            uint32_t nextCost = node.cost + format.GetMatchCost(match.length, match.offset);

            if (nextCost < nextNode.cost)
            {
                nextNode = PathNode{nextCost, match.length, match.offset};
            }
        }
    }

    // Backtrack to reconstruct the optimal parse sequence.

    std::vector<ParseStep> parse;

    while (inputSize)
    {
        const PathNode& node = nodes[inputSize];
        parse.emplace_back(node.length, node.offset);
        inputSize -= node.length;
    }

    std::reverse(parse.begin(), parse.end());

    return parse;
}
