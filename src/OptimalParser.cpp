// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "OptimalParser.h"
#include "PrefixMatcher.h"

struct PathNode
{
    uint32_t cost = 0xFFFFFFFF;
    uint16_t length = 0;
    uint16_t offset = 0;
};

std::vector<ParseStep> Parse(const uint8_t* pInput, uint16_t inputSize, const Format& format)
{
    std::vector<Match> matches;

    PrefixMatcher matcher(
        pInput,
        inputSize,
        format.MinMatchLength(),
        format.MaxMatchLength(),
        format.MaxMatchOffset()
    );

    std::vector<PathNode> nodes(inputSize + 1);
    nodes[0].cost = 0;

    for (uint16_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        // Consider all available literals.

        uint16_t maxLength = std::min<uint16_t>(inputSize - inputPos, format.MaxLiteralLength());

        for (uint16_t length = maxLength; length > 0; length--)
        {
            uint32_t cost = nodes[inputPos].cost + format.GetLiteralCost(length);

            if (cost < nodes[inputPos + length].cost)
            {
                nodes[inputPos + length].cost = cost;
                nodes[inputPos + length].length = length;
                nodes[inputPos + length].offset = 0;
            }
        }

        // Consider all available matches.

        if (inputPos + format.MinMatchLength() > inputSize)
        {
            continue;
        }

        matches.clear();
        matcher.FindMatches(inputPos, false, matches);

        for (const Match& match: matches)
        {
            uint32_t cost = nodes[inputPos].cost + format.GetMatchCost(match.length, match.offset);

            if (cost < nodes[inputPos + match.length].cost)
            {
                nodes[inputPos + match.length].cost = cost;
                nodes[inputPos + match.length].length = match.length;
                nodes[inputPos + match.length].offset = match.offset;
            }
        }
    }

    // Backtrack and reconstruct the optimal parse sequence.

    std::vector<ParseStep> parse;

    while (inputSize)
    {
        parse.emplace_back(nodes[inputSize].length, nodes[inputSize].offset);
        inputSize -= nodes[inputSize].length;
    }

    std::reverse(parse.begin(), parse.end());

    return parse;
}
