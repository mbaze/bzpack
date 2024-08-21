// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "Compressor.h"
#include "UniversalCodes.h"

bool DecodeLZS(BitStream& packedStream, FormatOptions options, uint16_t inputSize, std::vector<uint8_t>& output)
{
    if (options.id != FormatId::LZ)
    {
        return false;
    }

    output.clear();
    packedStream.ReadReset();
    bool checkEndMarker = (inputSize == 0);

    while (true)
    {
        uint16_t length = packedStream.ReadByte();

        if (checkEndMarker && length == 0)
        {
            break;
        }

        bool isLiteral = (length & 1);
        length >>= 1;
        if (options.extendLength) length++;

        if (isLiteral)
        {
            while (length--)
            {
                output.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            uint16_t offset = packedStream.ReadByte();
            if (options.extendOffset) offset++;

            while (length--)
            {
                output.push_back(output[output.size() - offset]);
            }
        }

        if (!checkEndMarker && output.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1(BitStream& packedStream, FormatOptions options, uint16_t inputSize, std::vector<uint8_t>& output)
{
    if (options.id != FormatId::E1)
    {
        return false;
    }

    output.clear();
    packedStream.ReadReset();
    bool checkEndMarker = (inputSize == 0);

    while (true)
    {
        uint16_t length = DecodeElias1(packedStream);

        if (checkEndMarker && length > 255)
        {
            break;
        }

        if (packedStream.ReadBit())
        {
            while (length--)
            {
                output.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            length++;
            size_t offset = packedStream.ReadByte();
            if (options.extendOffset) offset++;

            while (length--)
            {
                output.push_back(output[output.size() - offset]);
            }
        }

        if (!checkEndMarker && output.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1ZX(BitStream& packedStream, FormatOptions options, uint16_t inputSize, std::vector<uint8_t>& output)
{
    if (options.id != FormatId::E1ZX)
    {
        return false;
    }

    output.clear();
    packedStream.ReadReset();

    while (true)
    {
        uint16_t length = DecodeElias1Neg(packedStream);

        if (packedStream.ReadBitNeg())
        {
            while (length--)
            {
                output.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            length++;
            uint16_t offset = packedStream.ReadByte();
            if (options.extendOffset) offset++;

            while (length--)
            {
                output.push_back(output[output.size() - offset]);
            }
        }

        if (output.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeUE2(BitStream& packedStream, FormatOptions options, size_t inputSize, std::vector<uint8_t>& output)
{
    if (options.id != FormatId::UE2)
    {
        return false;
    }

    output.clear();
    packedStream.ReadReset();
    bool checkEndMarker = (inputSize == 0);

    while (true)
    {
        if (packedStream.ReadBit())
        {
            output.push_back(packedStream.ReadByte());
        }
        else
        {
            uint16_t length = DecodeElias2(packedStream);

            if (checkEndMarker && length > 255)
            {
                break;
            }

            uint16_t offset = packedStream.ReadByte();
            if (options.extendOffset) offset++;

            while (length--)
            {
                output.push_back(output[output.size() - offset]);
            }
        }

        if (!checkEndMarker && output.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

// Zero inputSize tells the decompressor to expect the end-of-stream marker.

bool Decompress(BitStream& packedStream, FormatOptions options, uint16_t inputSize, std::vector<uint8_t>& output)
{
    bool success = false;
    output.clear();

    switch (options.id)
    {
        case FormatId::LZ:
            success = DecodeLZS(packedStream, options, inputSize, output);
            break;

        case FormatId::E1:
            success = DecodeE1(packedStream, options, inputSize, output);
            break;

        case FormatId::E1ZX:
            success = DecodeE1ZX(packedStream, options, inputSize, output);
            break;

        case FormatId::UE2:
            success = DecodeUE2(packedStream, options, inputSize, output);
            break;
    }

    if (!success)
    {
        return false;
    }

    if (options.reverse)
    {
        std::reverse(output.begin(), output.end());
    }

    return true;
}
