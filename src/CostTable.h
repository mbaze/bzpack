// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef COST_TABLE_H
#define COST_TABLE_H

#include <algorithm>
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
            elementCount += RowWidth(i, maxOffset);
        }

        size_t byteCount = (elementCount * 5 + 1) >> 1;
        mCosts.resize(byteCount, 255);
    }

    void Set(uint16_t inputPos, uint16_t repOffset, uint32_t cost)
    {
        size_t nibblePos = NibbleIndex(inputPos, repOffset);
        size_t i = nibblePos >> 1;

        if (nibblePos & 1)
        {
            mCosts[i + 2] = cost;
            cost >>= 8;
            mCosts[i + 1] = cost;
            cost >>= 8;
            mCosts[i] = (mCosts[i] & 0x0F) | (cost << 4);
        }
        else
        {
            mCosts[i] = cost;
            cost >>= 8;
            mCosts[i + 1] = cost;
            cost >>= 8;
            mCosts[i + 2] = (mCosts[i + 2] & 0xF0) | cost;
        }
    }

    uint32_t Get(uint16_t inputPos, uint16_t repOffset)
    {
        size_t nibblePos = NibbleIndex(inputPos, repOffset);
        size_t i = nibblePos >> 1;

        if (nibblePos & 1)
        {
            uint32_t cost = mCosts[i] >> 4;
            cost = (cost << 8) | mCosts[i + 1];
            cost = (cost << 8) | mCosts[i + 2];
            return cost;
        }
        else
        {
            uint32_t cost = mCosts[i + 2] & 0x0F;
            cost = (cost << 8) | mCosts[i + 1];
            cost = (cost << 8) | mCosts[i];
            return cost;
        }
    }

private:

    // The maximum valid offset to reach position P is P - 1, capped
    // by the format's maximum allowed length. One entry is reserved
    // for a repeat offset that has not yet been set (zero).

    static uint16_t RowWidth(uint16_t inputPos, uint16_t maxOffset)
    {
        uint16_t rowWidth = 1;

        if (inputPos)
        {
            rowWidth += std::min<uint16_t>(inputPos - 1, maxOffset);
        }

        return rowWidth;
    }

    size_t NibbleIndex(uint16_t inputPos, uint16_t repOffset) const
    {
        size_t elementIndex = mRowOffsets[inputPos] + repOffset;
        return (elementIndex << 2) + elementIndex;
    }

    std::vector<size_t> mRowOffsets;
    std::vector<uint8_t> mCosts;
};

#endif // COST_TABLE_H
