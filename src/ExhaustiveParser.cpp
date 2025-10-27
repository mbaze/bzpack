// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "ExhaustiveParser.h"
#include "PrefixMatcher.h"

std::vector<ParseStep> ExhaustiveParser::Parse(const uint8_t* pInput, uint32_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    // Precompute all available matches for each input position.

    std::vector<Match> matches;
    PrefixMatcher matcher(pInput, inputSize, format.MinMatchLength(), format.MaxMatchLength(), format.MaxMatchOffset());

    // Allocate a triangular DP table (row pointers into a contiguous buffer).

    size_t nodeCount = GetNodeCount(inputSize, format.MaxMatchOffset());
    std::vector<PathNode> nodeBuffer(nodeCount);

    std::vector<PathNode*> nodes(inputSize + 1);
    PathNode* rowPtr = nodeBuffer.data();

    for (uint32_t inputPos = 0; inputPos <= inputSize; inputPos++)
    {
        nodes[inputPos] = rowPtr;
        rowPtr += GetRowWidth(inputPos, format.MaxMatchOffset());
    }

    // Initialize the state and sweep over all coding paths at each input position.

    nodes[0]->costAfterMatch = 0;
    nodes[0]->literalRowWidth = 1;

    for (uint32_t inputPos = 0; inputPos < inputSize; inputPos++)
    {
        size_t matchIndex = matcher.GetMatches(matches, inputPos, true);

        // Mark future positions that are reachable by available matches.

        for (const Match& match: matches)
        {
            uint32_t nextPos = inputPos + match.length;
            nodes[nextPos]->literalRowWidth = GetRowWidth(nextPos, format.MaxMatchOffset());
        }

        // Propagate literals (only from states that ended with a match).

        uint16_t rowWidth = nodes[inputPos]->literalRowWidth;
        uint16_t maxLength = std::min<uint16_t>(inputSize - inputPos, format.MaxLiteralLength());

        for (uint16_t offset = 0; offset < rowWidth; offset++)
        {
            uint32_t cost = nodes[inputPos][offset].CostAfterMatch();
            if (cost == PathNode::INVALID_COST)
                continue;

            for (uint16_t length = 1; length <= maxLength; length++)
            {
                PathNode& nextNode = nodes[inputPos + length][offset];
                uint32_t nextCost = cost + format.GetLiteralCost(length);

                if (nextCost < nextNode.costAfterLiteral)
                {
                    nextNode.costAfterLiteral = nextCost;
                    nextNode.literalLength = length;
                }
            }
        }

        // Propagate repeat matches (only from states that ended with a literal).

        if (matches.empty())
            continue;

        rowWidth = GetRowWidth(inputPos, format.MaxMatchOffset());

        for (uint16_t offset = 1; offset < rowWidth; offset++)
        {
            uint32_t cost = nodes[inputPos][offset].costAfterLiteral;
            if (cost == PathNode::INVALID_COST)
                continue;

            for (const Match& match: matches)
            {
                if (match.offset != offset)
                    continue;

                PathNode& nextNode = nodes[inputPos + match.length][offset];
                uint32_t nextCost = cost + format.GetRepMatchCost(match.length);

                if (nextCost < nextNode.CostAfterMatch())
                {
                    nextNode.costAfterMatch = 0x80000000 | nextCost;
                    nextNode.matchLength = match.length;
                }
            }
        }

        // Find the minimum cost at the current position and store the backtracking offset.

        uint32_t bestCost = PathNode::INVALID_COST;
        uint16_t bestOffset = 0xFFFF;

        for (uint16_t offset = 0; offset < rowWidth; offset++)
        {
            uint32_t minCost = nodes[inputPos][offset].MinCost();

            if (minCost < bestCost)
            {
                bestCost = minCost;
                bestOffset = offset;
            }
        }

        nodes[inputPos]->backtrackOffset = bestOffset;

        // Propagate regular matches (prior offset is irrelevant).

        for (size_t i = matchIndex; i < matches.size(); i++)
        {
            const Match& match = matches[i];
            PathNode& nextNode = nodes[inputPos + match.length][match.offset];
            uint32_t nextCost = bestCost + format.GetMatchCost(match.length, match.offset);

            if (nextCost < nextNode.CostAfterMatch())
            {
                nextNode.costAfterMatch = nextCost;
                nextNode.matchLength = match.length;
            }
        }
    }

    // Find the best final state at the end of input.

    uint16_t rowWidth = GetRowWidth(inputSize, format.MaxMatchOffset());
    uint32_t bestCost = PathNode::INVALID_COST;
    uint16_t bestOffset = 0;

    for (uint16_t offset = 0; offset < rowWidth; offset++)
    {
        uint32_t minCost = nodes[inputSize][offset].MinCost();

        if (minCost < bestCost)
        {
            bestCost = minCost;
            bestOffset = offset;
        }
    }

    bool isLiteral = nodes[inputSize][bestOffset].PreferLiteralPath();

    // Backtrack to reconstruct the optimal parse sequence.

    std::vector<ParseStep> parse;

    while (inputSize)
    {
        const PathNode& node = nodes[inputSize][bestOffset];

        if (isLiteral)
        {
            parse.emplace_back(node.literalLength, 0);
            inputSize -= node.literalLength;
            isLiteral = false;
        }
        else
        {
            parse.emplace_back(node.matchLength, bestOffset);
            inputSize -= node.matchLength;
            isLiteral = node.IsRepeatMatch();

            if (!isLiteral)
            {
                bestOffset = nodes[inputSize]->backtrackOffset;
                isLiteral = nodes[inputSize][bestOffset].PreferLiteralPath();
            }
        }
    }

    std::reverse(parse.begin(), parse.end());

    return parse;
}
