// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef COST_TABLE_H
#define COST_TABLE_H

#include <algorithm>
#include <cstdint>
#include <vector>

class CostTable
{
public:

    CostTable() = delete;

    CostTable(uint16_t inputSize, uint16_t maxOffset)
    {
        mRowOffsets.resize(inputSize + 1);
        size_t elementCount = 0;

        for (uint16_t i = 0; i < mRowOffsets.size(); i++)
        {
            mRowOffsets[i] = elementCount;
            elementCount += GetRowSize(i, maxOffset);
        }

        size_t byteCount = (elementCount * 5 + 3) & ~3;
        mCosts.resize(byteCount, 255);
    }

    uint32_t Get(uint16_t inputPos, uint16_t repOffset)
    {
        size_t nibble = GetNibbleIndex(inputPos, repOffset);
        uint32_t shift = (nibble & 1) << 2;

        const uint32_t& value = reinterpret_cast<const uint32_t&>(mCosts[nibble >> 1]);
        return (value >> shift) & 0xFFFFF;
    }

    void Set(uint16_t inputPos, uint16_t repOffset, uint32_t cost)
    {
        size_t nibble = GetNibbleIndex(inputPos, repOffset);
        uint32_t shift = (nibble & 1) << 2;
        uint32_t mask = ~(0xFFFFF << shift);

        uint32_t& value = reinterpret_cast<uint32_t&>(mCosts[nibble >> 1]);
        value = (value & mask) | (cost << shift);
    }

private:

    // The maximum valid offset to reach position P is P - 1, capped
    // by the format's maximum allowed length. One entry is reserved
    // for a repeat offset that has not yet been set (zero).

    static uint16_t GetRowSize(uint16_t inputPos, uint16_t maxOffset)
    {
        uint16_t rowWidth = 1;

        if (inputPos)
        {
            rowWidth += std::min<uint16_t>(inputPos - 1, maxOffset);
        }

        return rowWidth;
    }

    size_t GetNibbleIndex(uint16_t inputPos, uint16_t repOffset) const
    {
        size_t elementIndex = mRowOffsets[inputPos] + repOffset;
        return (elementIndex << 2) + elementIndex;
    }

    std::vector<size_t> mRowOffsets;
    std::vector<uint8_t> mCosts;
};

#endif // COST_TABLE_H
