// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "Compression.h"
#include "UniversalCodes.h"

std::vector<uint8_t> DecodeLZ(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::LZ)
        return {};

    stream.ResetForRead();
    std::vector<uint8_t> data;

    while (true)
    {
        uint16_t length = stream.ReadByte();

        if (format.AddEndMarker() && length == 0)
            break;

        bool isLiteral = (length & 1);
        length >>= 1;

        if (format.ExtendLength())
        {
            length++;
        }

        if (isLiteral)
        {
            while (length--)
            {
                data.emplace_back(stream.ReadByte());
            }
        }
        else
        {
            uint16_t offset = stream.ReadByte() + format.ExtendOffset();

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

    stream.ResetForRead();
    std::vector<uint8_t> data;

    while (true)
    {
        uint16_t length = DecodeElias(stream);

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
            size_t offset = stream.ReadByte() + format.ExtendOffset();

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

    stream.ResetForRead();
    std::vector<uint8_t> data;

    while (true)
    {
        uint16_t length = DecodeElias(stream);

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
            uint16_t offset = stream.ReadByte() + format.ExtendOffset();

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

std::vector<uint8_t> DecodeBX0(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::BX0)
        return {};

    stream.ResetForRead();
    std::vector<uint8_t> data;

    uint16_t repOffset = 0;
    bool wasLiteral = false;

    while (true)
    {
        if (data.size() ? stream.ReadBit() : true)
        {
            uint16_t length = DecodeElias(stream);

            if (wasLiteral)
            {
                while (length--)
                {
                    data.emplace_back(data[data.size() - repOffset]);
                }
            }
            else
            {
                while (length--)
                {
                    data.emplace_back(stream.ReadByte());
                }
            }

            wasLiteral = !wasLiteral;
        }
        else
        {
            uint16_t offset = DecodeElias(stream) - 1;

            if (format.AddEndMarker() && (offset & 0x80))
                break;

            offset = (offset << 8) | stream.ReadByte();
            uint16_t length = DecodeEliasWithFlag(stream, offset & 1) + 1;
            offset = (offset >> 1) + format.ExtendOffset();

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

std::vector<uint8_t> DecodeBX2(BitStream& stream, const Format& format, uint16_t inputSize)
{
    if (format.Id() != FormatId::BX2)
        return {};

    stream.ResetForRead();
    std::vector<uint8_t> data;

    uint16_t repOffset = 0;
    bool wasLiteral = false;

    while (true)
    {
        uint16_t length = DecodeElias(stream);

        if (stream.ReadBit())
        {
            if (wasLiteral)
            {
                while (length--)
                {
                    data.emplace_back(data[data.size() - repOffset]);
                }
            }
            else
            {
                while (length--)
                {
                    data.emplace_back(stream.ReadByte());
                }
            }

            wasLiteral = !wasLiteral;
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

        case FormatId::BX0:
            data = DecodeBX0(stream, format, inputSize);
            break;

        case FormatId::BX2:
            data = DecodeBX2(stream, format, inputSize);
            break;
    }

    if (format.Reverse())
    {
        std::reverse(data.begin(), data.end());
    }

    return data;
}
