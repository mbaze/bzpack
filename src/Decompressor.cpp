// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <algorithm>
#include "Compressor.h"
#include "UniversalCodes.h"

bool DecodeAlignedLZSS(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
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

    while (1)
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

bool DecodeBlockElias(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool testEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::BlockElias1 && format != Format::BlockElias2)
    {
        return false;
    }

    bool isElias1 = (format == Format::BlockElias1);

    outputStream.clear();
    packedStream.ReadReset();

    while (1)
    {
        size_t length = isElias1 ? DecodeElias1(packedStream) : DecodeElias2(packedStream);

        if (testEndMarker && length > 255)
        {
            break;
        }

        if (packedStream.ReadBit())
        {
            if (!isElias1) length--;

            for (size_t i = 0; i < length; i++)
            {
                outputStream.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = packedStream.ReadByte();

            if (isElias1) length++;
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

bool DecodeUnaryElias(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream)
{
    bool testEndMarker = (inputSize == 0);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::UnaryElias1 && format != Format::UnaryElias2)
    {
        return false;
    }

    bool isElias1 = (format == Format::UnaryElias1);

    outputStream.clear();
    packedStream.ReadReset();

    while (1)
    {
        if (packedStream.ReadBit())
        {
            outputStream.push_back(packedStream.ReadByte());
        }
        else
        {
            size_t length = isElias1 ? DecodeElias1(packedStream) : DecodeElias2(packedStream);

            if (testEndMarker && length > 255)
            {
                break;
            }

            if (isElias1) length++;
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
    outputStream.clear();
    bool success = false;

    switch (format & Format::Mask)
    {
    case Format::AlignedLZSS:
        success = DecodeAlignedLZSS(packedStream, format, inputSize, outputStream);
        break;

    case Format::BlockElias1:
    case Format::BlockElias2:
        success = DecodeBlockElias(packedStream, format, inputSize, outputStream);
        break;

    case Format::UnaryElias1:
    case Format::UnaryElias2:
        success = DecodeUnaryElias(packedStream, format, inputSize, outputStream);
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
