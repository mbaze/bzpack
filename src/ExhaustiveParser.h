#ifndef EXHAUSTIVE_PARSER_H
#define EXHAUSTIVE_PARSER_H

#include <vector>
#include "CommonTypes.h"
#include "Formats.h"

class ExhaustiveParser
{
public:

    static std::vector<ParseStep> Parse(const uint8_t* pInput, uint32_t inputSize, const Format& format);
    ExhaustiveParser() = delete;

private:

    static uint16_t GetRowWidth(uint32_t inputPos, uint16_t maxOffset)
    {
        return 1 + std::min<uint16_t>(inputPos - (inputPos > 0), maxOffset);
    }

    static size_t GetNodeCount(uint32_t inputSize, uint16_t maxOffset)
    {
        maxOffset = std::min<uint16_t>(maxOffset, inputSize - 1);
        return (maxOffset + 1) * inputSize - ((maxOffset + 1) * maxOffset >> 1) + 1;
    }

    struct PathNode
    {
        static constexpr uint32_t INVALID_COST = 0xFFFFFFFF;

        uint32_t costAfterLiteral = INVALID_COST;
        uint32_t costAfterMatch = INVALID_COST;
        uint16_t literalLength = 0;
        uint16_t matchLength = 0;
    	uint16_t prevOffset = 0;
    };
};

#endif // EXHAUSTIVE_PARSER_H
