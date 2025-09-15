// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

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

void BitStream::ResetForRead()
{
    mReadBitMask = 0;
    mReadBitCursor = 0;
    mReadByteCursor = 0;
}

void BitStream::ResetForWrite()
{
    mWriteBitPos = 0;
    mWriteBitCursor = 0;
    mBytes.clear();
    mIssueCarryWarning = false;

    ResetForRead();
}

void BitStream::WriteBit(bool value)
{
    if (mWriteBitPos == 0)
    {
        mWriteBitCursor = mBytes.size();
        mBytes.emplace_back(0);
    }

    mWriteBitPos = --mWriteBitPos & 7;
    mBytes[mWriteBitCursor] |= (value << mWriteBitPos);
}

void BitStream::WriteByte(uint8_t value)
{
    mBytes.emplace_back(value);
}

uint32_t BitStream::ReadBit()
{
    mReadBitMask >>= 1;

    if (mReadBitMask == 0)
    {
        mReadBitMask = 128;
        mReadBitCursor = mReadByteCursor++;
    }

    return (mBytes[mReadBitCursor] & mReadBitMask) > 0;
}

uint8_t BitStream::ReadByte()
{
    return mBytes[mReadByteCursor++];
}

// Only used by the E1ZX format.

void BitStream::WriteBitNeg(bool value)
{
    if (mWriteBitPos == 0)
    {
        if (mBytes.size())
        {
            mBytes[mWriteBitCursor] = -mBytes[mWriteBitCursor];
            mIssueCarryWarning |= !mBytes[mWriteBitCursor];
        }

        mWriteBitCursor = mBytes.size();
        mBytes.emplace_back(0);
    }

    mWriteBitPos = --mWriteBitPos & 7;
    mBytes[mWriteBitCursor] |= (value << mWriteBitPos);
}

uint32_t BitStream::ReadBitNeg()
{
    mReadBitMask >>= 1;

    if (mReadBitMask == 0)
    {
        mReadBitMask = 128;
        mReadBitCursor = mReadByteCursor++;
    }

    return (-mBytes[mReadBitCursor] & mReadBitMask) > 0;
}

void BitStream::FlushBitsNeg()
{
    // Setting unused bits to 1 reduces the chance of there being a zero byte.

    if (mBytes.size())
    {
        uint8_t padding = (1 << mWriteBitPos) - 1;
        mBytes[mWriteBitCursor] = -(mBytes[mWriteBitCursor] | padding);
        mIssueCarryWarning |= !mBytes[mWriteBitCursor];
    }
}

bool BitStream::IssueCarryWarning() const
{
    return mIssueCarryWarning;
}
