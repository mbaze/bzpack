// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <cassert>
#include <algorithm>
#include "Compressor.h"
#include "OptimalParser.h"
#include "UniversalCodes.h"

#ifdef _DEBUG
#define VERIFY
#endif // _DEBUG

bool EncodeLZS(const uint8_t* pInput, const std::vector<Match>& parse, FormatOptions options, BitStream& packedStream)
{
    if (options.id != FormatId::LZ)
    {
        return false;
    }

    packedStream.WriteReset();
    uint16_t i = 0;

    for (const Match& match: parse)
    {
        uint16_t length = match.length;
        if (options.extendLength) length--;

        if (match.offset)
        {
            uint16_t offset = match.offset;
            if (options.extendOffset) offset--;

            packedStream.WriteByte(static_cast<uint8_t>(length << 1));
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            i += match.length;
        }
        else
        {
            packedStream.WriteByte(static_cast<uint8_t>(length << 1) | 1);

            for (uint16_t b = 0; b < match.length; b++)
            {
                packedStream.WriteByte(pInput[i++]);
            }
        }
    }

    if (options.addEndMarker)
    {
        packedStream.WriteByte(0);
    }

    return true;
}

bool EncodeE1(const uint8_t* pInput, const std::vector<Match>& parse, FormatOptions options, BitStream& packedStream)
{
    if (options.id != FormatId::E1)
    {
        return false;
    }

    packedStream.WriteReset();

    for (const Match& match: parse)
    {
        if (match.offset)
        {
            uint16_t offset = match.offset;
            if (options.extendOffset) offset--;

            EncodeElias1(packedStream, match.length - 1);
            packedStream.WriteBit(0);
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            pInput += match.length;
        }
        else
        {
            EncodeElias1(packedStream, match.length);
            packedStream.WriteBit(1);

            for (uint16_t b = 0; b < match.length; b++)
            {
                packedStream.WriteByte(*pInput++);
            }
        }
    }

    if (options.addEndMarker)
    {
        for (uint16_t i = 0; i < 16; i++)
        {
            packedStream.WriteBit(1);
        }

        packedStream.WriteBit(0);
    }

    return true;
}

bool EncodeE1ZX(const uint8_t* pInput, const std::vector<Match>& parse, FormatOptions options, BitStream& packedStream)
{
    if (options.id != FormatId::E1ZX)
    {
        return false;
    }

    packedStream.WriteReset();

    for (const Match& match: parse)
    {
        if (match.offset)
        {
            uint16_t offset = match.offset;
            if (options.extendOffset) offset--;

            EncodeElias1Neg(packedStream, match.length - 1);
            packedStream.WriteBitNeg(0);
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            pInput += match.length;
        }
        else
        {
            EncodeElias1Neg(packedStream, match.length);
            packedStream.WriteBitNeg(1);

            for (size_t b = 0; b < match.length; b++)
            {
                packedStream.WriteByte(*pInput++);
            }
        }
    }

    packedStream.FlushBitsNeg();

    return true;
}

bool EncodeUE2(const uint8_t* pInputStream, const std::vector<Match>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.id != FormatId::UE2)
    {
        return false;
    }

    packedStream.WriteReset();
    size_t i = 0;

    for (const Match& ref: refs)
    {
        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.extendOffset) offset--;

            packedStream.WriteBit(0);
            EncodeElias2(packedStream, ref.length);
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            i += ref.length;
        }
        else
        {
            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteBit(1);
                packedStream.WriteByte(pInputStream[i++]);
            }
        }
    }

    if (format.addEndMarker)
    {
        packedStream.WriteBit(0);

        for (size_t i = 0; i < 15; i++)
        {
            packedStream.WriteBit(1);
        }

        packedStream.WriteBit(0);
    }

    return true;
}

bool Compress(uint8_t* pInput, uint16_t inputSize, const Format& format, BitStream& packedStream)
{
    if (pInput == nullptr || inputSize == 0)
    {
        return false;
    }

    bool success = false;
    FormatOptions options = format.GetOptions();

    if (options.reverse)
    {
        std::reverse(pInput, pInput + inputSize);
    }

    std::vector<Match> parse;
    if (!Parse(parse, pInput, inputSize, format))
    {
        return false;
    }

    switch (options.id)
    {
        case FormatId::LZ:
            success = EncodeLZS(pInput, parse, options, packedStream);
            break;

        case FormatId::E1:
            success = EncodeE1(pInput, parse, options, packedStream);
            break;

        case FormatId::E1ZX:
            success = EncodeE1ZX(pInput, parse, options, packedStream);
            break;

        case FormatId::UE2:
            success = EncodeUE2(pInput, parse, options, packedStream);
            break;
    }

    if (!success)
    {
        return false;
    }

#ifdef VERIFY

    std::vector<uint8_t> unpackedStream;
    if (!Decompress(packedStream, options, inputSize, unpackedStream))
    {
        return false;
    }

    if (options.reverse)
    {
        std::reverse(unpackedStream.begin(), unpackedStream.end());
    }

    if (strncmp(reinterpret_cast<const char*>(pInput), reinterpret_cast<const char*>(unpackedStream.data()), inputSize))
    {
        assert(0);
        return false;
    }

#endif // VERIFY

    if (options.reverse)
    {
        packedStream.Reverse();
    }

    return true;
}
