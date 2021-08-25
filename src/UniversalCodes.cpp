// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "UniversalCodes.h"

// Elias-Gamma code 1..N with interleaved flags and implicit MSB.

// 1: 0
// 2: 100
// 3: 110
// 4: 10100
// 5: 10110
// 6: 11100
// 7: 11110

uint32_t GetElias1Cost(uint32_t value)
{
    _ASSERT(value > 0);

    uint32_t mask = ~1;
    uint32_t bitCount = 0;

    while (value & mask)
    {
        bitCount++;
        mask <<= 1;
    }

    return (bitCount << 1) + 1;
}

void EncodeElias1(BitStream& stream, uint32_t value)
{
    _ASSERT(value > 0);

    uint32_t mask = ~1;
    uint32_t bitCount = 0;

    while (value & mask)
    {
        bitCount++;
        mask <<= 1;
    }

    mask = 1 << bitCount;
    mask >>= 1;

    while (mask)
    {
        stream.WriteBit(1);
        stream.WriteBit(value & mask);
        mask >>= 1;
    }

    stream.WriteBit(0);
}

uint32_t DecodeElias1(BitStream& stream)
{
    uint32_t value = 1;

    while (stream.ReadBit())
    {
        value = (value << 1) | stream.ReadBit();
    }

    return value;
}

// Elias-Gamma code 2..N with interleaved flags and implicit MSB.

// 2: 00
// 3: 10
// 4: 0100
// 5: 0110
// 6: 1100
// 7: 1110

uint32_t GetElias2Cost(uint32_t value)
{
    _ASSERT(value > 1);

    uint32_t mask = ~3;
    uint32_t bitCount = 1;

    while (value & mask)
    {
        bitCount++;
        mask <<= 1;
    }

    return bitCount << 1;
}

void EncodeElias2(BitStream& stream, uint32_t value)
{
    _ASSERT(value > 1);

    uint32_t mask = ~3;
    uint32_t bitCount = 0;

    while (value & mask)
    {
        bitCount++;
        mask <<= 1;
    }

    mask = 1 << bitCount;

    while (mask)
    {
        stream.WriteBit(value & mask);
        mask >>= 1;
        if (mask)
        {
            stream.WriteBit(1);
        }
    }

    stream.WriteBit(0);
}

uint32_t DecodeElias2(BitStream& stream)
{
    uint32_t value = 1;

    do
    {
        value = (value << 1) | stream.ReadBit();
    } while (stream.ReadBit());

    return value;
}

// Rice coding (K = 1).

uint32_t GetRiceCost(uint32_t value)
{
    return (value >> 1) + 2;
}

void EncodeRice(BitStream& stream, uint32_t value)
{
    uint32_t count = value >> 1;

    while (count--)
    {
        stream.WriteBit(1);
    }

    stream.WriteBit(0);
    stream.WriteBit(value & 1);
}

uint32_t DecodeRice(BitStream& stream)
{
    uint32_t value = 0;

    while (stream.ReadBit())
    {
        value++;
    }

    return (value << 1) | stream.ReadBit();
}
