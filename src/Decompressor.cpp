// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <algorithm>
#include "Compressor.h"
#include "UniversalCodes.h"

bool DecodeLZS(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    if ((format & Format::Mask) != Format::Aligned_LZSS)
    {
        return false;
    }

    bool checkEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    bool extendLength = (format & Format::FlagExtendLength);

    outputStream.clear();
    packedStream.ReadReset();

    while (true)
    {
        size_t length = packedStream.ReadByte();

        if (checkEndMarker && length == 0)
        {
            break;
        }

        bool isLiteral = (length & 1);

        length >>= 1;
        if (extendLength) length++;

        if (isLiteral)
        {
            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = packedStream.ReadByte();
            if (extendOffset) offset++;

            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(outputStream[outputStream.size() - offset]);
            }
        }

        if (!checkEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1E1(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    if ((format & Format::Mask) != Format::Elias1_Elias1)
    {
        return false;
    }

    bool checkEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);

    outputStream.clear();
    packedStream.ReadReset();

    while (true)
    {
        size_t length = DecodeElias1(packedStream);

        if (checkEndMarker && length > 255)
        {
            break;
        }

        if (packedStream.ReadBit())
        {
            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = packedStream.ReadByte();
            if (extendOffset) offset++;

            for (size_t i = 0; i <= length; i++)
            {
                outputStream.push_back(outputStream[outputStream.size() - offset]);
            }
        }

        if (!checkEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1X1(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    if ((format & Format::Mask) != Format::Elias1_Elias1_X)
    {
        return false;
    }

    bool checkEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);

    outputStream.clear();
    packedStream.ReadReset();

    bool outputLiteral = false;

    while (true)
    {
        uint32_t offsetBit = 0;
        size_t length = DecodeElias1(packedStream);

        if (checkEndMarker && length > 255)
        {
            break;
        }

        if (outputLiteral)
        {
            outputLiteral = false;
            offsetBit = !packedStream.ReadBit() << 8;
        }
        else
        {
            outputLiteral = packedStream.ReadBit();
        }

        if (outputLiteral)
        {
            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = offsetBit | packedStream.ReadByte();
            if (extendOffset) offset++;

            for (size_t i = 0; i <= length; i++)
            {
                outputStream.push_back(outputStream[outputStream.size() - offset]);
            }
        }

        if (!checkEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1R1(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    if ((format & Format::Mask) != Format::Elias1_Elias1_R)
    {
        return false;
    }

    bool checkEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);

    outputStream.clear();
    packedStream.ReadReset();

    size_t prevOffset = 0;
    bool outputLiteral = false;

    while (true)
    {
        bool reuseOffset = false;
        size_t length = DecodeElias1(packedStream);

        if (checkEndMarker && length > 255)
        {
            break;
        }

        if (outputLiteral)
        {
            outputLiteral = false;
            reuseOffset = packedStream.ReadBit();
        }
        else
        {
            outputLiteral = packedStream.ReadBit();
            reuseOffset = false;
        }

        if (outputLiteral)
        {
            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = reuseOffset ? prevOffset : packedStream.ReadByte();
            prevOffset = offset;
            if (extendOffset) offset++;

            for (size_t i = 0; i <= length; i++)
            {
                outputStream.push_back(outputStream[outputStream.size() - offset]);
            }
        }

        if (!checkEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeUE2(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    if ((format & Format::Mask) != Format::Unary_Elias2)
    {
        return false;
    }

    bool checkEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);

    outputStream.clear();
    packedStream.ReadReset();

    while (true)
    {
        if (packedStream.ReadBit())
        {
            outputStream.push_back(packedStream.ReadByte());
        }
        else
        {
            size_t length = DecodeElias2(packedStream);

            if (checkEndMarker && length > 255)
            {
                break;
            }

            size_t offset = packedStream.ReadByte();
            if (extendOffset) offset++;

            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(outputStream[outputStream.size() - offset]);
            }
        }

        if (!checkEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool Decompress(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool success = false;
    outputStream.clear();

    switch (format & Format::Mask)
    {
        case Format::Aligned_LZSS:
            success = DecodeLZS(packedStream, format, inputSize, outputStream);
            break;

        case Format::Elias1_Elias1:
            success = DecodeE1E1(packedStream, format, inputSize, outputStream);
            break;

        case Format::Elias1_Elias1_X:
            success = DecodeE1X1(packedStream, format, inputSize, outputStream);
            break;

        case Format::Elias1_Elias1_R:
            success = DecodeE1R1(packedStream, format, inputSize, outputStream);
            break;

        case Format::Unary_Elias2:
            success = DecodeUE2(packedStream, format, inputSize, outputStream);
            break;
    }

    if (!success)
    {
        return false;
    }

    if (format & Format::FlagReverse)
    {
        std::reverse(outputStream.begin(), outputStream.end());
    }

    return true;
}
