// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <cassert>
#include <algorithm>
#include "Compressor.h"
#include "OptimalParser.h"
#include "UniversalCodes.h"

#ifdef _DEBUG
#define VERIFY
#endif // _DEBUG

bool EncodeLZS(const uint8_t* pInputStream, const std::vector<StreamRef>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.Id != FormatId::Aligned_LZSS)
    {
        return false;
    }

    packedStream.WriteReset();
    size_t i = 0;

    for (const StreamRef& ref: refs)
    {
        size_t length = ref.length;
        if (format.ExtendLength) length--;

        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.ExtendOffset) offset--;

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

    if (format.AddEndMarker)
    {
        packedStream.WriteByte(0);
    }

    return true;
}

bool EncodeE1(const uint8_t* pInputStream, const std::vector<StreamRef>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.Id != FormatId::Elias1)
    {
        return false;
    }

    packedStream.WriteReset();
    size_t i = 0;

    for (const StreamRef& ref: refs)
    {
        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.ExtendOffset) offset--;

            EncodeElias1(packedStream, ref.length - 1);
            packedStream.WriteBit(0);
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            i += ref.length;
        }
        else
        {
            EncodeElias1(packedStream, ref.length);
            packedStream.WriteBit(1);

            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteByte(pInputStream[i++]);
            }
        }
    }

    if (format.AddEndMarker)
    {
        for (size_t i = 0; i < 16; i++)
        {
            packedStream.WriteBit(1);
        }

        packedStream.WriteBit(0);
    }

    return true;
}

bool EncodeE1ZX(const uint8_t* pInputStream, const std::vector<StreamRef>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.Id != FormatId::Elias1_ZX)
    {
        return false;
    }

    packedStream.WriteReset();
    size_t i = 0;

    for (const StreamRef& ref: refs)
    {
        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.ExtendOffset) offset--;

            EncodeElias1Neg(packedStream, ref.length - 1);
            packedStream.WriteBitNeg(0);
            packedStream.WriteByte(static_cast<uint8_t>(offset));

            i += ref.length;
        }
        else
        {
            EncodeElias1Neg(packedStream, ref.length);
            packedStream.WriteBitNeg(1);

            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteByte(pInputStream[i++]);
            }
        }
    }

    packedStream.FlushBitsNeg();

    return true;
}

bool EncodeE1X(const uint8_t* pInputStream, const std::vector<StreamRef>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.Id != FormatId::Elias1_Ext)
    {
        return false;
    }

    packedStream.WriteReset();

    size_t i = 0;
    bool wasPhrase = false;

    for (const StreamRef& ref: refs)
    {
        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.ExtendOffset) offset--;

            EncodeElias1(packedStream, ref.length - 1);

            if (wasPhrase)
            {
                packedStream.WriteBit(0);
            }
            else
            {
                bool longOffset = (offset > 255);
                packedStream.WriteBit(!longOffset);
            }

            packedStream.WriteByte(static_cast<uint8_t>(offset & 255));

            i += ref.length;
        }
        else
        {
            EncodeElias1(packedStream, ref.length);
            packedStream.WriteBit(1);

            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteByte(pInputStream[i++]);
            }
        }

        wasPhrase = (ref.offset > 0);
    }

    if (format.AddEndMarker)
    {
        for (size_t i = 0; i < 16; i++)
        {
            packedStream.WriteBit(1);
        }

        packedStream.WriteBit(0);
    }

    return true;
}

bool EncodeE1R(const uint8_t* pInputStream, const std::vector<StreamRef>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.Id != FormatId::Elias1_Rep)
    {
        return false;
    }

    packedStream.WriteReset();

    size_t i = 0;
    bool wasPhrase = false;
    uint32_t lastOffset = 0;

    for (const StreamRef& ref: refs)
    {
        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.ExtendOffset) offset--;

            EncodeElias1(packedStream, ref.length - 1);

            if (wasPhrase || (ref.offset != lastOffset))
            {
                packedStream.WriteBit(0);
                packedStream.WriteByte(static_cast<uint8_t>(offset));
                lastOffset = ref.offset;
            }
            else
            {
                packedStream.WriteBit(1);
            }

            i += ref.length;
        }
        else
        {
            EncodeElias1(packedStream, ref.length);
            packedStream.WriteBit(1);

            for (size_t b = 0; b < ref.length; b++)
            {
                packedStream.WriteByte(pInputStream[i++]);
            }
        }

        wasPhrase = (ref.offset > 0);
    }

    if (format.AddEndMarker)
    {
        for (size_t i = 0; i < 16; i++)
        {
            packedStream.WriteBit(1);
        }

        packedStream.WriteBit(0);
    }

    return true;
}

bool EncodeUE2(const uint8_t* pInputStream, const std::vector<StreamRef>& refs, FormatOptions format, BitStream& packedStream)
{
    if (format.Id != FormatId::Unary_Elias2)
    {
        return false;
    }

    packedStream.WriteReset();
    size_t i = 0;

    for (const StreamRef& ref: refs)
    {
        if (ref.offset)
        {
            size_t offset = ref.offset;
            if (format.ExtendOffset) offset--;

            packedStream.WriteBit(0);
            EncodeElias2(packedStream, ref.length);
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

    if (format.AddEndMarker)
    {
        packedStream.WriteBit(0);

        for (size_t i = 0; i < 15; i++)
        {
            packedStream.WriteBit(1);
        }

        packedStream.WriteBit(0);
    }

    return true;
}

bool Compress(uint8_t* pInputStream, size_t inputSize, FormatOptions format, BitStream& packedStream)
{
    bool success = false;

    if (pInputStream == nullptr || inputSize == 0)
    {
        return false;
    }

    if (format.Reverse)
    {
        std::reverse(pInputStream, pInputStream + inputSize);
    }

    std::vector<StreamRef> refs;
    if (!Parse(pInputStream, inputSize, format, refs))
    {
        return false;
    }

    switch (format.Id)
    {
        case FormatId::Aligned_LZSS:
            success = EncodeLZS(pInputStream, refs, format, packedStream);
            break;

        case FormatId::Elias1:
            success = EncodeE1(pInputStream, refs, format, packedStream);
            break;

        case FormatId::Elias1_ZX:
            success = EncodeE1ZX(pInputStream, refs, format, packedStream);
            break;

        case FormatId::Elias1_Ext:
            success = EncodeE1X(pInputStream, refs, format, packedStream);
            break;

        case FormatId::Elias1_Rep:
            success = EncodeE1R(pInputStream, refs, format, packedStream);
            break;

        case FormatId::Unary_Elias2:
            success = EncodeUE2(pInputStream, refs, format, packedStream);
            break;
    }

    if (!success)
    {
        return false;
    }

#ifdef VERIFY

    std::vector<uint8_t> unpackedStream;
    if (!Decompress(packedStream, format, inputSize, unpackedStream))
    {
        return false;
    }

    if (format.Reverse)
    {
        std::reverse(unpackedStream.begin(), unpackedStream.end());
    }

    if (strncmp(reinterpret_cast<const char*>(pInputStream), reinterpret_cast<const char*>(unpackedStream.data()), inputSize))
    {
        assert(0);
        return false;
    }

#endif // VERIFY

    if (format.Reverse)
    {
        packedStream.Reverse();
    }

    return true;
}
