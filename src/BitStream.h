// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <vector>

class BitStream
{
public:

    BitStream(bool complement = false)
    {
        mComplement = complement ? 0xFF : 0;
        ResetForWrite();
    }

    BitStream(BitStream&&) noexcept = default;
    BitStream& operator = (BitStream&&) noexcept = default;

    size_t Size() const;
    const uint8_t* Data() const;
    void Reverse();

    void ResetForRead();
    void ResetForWrite();

    void WriteBit(bool bit);
    void WriteByte(uint8_t byte);

    uint32_t ReadBit();
    uint8_t ReadByte();

    void FlushBits();
    bool IssueCarryWarning() const;

private:

    std::vector<uint8_t> mBytes;
    uint8_t mComplement;

    uint8_t mWriteBitPos;
    size_t mWriteBitCursor;
    size_t mFirstWriteBitCursor;

    uint8_t mReadBitMask;
    size_t mReadBitCursor;
    size_t mReadByteCursor;

    bool mIssueCarryWarning;
};

#endif // BIT_STREAM_H
