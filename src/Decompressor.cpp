// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <algorithm>
#include "Compressor.h"
#include "UniversalCodes.h"

bool DecodeLZS(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool testEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    bool extendLength = (format & Format::FlagExtendLength);
    format &= Format::Mask;

    if (format != Format::AlignedLZSS)
    {
        return false;
    }

    outputStream.clear();
    packedStream.ReadReset();

    while (true)
    {
        size_t length = packedStream.ReadByte();

        if (testEndMarker && length == 0)
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

        if (!testEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1E1(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool testEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::Elias1_Elias1)
    {
        return false;
    }

    outputStream.clear();
    packedStream.ReadReset();

    while (true)
    {
        size_t length = DecodeElias1(packedStream);

        if (testEndMarker && length > 255)
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

        if (!testEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeE1X1(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool testEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::Elias1_ExtElias1)
    {
        return false;
    }

    outputStream.clear();
    packedStream.ReadReset();

    uint32_t offsetBit;
    bool outputLiteral = false;

    while (true)
    {
        size_t length = DecodeElias1(packedStream);

        if (testEndMarker && length > 255)
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
            offsetBit = 0;
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

        if (!testEndMarker && outputStream.size() >= inputSize)
        {
            break;
        }
    }

    return true;
}

bool DecodeUE2(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool testEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::Unary_Elias2)
    {
        return false;
    }

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

            if (testEndMarker && length > 255)
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

        if (!testEndMarker && outputStream.size() >= inputSize)
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
    case Format::AlignedLZSS:
        success = DecodeLZS(packedStream, format, inputSize, outputStream);
        break;

    case Format::Elias1_Elias1:
        success = DecodeE1E1(packedStream, format, inputSize, outputStream);
        break;

    case Format::Elias1_ExtElias1:
        success = DecodeE1X1(packedStream, format, inputSize, outputStream);
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
