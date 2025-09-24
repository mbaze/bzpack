// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "BitStream.h"
#include <algorithm>

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

void BitStream::ResetForRead()
{
    mReadBitMask = 0;
    mReadBitCursor = 0;
    mReadByteCursor = 0;
}

void BitStream::ResetForWrite()
{
    mBytes.clear();

    mWriteBitPos = 0;
    mWriteBitCursor = 0;
    mFirstWriteBitCursor = SIZE_MAX;

    ResetForRead();
}

void BitStream::WriteBit(bool bit)
{
    if (mWriteBitPos == 0)
    {
        mWriteBitCursor = mBytes.size();
        mFirstWriteBitCursor = std::min(mFirstWriteBitCursor, mWriteBitCursor);
        mBytes.emplace_back(mComplement);
    }

    mWriteBitPos = --mWriteBitPos & 7;
    mBytes[mWriteBitCursor] ^= bit << mWriteBitPos;
}

void BitStream::WriteByte(uint8_t byte)
{
    mBytes.emplace_back(byte);
}

uint32_t BitStream::ReadBit()
{
    mReadBitMask >>= 1;

    if (mReadBitMask == 0)
    {
        mReadBitMask = 128;
        mReadBitCursor = mReadByteCursor++;
    }

    uint8_t bits = mBytes[mReadBitCursor];
    if (mReadBitCursor == mFirstWriteBitCursor)
    {
        bits--;
    }

    return ((bits ^ mComplement) & mReadBitMask) > 0;
}

uint8_t BitStream::ReadByte()
{
    return mBytes[mReadByteCursor++];
}

void BitStream::FlushBits()
{
    if (mComplement == 0 || mBytes.empty())
        return;

    if (mFirstWriteBitCursor != SIZE_MAX)
    {
        mBytes[mFirstWriteBitCursor]++;
    }
}
