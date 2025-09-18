// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef BLOCK_VECTOR_H
#define BLOCK_VECTOR_H

#include <array>
#include <vector>

template<typename T, size_t BLOCK_BITS = 21>
class BlockVector
{
    static constexpr size_t BLOCK_SIZE = 1 << BLOCK_BITS;
    static constexpr size_t BLOCK_MASK = BLOCK_SIZE - 1;
    using Block = std::array<T, BLOCK_SIZE>;

public:

    size_t size() const { return mSize; }

    T& front() { return mBlocks[0][0]; }
    const T& front() const { return mBlocks[0][0]; }

    T& back() { return (*this)[mSize - 1]; }
    const T& back() const { return (*this)[mSize - 1]; }

    T& operator [] (size_t i)
    {
        return mBlocks[i >> BLOCK_BITS][i & BLOCK_MASK];
    }

    const T& operator [] (size_t i) const
    {
        return mBlocks[i >> BLOCK_BITS][i & BLOCK_MASK];
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        size_t blockIndex = mSize >> BLOCK_BITS;
        size_t blockOffset = mSize & BLOCK_MASK;

        if (blockIndex >= mBlocks.size())
        {
            mBlocks.emplace_back();
        }

        mBlocks[blockIndex][blockOffset] = T(std::forward<Args>(args)...);
        mSize++;
    }

    void pop_back()
    {
        if (!mSize || (mSize-- & BLOCK_MASK))
            return;

        mBlocks.pop_back();
    }

private:

    size_t mSize = 0;
    std::vector<Block> mBlocks;
};

#endif // BLOCK_VECTOR_H
