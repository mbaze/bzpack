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

    // To avoid ambiguity with repState = 0, literals use matchLength = 0, and value stores the length.

    struct PathNode
    {
        uint32_t cost = 0xFFFFFFFF;
        uint16_t matchLength = 0;
        uint16_t value = 0;
    };

    static uint16_t GetRowWidth(uint32_t inputPos, uint16_t maxOffset)
    {
        return 1 + std::min<uint16_t>(inputPos - (inputPos > 0), maxOffset);
    }
};

#endif // EXHAUSTIVE_PARSER_H
