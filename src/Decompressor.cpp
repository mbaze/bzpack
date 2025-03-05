// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "CommonTypes.h"
#include "Compressor.h"
#include "UniversalCodes.h"

std::vector<uint8_t> DecodeLZ(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::LZ)
        return {};

    std::vector<uint8_t> data;
    stream.ReadReset();

    while (true)
    {
        uint16_t length = stream.ReadByte();

        if (format.AddEndMarker() && length == 0)
            break;

        bool isLiteral = (length & 1);

        length >>= 1;
        if (format.ExtendLength())
            length++;

        if (isLiteral)
        {
            while (length--)
            {
                data.emplace_back(stream.ReadByte());
            }
        }
        else
        {
            uint16_t offset = stream.ReadByte();
            if (format.ExtendOffset())
                offset++;

            while (length--)
            {
                data.emplace_back(data[data.size() - offset]);
            }
        }

        if (!format.AddEndMarker() && data.size() >= inputSize)
            break;
    }

    return data;
}

std::vector<uint8_t> DecodeE1(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::E1)
        return {};

    std::vector<uint8_t> data;
    stream.ReadReset();

    while (true)
    {
        uint16_t length = DecodeElias1(stream);

        if (format.AddEndMarker() && length > 255)
            break;

        if (stream.ReadBit())
        {
            while (length--)
            {
                data.emplace_back(stream.ReadByte());
            }
        }
        else
        {
            length++;

            size_t offset = stream.ReadByte();
            if (format.ExtendOffset())
                offset++;

            while (length--)
            {
                data.emplace_back(data[data.size() - offset]);
            }
        }

        if (!format.AddEndMarker() && data.size() >= inputSize)
            break;
    }

    return data;
}

std::vector<uint8_t> DecodeE1ZX(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::E1ZX)
        return {};

    std::vector<uint8_t> data;
    stream.ReadReset();

    while (true)
    {
        uint16_t length = DecodeElias1Neg(stream);

        if (stream.ReadBitNeg())
        {
            while (length--)
            {
                data.emplace_back(stream.ReadByte());
            }
        }
        else
        {
            length++;

            uint16_t offset = stream.ReadByte();
            if (format.ExtendOffset())
                offset++;

            while (length--)
            {
                data.emplace_back(data[data.size() - offset]);
            }
        }

        if (data.size() >= inputSize)
            break;
    }

    return data;
}

std::vector<uint8_t> DecodeBX2(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::BX2)
        return {};

    std::vector<uint8_t> data;
    stream.ReadReset();

    uint16_t repOffset = 0;
    bool wasLiteral = false;

    while (true)
    {
        uint16_t length = DecodeElias1(stream);

        if (stream.ReadBit())
        {
            if (wasLiteral)
            {
                while (length--)
                {
                    data.emplace_back(data[data.size() - repOffset]);
                }

                wasLiteral = false;
            }
            else
            {
                while (length--)
                {
                    data.emplace_back(stream.ReadByte());
                }

                wasLiteral = true;
            }
        }
        else
        {
            length++;
            uint16_t offset = stream.ReadByte();

            if (format.AddEndMarker() && offset == 0)
                break;

            while (length--)
            {
                data.emplace_back(data[data.size() - offset]);
            }

            repOffset = offset;
            wasLiteral = false;
        }

        if (!format.AddEndMarker() && data.size() >= inputSize)
            break;
    }

    return data;
}

std::vector<uint8_t> DecodeUE2(BitStream& stream, const Format& format, size_t inputSize)
{
    if (format.Id() != FormatId::UE2)
        return {};

    stream.ReadReset();
    std::vector<uint8_t> data;

    while (true)
    {
        if (stream.ReadBit())
        {
            data.emplace_back(stream.ReadByte());
        }
        else
        {
            uint16_t length = DecodeElias2(stream);

            if (format.AddEndMarker() && length > 255)
            {
                break;
            }

            uint16_t offset = stream.ReadByte();
            if (format.ExtendOffset())
                offset++;

            while (length--)
            {
                data.emplace_back(data[data.size() - offset]);
            }
        }

        if (!format.AddEndMarker() && data.size() >= inputSize)
            break;
    }

    return data;
}

std::vector<uint8_t> Decompress(BitStream& stream, const Format& format, uint16_t inputSize)
{
    std::vector<uint8_t> data;

    switch (format.Id())
    {
        case FormatId::LZ:
            data = DecodeLZ(stream, format, inputSize);
            break;

        case FormatId::E1:
            data = DecodeE1(stream, format, inputSize);
            break;

        case FormatId::E1ZX:
            data = DecodeE1ZX(stream, format, inputSize);
            break;

        case FormatId::BX2:
            data = DecodeBX2(stream, format, inputSize);
            break;

        case FormatId::UE2:
            data = DecodeUE2(stream, format, inputSize);
            break;
    }

    if (format.Reverse())
        std::reverse(data.begin(), data.end());

    return data;
}
