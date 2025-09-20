// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef MULTI_HEAP_H
#define MULTI_HEAP_H

#include "BlockVector.h"

template <typename T, size_t ARITY_BITS = 4>
class MultiHeap
{
public:

    inline size_t Size() const
    {
        return mHeap.size();
    }

    inline void Push(const T& value)
    {
        mHeap.emplace_back(value);
        SiftUp(mHeap.size() - 1);
    }

    inline T Pop()
    {
        T value = mHeap.front();
        mHeap.front() = mHeap.back();
        mHeap.pop_back();

        if (mHeap.size())
        {
            SiftDown(0);
        }

        return value;
    }

private:

    void SiftUp(size_t child)
    {
        while (child)
        {
            size_t parent = (child - 1) >> ARITY_BITS;

            if (mHeap[child] >= mHeap[parent])
                break;

            Swap(parent, child);
            child = parent;
        }
    }

    void SiftDown(size_t parent)
    {
        while (true)
        {
            size_t lastChild = std::min((parent + 1) << ARITY_BITS, mHeap.size() - 1);
            size_t minChild = parent;

            for (size_t child = (parent << ARITY_BITS) + 1; child <= lastChild; child++)
            {
                if (mHeap[child] < mHeap[minChild])
                {
                    minChild = child;
                }
            }

            if (minChild == parent)
                break;

            Swap(parent, minChild);
            parent = minChild;
        }
    }

    void Swap(size_t index1, size_t index2)
    {
        T temp = mHeap[index1];
        mHeap[index1] = mHeap[index2];
        mHeap[index2] = temp;
    }

    BlockVector<T> mHeap;
};

#endif // MULTI_HEAP_H
