// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <algorithm>
#include "BitStream.h"

size_t BitStream::Size() const
{
    return mBytes.size();
}

const uint8_t* BitStream::Data() const
{
    return mBytes.data();
}

void BitStream::Reverse()
{
    std::reverse(mBytes.begin(), mBytes.end());
}

void BitStream::ReadReset()
{
    mReadMask = 0;
    mReadBitPos = 0;
    mReadBytePos = 0;
}

void BitStream::WriteReset()
{
    mBytes.clear();
    mWriteMask = 0;
    mWriteBitPos = 0;
}

void BitStream::WriteBit(bool value)
{
    if (mWriteMask == 0)
    {
        mWriteMask = 128;
        mWriteBitPos = mBytes.size();
        mBytes.push_back(0);
    }

    if (value)
    {
        mBytes[mWriteBitPos] |= mWriteMask;
    }

    mWriteMask >>= 1;
}

void BitStream::WriteByte(uint8_t value)
{
    mBytes.push_back(value);
}

uint32_t BitStream::ReadBit()
{
    if (mReadMask == 0)
    {
        mReadMask = 128;
        mReadBitPos = mReadBytePos;
        mReadBytePos++;
    }

    bool value = mBytes[mReadBitPos] & mReadMask;
    mReadMask >>= 1;

    return value;
}

uint8_t BitStream::ReadByte()
{
    return mBytes[mReadBytePos++];
}
