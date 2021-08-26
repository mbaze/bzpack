// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "Compressor.h"
#include "OptimalParser.h"
#include "UniversalCodes.h"

#ifdef _DEBUG
#define VERIFY
#endif // _DEBUG

bool EncodeAlignedLZSS(const uint8_t* pInputStream, size_t inputSize, const std::vector<StreamRef>& refs, uint32_t format, BitStream& packedStream)
{
    bool addEndMarker = (format & Format::FlagAddEndMarker);
    bool extendOffset = (format & Format::FlagExtendOffset);
    bool extendLength = (format & Format::FlagExtendLength);
    format &= Format::Mask;

    if (format != Format::AlignedLZSS)
    {
        return false;
    }

    size_t i = 0;
    packedStream.WriteReset();

    for (const StreamRef& ref : refs)
    {
        size_t length = ref.length;
        if (extendLength) length--;

        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (extendOffset) offset--;

            packedStream.WriteByte(static_cast<uint8_t>(length << 1));
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            i += ref.length;
        }
        else
        {
            packedStream.WriteByte(static_cast<uint8_t>(length << 1) | 1);

            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteByte(pInputStream[i++]);
            }
        }
    }

    if (addEndMarker)
    {
        packedStream.WriteByte(0);
    }

    return true;
}

bool EncodeBlockElias(const uint8_t* pInputStream, size_t inputSize, const std::vector<StreamRef>& refs, uint32_t format, BitStream& packedStream)
{
    bool addEndMarker = (format & Format::FlagAddEndMarker);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::BlockElias1 && format != Format::BlockElias2)
    {
        return false;
    }

    size_t i = 0;
    packedStream.WriteReset();
    bool isElias1 = (format == Format::BlockElias1);

    for (const StreamRef& ref : refs)
    {
        if (ref.offset)
        {
            if (isElias1)
            {
                EncodeElias1(packedStream, ref.length - 1);
            }
            else
            {
                EncodeElias2(packedStream, ref.length);
            }

            size_t offset = ref.offset;
            if (extendOffset) offset--;

            packedStream.WriteBit(0);
            packedStream.WriteByte(static_cast<uint8_t>(offset));
            i += ref.length;
        }
        else
        {
            if (isElias1)
            {
                EncodeElias1(packedStream, ref.length);
            }
            else
            {
                EncodeElias2(packedStream, ref.length + 1);
            }

            packedStream.WriteBit(1);

            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteByte(pInputStream[i++]);
            }
        }
    }

    if (addEndMarker)
    {
        for (size_t i = 0; i < 15; i++)
        {
            packedStream.WriteBit(1);
        }
    }

    return true;
}

bool EncodeUnaryElias(const uint8_t* pInputStream, size_t inputSize, const std::vector<StreamRef>& refs, uint32_t format, BitStream& packedStream)
{
    bool addEndMarker = (format & Format::FlagAddEndMarker);
    bool extendOffset = (format & Format::FlagExtendOffset);
    format &= Format::Mask;

    if (format != Format::UnaryElias1 && format != Format::UnaryElias2)
    {
        return false;
    }

    size_t i = 0;
    packedStream.WriteReset();
    bool isElias1 = (format == Format::BlockElias1);

    for (const StreamRef& ref : refs)
    {
        if (ref.offset)
        {
            packedStream.WriteBit(0);

            if (isElias1)
            {
                EncodeElias1(packedStream, ref.length - 1);
            }
            else
            {
                EncodeElias2(packedStream, ref.length);
            }

            size_t offset = ref.offset;
            if (extendOffset) offset--;

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

    if (addEndMarker)
    {
        for (size_t i = 0; i < 14; i++)
        {
            packedStream.WriteBit(1);
        }
    }

    return true;
}

bool Compress(uint8_t* pInputStream, size_t inputSize, uint32_t format, BitStream& packedStream)
{
    if (pInputStream == nullptr || inputSize == 0)
    {
        return false;
    }

    if (format & Format::FlagReverse)
    {
        std::reverse(pInputStream, pInputStream + inputSize);
    }

    std::vector<StreamRef> streamRefs;
    if (Parse(pInputStream, inputSize, format, streamRefs) == false)
    {
        return false;
    }

    bool success = false;

    switch (format & Format::Mask)
    {
    case Format::AlignedLZSS:
        success = EncodeAlignedLZSS(pInputStream, inputSize, streamRefs, format, packedStream);
        break;

    case Format::BlockElias1:
    case Format::BlockElias2:
        success = EncodeBlockElias(pInputStream, inputSize, streamRefs, format, packedStream);
        break;

    case Format::UnaryElias1:
    case Format::UnaryElias2:
        success = EncodeUnaryElias(pInputStream, inputSize, streamRefs, format, packedStream);
        break;
    }

    if (!success)
    {
        return false;
    }

#ifdef VERIFY

    std::vector<uint8_t> unpackedStream;
    if (Decompress(packedStream, format, inputSize, unpackedStream) == false)
    {
        return false;
    }

    if (strncmp((char*) pInputStream, (char*) unpackedStream.data(), inputSize) != 0)
    {
        _ASSERT(0);
        return false;
    }

#endif // VERIFY

    if (format & Format::FlagReverse)
    {
        packedStream.Reverse();
    }

    return true;
}
