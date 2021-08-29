// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <cassert>
#include "UniversalCodes.h"

// Elias-Gamma 1..N encoding (interleaved format).

// 1: 0
// 2: 100
// 3: 110
// 4: 10100
// 5: 10110
// 6: 11100
// 7: 11110
// 8: 1010100

uint32_t GetElias1Cost(uint32_t value)
{
    assert(value > 0);

    uint32_t mask = ~1;
    uint32_t count = 0;

    while (value & mask)
    {
        count++;
        mask <<= 1;
    }

    return (count << 1) + 1;
}

void EncodeElias1(BitStream& stream, uint32_t value)
{
    assert(value > 0);

    uint32_t mask = ~1;
    uint32_t count = 0;

    while (value & mask)
    {
        count++;
        mask <<= 1;
    }

    mask = 1 << count;
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

// Elias-Gamma 2..N encoding (interleaved format).

// 2: 00
// 3: 10
// 4: 0100
// 5: 0110
// 6: 1100
// 7: 1110
// 8: 010100
// 9: 010110

uint32_t GetElias2Cost(uint32_t value)
{
    assert(value > 1);

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
    assert(value > 1);

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
    }
    while (stream.ReadBit());

    return value;
}

// Unary encoding.

// 0: 0
// 1: 10
// 2: 110
// 3: 1110
// 4: 11110
// 5: 111110
// 6: 1111110
// 7: 11111110

uint32_t GetUnaryCost(uint32_t value)
{
    return value + 1;
}

void EncodeUnary(BitStream& stream, uint32_t value)
{
    for (uint32_t i = 0; i < value; i++)
    {
        stream.WriteBit(1);
    }

    stream.WriteBit(0);
}

uint32_t DecodeUnary(BitStream& stream)
{
    uint32_t value = 0;

    while (stream.ReadBit())
    {
        value++;
    }

    return value;
}

// Rice encoding (K = 1).

// 0: 00
// 1: 01
// 2: 100
// 3: 101
// 4: 1100
// 5: 1101
// 6: 11100
// 7: 11101

uint32_t GetRiceCost(uint32_t value)
{
    return (value >> 1) + 2;
}

void EncodeRice(BitStream& stream, uint32_t value)
{
    for (uint32_t i = 0; i < value >> 1; i++)
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

// Vbinary 2x2 encoding.

// 0: 00
// 1: 01
// 2: 10
// 3: 1100
// 4: 1101
// 5: 1110
// 6: 111100
// 7: 111101

uint32_t GetVbinCost(uint32_t value)
{
    return (value / 3 + 1) << 1;
}

void EncodeVbin(BitStream& stream, uint32_t value)
{
    uint32_t count = value / 3;

    for (uint32_t i = 0; i < count; i++)
    {
        stream.WriteBit(1);
        stream.WriteBit(1);
        value -= 3;
    }

    stream.WriteBit(value & 2);
    stream.WriteBit(value & 1);
}

uint32_t DecodeVbin(BitStream& stream)
{
    uint32_t value = 0;

    while (1)
    {
        uint32_t bits = stream.ReadBit();
        bits = (bits << 1) | stream.ReadBit();
        value += bits;

        if ((bits & 3) < 3)
        {
            break;
        }
    }

    return value;
}

// Plain binary encoding (the number of bits is explicit).

void EncodeRaw(BitStream& stream, uint32_t value, uint32_t bits)
{
    assert(bits > 0);

    uint32_t mask = 1 << (bits - 1);

    while (mask)
    {
        stream.WriteBit(value & mask);
        mask >>= 1;
    }
}

uint32_t DecodeRaw(BitStream& stream, uint32_t bits)
{
    assert(bits > 0);

    uint32_t value = 0;

    while (bits--)
    {
        value = (value << 1) | stream.ReadBit();
    }

    return value;
}
