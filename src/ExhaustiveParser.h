#ifndef EXHAUSTIVE_PARSER_H
#define EXHAUSTIVE_PARSER_H

#include <vector>
#include "CommonTypes.h"
#include "Formats.h"

class ExhaustiveParser
{
public:

    std::vector<ParseStep> Parse(const uint8_t* pInput, uint32_t inputSize, const Format& format);

private:

    // To avoid ambiguity with repState = 0, literals use matchLength = 0, and value stores the length.

    struct PathNode
    {
        uint32_t cost = 0xFFFFFFFF;
        uint16_t matchLength = 0;
        uint16_t value = 0;
    };

    PathNode& NodeAt(uint32_t inputPos, uint32_t repOffset)
    {
        return mNodes[mRowOffsets[inputPos] + repOffset];
    }

    static uint16_t GetRowWidth(uint32_t inputPos, uint32_t maxOffset)
    {
        uint32_t rowWidth = 1;

        if (inputPos)
        {
            rowWidth += std::min(inputPos - 1, maxOffset);
        }

        return rowWidth;
    }

    std::vector<PathNode> mNodes;
    std::vector<size_t> mRowOffsets;
};

#endif // EXHAUSTIVE_PARSER_H
