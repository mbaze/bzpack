// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "OptimalParser.h"

bool Parse(std::vector<Match>& parse, const uint8_t* pInput, uint16_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
    {
        return false;
    }

    FormatLimits limits = format.GetLimits();
    MatchFinder matchFinder(pInput, inputSize, limits);

    std::vector<Match> matches;
    matches.reserve(256);

    std::vector<Node> nodes(inputSize + 1);
    nodes[0].cost = 0;

    // Forward pass.

    for (uint16_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        // Does encoding a literal reduce the cost?

        uint16_t maxLength = std::min<uint16_t>(inputSize - inputPos, limits.maxLiteralLength);

        for (uint16_t length = 1; length <= maxLength; length++)
        {
            uint32_t cost = nodes[inputPos].cost + format.GetLiteralCost(length);

            if (cost < nodes[inputPos + length].cost)
            {
                nodes[inputPos + length].offset = 0;
                nodes[inputPos + length].length = length;
                nodes[inputPos + length].cost = cost;
            }
        }

        // Does encoding a match reduce the cost?

        if (inputPos > inputSize - limits.minMatchLength)
        {
            continue;
        }

        matches.clear();
        matchFinder.FindMatches(matches, inputPos);

        for (const Match& match: matches)
        {
            uint32_t cost = nodes[inputPos].cost + format.GetMatchCost(match.offset, match.length);

            if (cost < nodes[inputPos + match.length].cost)
            {
                nodes[inputPos + match.length].offset = match.offset;
                nodes[inputPos + match.length].length = match.length;
                nodes[inputPos + match.length].cost = cost;
            }
        }
    }

    // Backward pass.

    parse.clear();
    size_t position = inputSize;

    while (position)
    {
        parse.push_back({nodes[position].offset, nodes[position].length});
        position -= nodes[position].length;
    }

    std::reverse(parse.begin(), parse.end());

    return true;
}
