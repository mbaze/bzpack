// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "Compression.h"
#include <algorithm>
#include "ExhaustiveParser.h"
#include "OptimalParser.h"
#include "UniversalCodes.h"

BitStream EncodeLZM(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::LZM || parse.empty())
        return {};

    BitStream stream;

    for (const ParseStep& parseStep: parse)
    {
        uint8_t length = parseStep.length - format.ExtendLength();

        if (parseStep.offset)
        {
            uint8_t offset = parseStep.offset - format.ExtendOffset();

            stream.WriteByte(length << 1);
            stream.WriteByte(offset);

            pInput += parseStep.length;
        }
        else
        {
            stream.WriteByte((length << 1) | 1);

            for (uint16_t i = 0; i < parseStep.length; i++)
            {
                stream.WriteByte(*pInput++);
            }
        }
    }

    if (format.EndMarker())
    {
        stream.WriteByte(0);
    }

    return stream;
}

BitStream EncodeEF8(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::EF8 || parse.empty())
        return {};

    BitStream stream(true);

    for (const ParseStep& parseStep: parse)
    {
        if (parseStep.offset)
        {
            uint8_t offset = parseStep.offset - format.ExtendOffset();

            EncodeElias(stream, parseStep.length - 1);
            stream.WriteBit(0);
            stream.WriteByte(offset);

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

    if (format.EndMarker())
    {
        EncodeElias(stream, 256);
    }

    stream.FlushBits();
    return stream;
}

BitStream EncodeBX0(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::BX0 || parse.empty())
        return {};

    BitStream stream(true);
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

    if (format.EndMarker())
    {
        stream.WriteBit(0);
        EncodeElias(stream, 255);
    }

    stream.FlushBits();
    return stream;
}

BitStream EncodeBX2(const uint8_t* pInput, const std::vector<ParseStep>& parse, const Format& format)
{
    if (format.Id() != FormatId::BX2 || parse.empty())
        return {};

    BitStream stream(true);
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

    if (format.EndMarker())
    {
        EncodeElias(stream, 1);
        stream.WriteBit(0);
        stream.WriteByte(0);
    }

    stream.FlushBits();
    return stream;
}

BitStream Compress(const uint8_t* pInput, uint32_t inputSize, const Format& format)
{
    if (pInput == nullptr || inputSize == 0)
        return {};

    BitStream stream;
    std::vector<ParseStep> parse;

    switch (format.Id())
    {
        case FormatId::LZM:
            parse = OptimalParser::Parse(pInput, inputSize, format);
            stream = EncodeLZM(pInput, parse, format);
            break;

        case FormatId::EF8:
            parse = OptimalParser::Parse(pInput, inputSize, format);
            stream = EncodeEF8(pInput, parse, format);
            break;

        case FormatId::BX0:
        {
//            DijkstraParser parser(pInput, inputSize, format);
            ExhaustiveParser parser;
            parse = parser.Parse(pInput, inputSize, format);
            stream = EncodeBX0(pInput, parse, format);
            break;
        }

        case FormatId::BX2:
        {
//            DijkstraParser parser(pInput, inputSize, format);
            ExhaustiveParser parser;
            parse = parser.Parse(pInput, inputSize, format);
            stream = EncodeBX2(pInput, parse, format);
            break;
        }
    }

    return stream;
}
