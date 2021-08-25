// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <vector>

class BitStream
{

public:

    size_t Size() const;
    const uint8_t* Data() const;
    void Reverse();

    void ReadReset();
    void WriteReset();

    void WriteBit(bool value);
    void WriteByte(uint8_t value);

    uint32_t ReadBit();
    uint8_t ReadByte();

private:

    std::vector<uint8_t> mBytes;

    uint8_t mWriteMask = 0;
    size_t mWriteBitPos = 0;

    uint8_t mReadMask = 0;
    size_t mReadBitPos = 0;
    size_t mReadBytePos = 0;
};

#endif // BIT_STREAM_H
