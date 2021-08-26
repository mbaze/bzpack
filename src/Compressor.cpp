// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include "Compressor.h"
#include "OptimalParser.h"
#include "UniversalCodes.h"

#ifdef _DEBUG
#define VERIFY
#endif // _DEBUG

bool EncodeBlockElias(const uint8_t* pInputStream, size_t inputSize, const std::vector<StreamRef>& refs, uint32_t format, BitStream& packedStream)
{
    if (pInputStream == nullptr || inputSize == 0)
    {
        return false;
    }

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

            size_t offset = extendOffset ? ref.offset - 1 : ref.offset;

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

#ifdef VERIFY

    packedStream.ReadReset();
    std::vector<uint8_t> unpack;

    while (1)
    {
        size_t size = isElias1 ? DecodeElias1(packedStream) : DecodeElias2(packedStream);

        if (packedStream.ReadBit())
        {
            if (!isElias1) size--;

            for (size_t i = 0; i < size; i++)
            {
                unpack.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = packedStream.ReadByte();

            if (isElias1) size++;
            if (extendOffset) offset++;

            for (size_t i = 0; i < size; i++)
            {
                unpack.push_back(unpack[unpack.size() - offset]);
            }
        }

        if (unpack.size() >= inputSize)
        {
            break;
        }
    }

    if (strncmp((char*) pInputStream, (char*) unpack.data(), inputSize) != 0)
    {
        _ASSERT(0);
    }

#endif // VERIFY

    return true;
}

bool EncodeUnaryElias(const uint8_t* pInputStream, size_t inputSize, const std::vector<StreamRef>& refs, uint32_t format, BitStream& packedStream)
{
    if (pInputStream == nullptr || inputSize == 0)
    {
        return false;
    }

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

            size_t offset = extendOffset ? ref.offset - 1 : ref.offset;
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

#ifdef VERIFY

    packedStream.ReadReset();
    std::vector<uint8_t> unpack;

    while (1)
    {
        if (packedStream.ReadBit())
        {
            unpack.push_back(packedStream.ReadByte());
        }
        else
        {
            size_t size = isElias1 ? DecodeElias1(packedStream) + 1 : DecodeElias2(packedStream);

            size_t offset = packedStream.ReadByte();
            if (extendOffset) offset++;

            for (size_t i = 0; i < size; i++)
            {
                unpack.push_back(unpack[unpack.size() - offset]);
            }
        }

        if (unpack.size() >= inputSize)
        {
            break;
        }
    }

    if (strncmp((char*) pInputStream, (char*) unpack.data(), inputSize) != 0)
    {
        _ASSERT(0);
    }

#endif // VERIFY

    return true;
}

bool EncodeAlignedLZSS(const uint8_t* pInputStream, size_t inputSize, const std::vector<StreamRef>& refs, uint32_t format, BitStream& packedStream)
{
    if (pInputStream == nullptr || inputSize == 0)
    {
        return false;
    }

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
        size_t length = extendLength ? ref.length - 1 : ref.length;

        if (ref.offset)
        {
            size_t offset = extendOffset ? ref.offset - 1 : ref.offset;

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

#ifdef VERIFY

    packedStream.ReadReset();
    std::vector<uint8_t> unpack;

    while (1)
    {
        size_t length = packedStream.ReadByte();
        bool isLiteral = (length & 1);

        length >>= 1;
        if (extendLength) length++;

        if (isLiteral)
        {
            for (size_t i = 0; i < length; i++)
            {
                unpack.push_back(packedStream.ReadByte());
            }
        }
        else
        {
            size_t offset = packedStream.ReadByte();
            if (extendOffset) offset++;

            for (size_t i = 0; i < length; i++)
            {
                unpack.push_back(unpack[unpack.size() - offset]);
            }
        }

        if (unpack.size() >= inputSize)
        {
            break;
        }
    }

    if (strncmp((char*) pInputStream, (char*) unpack.data(), inputSize) != 0)
    {
        _ASSERT(0);
    }

#endif // VERIFY

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
    case Format::BlockElias1:
    case Format::BlockElias2:
        success = EncodeBlockElias(pInputStream, inputSize, streamRefs, format, packedStream);
        break;

    case Format::UnaryElias1:
    case Format::UnaryElias2:
        success = EncodeUnaryElias(pInputStream, inputSize, streamRefs, format, packedStream);
        break;

    case Format::AlignedLZSS:
        success = EncodeAlignedLZSS(pInputStream, inputSize, streamRefs, format, packedStream);
        break;
    }

    if (!success)
    {
        return false;
    }

    if (format & Format::FlagReverse)
    {
        packedStream.Reverse();
    }

    return true;
}
