// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <cassert>
#include <algorithm>
#include "Compression.h"
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
            uint16_t offset = parseStep.offset - format.ExtendOffset();

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

BitStream EncodeE1(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::E1 || !parse.size())
        return {};

    BitStream stream;

    for (const ParseStep& parseStep: parse)
    {
        if (parseStep.offset)
        {
            uint16_t offset = parseStep.offset - format.ExtendOffset();

            EncodeElias(stream, parseStep.length - 1);
            stream.WriteBit(0);
            stream.WriteByte(static_cast<uint8_t>(offset));

            pInput += parseStep.length;
        }
        else
        {
            EncodeElias(stream, parseStep.length);
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
            uint16_t offset = parseStep.offset - format.ExtendOffset();

            EncodeEliasNeg(stream, parseStep.length - 1);
            stream.WriteBitNeg(0);
            stream.WriteByte(static_cast<uint8_t>(offset));

            pInput += parseStep.length;
        }
        else
        {
            EncodeEliasNeg(stream, parseStep.length);
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

BitStream EncodeBX0(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::BX0 || !parse.size())
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
                EncodeElias(stream, parseStep.length);
            }
            else
            {
                uint16_t offset = parseStep.offset - format.ExtendOffset();
                uint8_t flag = (parseStep.length > 2);

                stream.WriteBit(0);
                EncodeElias(stream, FormatBX0::GetEliasPart(offset));
                stream.WriteByte((FormatBX0::GetRawPart(offset) << 1) | flag);
                EncodeEliasWithoutFlag(stream, parseStep.length - 1);
            }

            pInput += parseStep.length;
            repOffset = parseStep.offset;
        }
        else
        {
            if (stream.Size())
            {
                stream.WriteBit(1);
            }

            EncodeElias(stream, parseStep.length);

            for (uint16_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }
        }

        wasLiteral = !parseStep.offset;
    }

    if (format.AddEndMarker())
    {
        stream.WriteBit(0);
        EncodeElias(stream, 255);
    }

    return stream;
}

BitStream EncodeBX2(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::BX2 || !parse.size())
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
                EncodeElias(stream, parseStep.length);
                stream.WriteBit(1);
            }
            else
            {
                EncodeElias(stream, parseStep.length - 1);
                stream.WriteBit(0);
                stream.WriteByte(static_cast<uint8_t>(parseStep.offset));
            }

            pInput += parseStep.length;
            repOffset = parseStep.offset;
        }
        else
        {
            EncodeElias(stream, parseStep.length);
            stream.WriteBit(1);

            for (uint16_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }
        }

        wasLiteral = !parseStep.offset;
    }

    if (format.AddEndMarker())
    {
        EncodeElias(stream, 1);
        stream.WriteBit(0);
        stream.WriteByte(0);
    }

    return stream;
}

BitStream Compress(uint8_t* pInput, uint16_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    if (format.Reverse())
    {
        std::reverse(pInput, pInput + inputSize);
    }

    BitStream stream;
    std::vector<ParseStep> parse;

    switch (format.Id())
    {
        case FormatId::LZ:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeLZ(pInput, parse, format);
            break;

        case FormatId::E1:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeE1(pInput, parse, format);
            break;

        case FormatId::E1ZX:
            parse = Parse(pInput, inputSize, format);
            stream = EncodeE1ZX(pInput, parse, format);
            break;

        case FormatId::BX0:
        {
            DijkstraParser parser(pInput, inputSize, format);
            parse = parser.Parse();
            stream = EncodeBX0(pInput, parse, format);
            break;
        }

        case FormatId::BX2:
        {
            DijkstraParser parser(pInput, inputSize, format);
            parse = parser.Parse();
            stream = EncodeBX2(pInput, parse, format);
            break;
        }
    }

    if (stream.Size() == 0)
        return {};

#ifdef VERIFY

    std::vector<uint8_t> data = Decompress(stream, format, inputSize);
    if (data.size() == 0)
        return {};

    if (format.Reverse())
    {
        std::reverse(data.begin(), data.end());
    }

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
    {
        stream.Reverse();
    }

    return stream;
}
