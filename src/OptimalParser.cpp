// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <cassert>
#include <algorithm>
#include "OptimalParser.h"
#include "UniversalCodes.h"

FormatLimits GetFormatLimits(uint32_t format, bool wasLiteral)
{
    FormatLimits limits;

    bool extendOffset = (format & Format::FlagExtendOffset);
    bool extendLength = (format & Format::FlagExtendLength);

    switch (format & Format::Mask)
    {
    case Format::AlignedLZSS:

        limits.minMatchLength = 2;
        limits.maxMatchOffset = extendOffset ? 256 : 255;
        limits.maxMatchLength = extendLength ? 128 : 127;
        limits.maxLiteralLength = extendLength ? 128 : 127;
        break;

    case Format::Elias1_Elias1:

        limits.minMatchLength = 1;
        limits.maxMatchOffset = extendOffset ? 256 : 255;
        limits.maxMatchLength = 255;
        limits.maxLiteralLength = 255;
        break;

    case Format::Elias1_ExtElias1:

        // This format doesn't allow two consecutive literals.

        if (wasLiteral)
        {
            limits.maxMatchOffset = extendOffset ? 512 : 511;
            limits.maxLiteralLength = 0;
        }
        else
        {
            limits.maxMatchOffset = extendOffset ? 256 : 255;
            limits.maxLiteralLength = 255;
        }

        limits.minMatchLength = 2;
        limits.maxMatchLength = 254;
        break;

    case Format::Unary_Elias2:

        limits.minMatchLength = 2;
        limits.maxMatchOffset = extendOffset ? 256 : 255;
        limits.maxMatchLength = 255;
        limits.maxLiteralLength = UINT32_MAX;
        break;

    default:
        assert(0);
    }

    return limits;
}

uint32_t GetLiteralCost(uint32_t format, uint32_t length)
{
    format &= Format::Mask;

    switch (format)
    {
    case Format::AlignedLZSS:
        return 8 + 8 * length;

    case Format::Elias1_Elias1:
    case Format::Elias1_ExtElias1:
        return GetElias1Cost(length) + 1 + 8 * length;

    case Format::Unary_Elias2:
        return (1 + 8) * length;
    }

    return UINT32_MAX;
}

uint32_t GetMatchCost(uint32_t format, uint32_t offset, uint32_t length)
{
    switch (format & Format::Mask)
    {
    case Format::AlignedLZSS:
        return 8 + 8;

    case Format::Elias1_Elias1:
        return GetElias1Cost(length) + 1 + 8;

    case Format::Elias1_ExtElias1:
        return GetElias1Cost(length & 255) + 1 + 8;

    case Format::Unary_Elias2:
        return 1 + GetElias2Cost(length) + 8;
    }

    return UINT32_MAX;
}

void FindMatches(std::vector<StreamRef>& matches, const uint8_t* pInputStream, size_t inputSize, size_t position, const FormatLimits& limits)
{
    matches.clear();

    uint32_t maxOffset = std::min(static_cast<uint32_t>(position), limits.maxMatchOffset);
    uint32_t maxLength = std::min(static_cast<uint32_t>(inputSize - position), limits.maxMatchLength);

    for (uint32_t offset = 1; offset <= maxOffset; offset++)
    {
        for (uint32_t i = 0; i < maxLength; )
        {
            if (pInputStream[position + i] != pInputStream[position - offset + i])
            {
                break;
            }

            i++;

            if (i >= limits.minMatchLength)
            {
                StreamRef match;
                match.offset = offset;
                match.length = i;
                matches.push_back(match);
            }
        }
    }
}

bool Parse(const uint8_t* pInputStream, size_t inputSize, uint32_t format, std::vector<StreamRef>& refs)
{
    if (pInputStream == nullptr || inputSize == 0)
    {
        return false;
    }

    // Forward pass.

    std::vector<StreamRef> matches(256);
    std::vector<Node> nodes(inputSize + 1);

    nodes[0].cost = GetLiteralCost(format, 1);

    for (size_t i = 0; i < inputSize; i++)
    {
        FormatLimits limits = GetFormatLimits(format, nodes[i].literalCount);

        // Does encoding a literal reduce the cost?

        uint32_t maxLength = std::min(static_cast<uint32_t>(inputSize - i), limits.maxLiteralLength);

        for (uint32_t l = 1; l <= maxLength; l++)
        {
            uint32_t literalCost = nodes[i].cost + GetLiteralCost(format, l);

            // Prefer literals over phrases if the cost is the same.

            if (literalCost <= nodes[i + l].cost)
            {
                nodes[i + l].offset = 0;
                nodes[i + l].length = 1;
                nodes[i + l].cost = literalCost;
                nodes[i + l].literalCount = l;
            }
        }

        // Does encoding a match reduce the cost?

        FindMatches(matches, pInputStream, inputSize, i, limits);

        for (const StreamRef& match: matches)
        {
            uint32_t matchCost = nodes[i].cost + GetMatchCost(format, match.offset, match.length);

            // Prefer shorter offsets if the cost is the same.

            if (matchCost < nodes[i + match.length].cost)
            {
                nodes[i + match.length].offset = match.offset;
                nodes[i + match.length].length = match.length;
                nodes[i + match.length].cost = matchCost;
                nodes[i + match.length].literalCount = 0;
            }
        }
    }

    // Backward pass.

    refs.clear();
    size_t position = inputSize;

    while (position)
    {
        StreamRef ref;
        ref.offset = nodes[position].offset;

        if (ref.offset)
        {
            ref.length = nodes[position].length;
        }
        else
        {
            ref.length = nodes[position].literalCount;
        }

        position -= ref.length;
        refs.push_back(ref);
    }

    std::reverse(refs.begin(), refs.end());

    return true;
}
