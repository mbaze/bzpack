// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <vector>

class BitStream
{

public:

    BitStream()
    {
        ResetForWrite();
    }

    size_t Size() const;
    const uint8_t* Data() const;
    void Reverse();

    void ResetForRead();
    void ResetForWrite();

    void WriteBit(bool value);
    void WriteByte(uint8_t value);

    uint32_t ReadBit();
    uint8_t ReadByte();

    // Only used by the E1ZX format.

    void WriteBitNeg(bool value);
    uint32_t ReadBitNeg();
    void FlushBitsNeg();
    bool IssueCarryWarning() const;

private:

    std::vector<uint8_t> mBytes;

    uint8_t mWriteBitPos;
    size_t mWriteBitCursor;

    uint8_t mReadBitMask;
    size_t mReadBitCursor;
    size_t mReadByteCursor;

    bool mIssueCarryWarning;
};

#endif // BIT_STREAM_H
