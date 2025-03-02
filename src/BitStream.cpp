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

void BitStream::ReadReset()
{
    mReadMask = 0;
    mReadBitPos = 0;
    mReadBytePos = 0;
}

void BitStream::WriteReset()
{
    mBytes.clear();
    mWriteBitNum = 0;
    mWriteBitPos = 0;
    mIssueCarryWarning = false;
}

void BitStream::WriteBit(bool value)
{
    if (mWriteBitNum == 0)
    {
        mWriteBitPos = mBytes.size();
        mBytes.emplace_back(0);
    }

    mWriteBitNum = --mWriteBitNum & 7;
    mBytes[mWriteBitPos] |= (value << mWriteBitNum);
}

void BitStream::WriteByte(uint8_t value)
{
    mBytes.emplace_back(value);
}

uint32_t BitStream::ReadBit()
{
    mReadMask >>= 1;

    if (mReadMask == 0)
    {
        mReadMask = 128;
        mReadBitPos = mReadBytePos++;
    }

    return (mBytes[mReadBitPos] & mReadMask) > 0;
}

uint8_t BitStream::ReadByte()
{
    return mBytes[mReadBytePos++];
}

// These methods are only required by the E1ZX format.

void BitStream::WriteBitNeg(bool value)
{
    if (mWriteBitNum == 0)
    {
        if (mBytes.size())
        {
            mBytes[mWriteBitPos] = -mBytes[mWriteBitPos];
            mIssueCarryWarning |= !mBytes[mWriteBitPos];
        }

        mWriteBitPos = mBytes.size();
        mBytes.emplace_back(0);
    }

    mWriteBitNum = --mWriteBitNum & 7;
    mBytes[mWriteBitPos] |= (value << mWriteBitNum);
}

uint32_t BitStream::ReadBitNeg()
{
    mReadMask >>= 1;

    if (mReadMask == 0)
    {
        mReadMask = 128;
        mReadBitPos = mReadBytePos++;
    }

    return (-mBytes[mReadBitPos] & mReadMask) > 0;
}

void BitStream::FlushBitsNeg()
{
    // Setting unused bits to 1 reduces the chance of there being a zero byte.

    if (mBytes.size())
    {
        uint8_t padding = (1 << mWriteBitNum) - 1;
        mBytes[mWriteBitPos] = -(mBytes[mWriteBitPos] | padding);
        mIssueCarryWarning |= !mBytes[mWriteBitPos];
    }
}

bool BitStream::IssueCarryWarning() const
{
    return mIssueCarryWarning;
}
