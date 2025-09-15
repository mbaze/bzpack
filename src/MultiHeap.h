// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef MULTI_HEAP_H
#define MULTI_HEAP_H

#include <vector>

template <typename T, size_t BITS = 3>
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
        T value = mHeap[0];

        mHeap[0] = mHeap.back();
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
            size_t parent = (child - 1) >> BITS;

            if (mHeap[child] >= mHeap[parent])
                break;

            std::swap(mHeap[child], mHeap[parent]);
            child = parent;
        }
    }

    void SiftDown(size_t parent)
    {
        while (true)
        {
            size_t lastChild = std::min((parent + 1) << BITS, mHeap.size() - 1);
            size_t minChild = parent;

            for (size_t child = (parent << BITS) + 1; child <= lastChild; child++)
            {
                if (mHeap[child] < mHeap[minChild])
                {
                    minChild = child;
                }
            }

            if (minChild == parent)
                break;

            std::swap(mHeap[parent], mHeap[minChild]);
            parent = minChild;
        }
    }

    std::vector<T> mHeap;
};

#endif // MULTI_HEAP_H
