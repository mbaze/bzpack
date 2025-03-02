// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <cassert>
#include <algorithm>
#include "Compressor.h"
#include "OptimalParser.h"
#include "DijkstraParser.h"
#include "UniversalCodes.h"

#ifdef _DEBUG
#define VERIFY
#endif // _DEBUG

BitStream EncodeLZ(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::LZ || !parse.size())
        return {};

    BitStream stream;

    for (const ParseStep& parseStep: parse)
    {
        uint16_t length = format.ExtendLength() ? parseStep.length - 1 : parseStep.length;

        if (parseStep.offset)
        {
            uint16_t offset = format.ExtendOffset() ? parseStep.offset - 1 : parseStep.offset;

            stream.WriteByte(static_cast<uint8_t>(length << 1));
            stream.WriteByte(static_cast<uint8_t>(offset));

            pInput += parseStep.length;
        }
        else
        {
            stream.WriteByte(static_cast<uint8_t>(length << 1) | 1);

            for (uint16_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }
        }
    }

    if (format.AddEndMarker())
    {
        stream.WriteByte(0);
    }

    return stream;
}

BitStream EncodeZX2(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::ZX2 || !parse.size())
        return {};

    BitStream stream;

    uint16_t repOffset = 0;
    bool wasLiteral = false;

    for (const ParseStep& parseStep: parse)
    {
        if (parseStep.offset)
        {
            if (wasLiteral && (parseStep.offset == repOffset))
            {
                stream.WriteBit(1);
                EncodeElias1(stream, parseStep.length);
            }
            else
            {
                uint16_t offset = format.ExtendOffset() ? parseStep.offset - 1 : parseStep.offset;

                stream.WriteBit(0);
                stream.WriteByte(static_cast<uint8_t>(offset));
                EncodeElias1(stream, parseStep.length - 1);
            }

            pInput += parseStep.length;
            repOffset = parseStep.offset;
            wasLiteral = false;
        }
        else
        {
            if (stream.Size())
            {
                stream.WriteBit(1);
            }

            EncodeElias1(stream, parseStep.length);

            for (uint16_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }

            wasLiteral = true;
        }
    }

    if (format.AddEndMarker())
    {
        stream.WriteByte(255);
    }

    return stream;
}

BitStream EncodeE1(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::E1 || !parse.size())
        return {};

    BitStream stream;

    for (const ParseStep& parseStep: parse)
    {
        if (parseStep.offset)
        {
            uint16_t offset = format.ExtendOffset() ? parseStep.offset - 1 : parseStep.offset;

            EncodeElias1(stream, parseStep.length - 1);
            stream.WriteBit(0);
            stream.WriteByte(static_cast<uint8_t>(offset));

            pInput += parseStep.length;
        }
        else
        {
            EncodeElias1(stream, parseStep.length);
            stream.WriteBit(1);

            for (uint16_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }
        }
    }

    if (format.AddEndMarker())
    {
        for (uint16_t i = 0; i < 16; i++)
        {
            stream.WriteBit(1);
        }

        stream.WriteBit(0);
    }

    return stream;
}

BitStream EncodeE1ZX(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::E1ZX || !parse.size())
        return {};

    BitStream stream;

    for (const ParseStep& parseStep: parse)
    {
        if (parseStep.offset)
        {
            uint16_t offset = format.ExtendOffset() ? parseStep.offset - 1 : parseStep.offset;

            EncodeElias1Neg(stream, parseStep.length - 1);
            stream.WriteBitNeg(0);
            stream.WriteByte(static_cast<uint8_t>(offset));

            pInput += parseStep.length;
        }
        else
        {
            EncodeElias1Neg(stream, parseStep.length);
            stream.WriteBitNeg(1);

            for (size_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }
        }
    }

    stream.FlushBitsNeg();

    return stream;
}

BitStream EncodeUE2(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::UE2 || !parse.size())
        return {};

    BitStream stream;

    for (const ParseStep& parseStep: parse)
    {
        if (parseStep.offset)
        {
            uint16_t offset = format.ExtendOffset() ? parseStep.offset - 1 : parseStep.offset;

            stream.WriteBit(0);
            EncodeElias2(stream, parseStep.length);
            stream.WriteByte(static_cast<uint8_t>(offset));

            pInput += parseStep.length;
        }
        else
        {
            for (size_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteBit(1);
                stream.WriteByte(*pInput++);
            }
        }
    }

    if (format.AddEndMarker())
    {
        stream.WriteBit(0);

        for (size_t i = 0; i < 15; i++)
        {
            stream.WriteBit(1);
        }

        stream.WriteBit(0);
    }

    return stream;
}

BitStream Compress(uint8_t* pInput, uint16_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    if (format.Reverse())
        std::reverse(pInput, pInput + inputSize);

    BitStream stream;
    std::vector<ParseStep> parse;

    switch (format.Id())
    {
        case FormatId::LZ:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeLZ(pInput, parse, format);
            break;

        case FormatId::ZX2:
        {
            DijkstraParser parser(pInput, inputSize, format);
            parse = parser.Parse();
            stream = EncodeZX2(pInput, parse, format);
            break;
        }

        case FormatId::E1:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeE1(pInput, parse, format);
            break;

        case FormatId::E1ZX:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeE1ZX(pInput, parse, format);
            break;

        case FormatId::UE2:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeUE2(pInput, parse, format);
            break;
    }

    if (stream.Size() == 0)
        return {};

#ifdef VERIFY

    std::vector<uint8_t> data = Decompress(stream, format, inputSize);
    if (data.size() == 0)
        return {};

    if (format.Reverse())
        std::reverse(data.begin(), data.end());

    for (uint16_t i = 0; i < inputSize; i++)
    {
        if (pInput[i] != data.data()[i])
        {
            assert(0);
            return {};
        }
    }

#endif // VERIFY

    if (format.Reverse())
        stream.Reverse();

    return stream;
}
