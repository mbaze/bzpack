// Copyright (c) 2024, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef FORMATS_H
#define FORMATS_H

#include "UniversalCodes.h"

enum FormatId
{
    LZ,
    E1,
    E1ZX,
    ZX2,
    UE2,
};

struct FormatOptions
{
    uint8_t id: 4;
    uint8_t reverse: 1;
    uint8_t addEndMarker: 1;
    uint8_t extendOffset: 1;
    uint8_t extendLength: 1;
};

class Format
{
public:

    Format() = delete;

    Format(FormatOptions options):
        mFormatId(static_cast<FormatId>(options.id)),
        mReverse(options.reverse),
        mAddEndMarker(options.addEndMarker),
        mExtendOffset(options.extendOffset),
        mExtendLength(options.extendLength)
    {}

    virtual ~Format() = default;
    virtual uint32_t GetLiteralCost(uint16_t length) const = 0;
    virtual uint32_t GetMatchCost(uint16_t length, uint16_t offset) const = 0;
    virtual uint32_t GetMatchCost(uint16_t length, uint16_t offset, uint16_t repOffset) const = 0;

    FormatId Id() const { return mFormatId; }

    uint16_t MaxLiteralLength() const { return mMaxLiteralLength; }
    uint16_t MinMatchLength() const { return mMinMatchLength; }
    uint16_t MaxMatchLength() const { return mMaxMatchLength; }
    uint16_t MaxMatchOffset() const { return mMaxMatchOffset; }

    bool Reverse() const { return mReverse; }
    bool AddEndMarker() const { return mAddEndMarker; }
    bool ExtendOffset() const { return mExtendOffset; }
    bool ExtendLength() const { return mExtendLength; }

protected:

    FormatId mFormatId;

    // Format limits.

    uint16_t mMaxLiteralLength = 0xFFFF;
    uint16_t mMinMatchLength = 2;
    uint16_t mMaxMatchLength = 0xFFFF;
    uint16_t mMaxMatchOffset = 0xFFFF;

    // Encoding options.

    bool mReverse;
    bool mAddEndMarker;
    bool mExtendOffset;
    bool mExtendLength;
};

class FormatLZ: public Format
{
public:

    FormatLZ() = delete;

    FormatLZ(FormatOptions options): Format{options}
    {
        mMaxLiteralLength = options.extendLength ? 128 : 127;
        mMinMatchLength = 2;
        mMaxMatchLength = options.extendLength ? 128 : 127;
        mMaxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return 8 + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override
    {
        return 8 + 8;
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset, uint16_t repOffset) const override
    {
        return 8 + 8;
    }
};

class FormatZX2: public Format
{
public:

    FormatZX2() = delete;

    FormatZX2(FormatOptions options): Format{options}
    {
        mMaxLiteralLength = 0xFFFF;
        mMinMatchLength = 2;
        mMaxMatchLength = 0xFFFF;

        if (mAddEndMarker)
        {
            mExtendOffset = true;
            mMaxMatchOffset = 255;
        }
        else
        {
            mMaxMatchOffset = mExtendOffset ? 256 : 255;
        }
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return 1 + GetElias1Cost(length) + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override
    {
        return 1 + 8 + GetElias1Cost(length - 1);
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset, uint16_t repOffset) const override
    {
        if (offset == repOffset)
        {
            return 1 + GetElias1Cost(length);
        }
        else
        {
            return 1 + 8 + GetElias1Cost(length - 1);
        }
    }
};

class FormatE1: public Format
{
public:

    FormatE1() = delete;

    FormatE1(FormatOptions options): Format{options}
    {
        mMaxLiteralLength = 255;
        mMinMatchLength = 2;
        mMaxMatchLength = 256;
        mMaxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return GetElias1Cost(length) + 1 + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override
    {
        return GetElias1Cost(length - 1) + 1 + 8;
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset, uint16_t repOffset) const override
    {
        return GetMatchCost(length, offset);
    }
};

class FormatE1ZX: public Format
{
public:

    FormatE1ZX() = delete;

    FormatE1ZX(FormatOptions options): Format{options}
    {
        mMaxLiteralLength = 255;
        mMinMatchLength = 2;
        mMaxMatchLength = 256;
        mMaxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return GetElias1Cost(length) + 1 + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override
    {
        return GetElias1Cost(length - 1) + 1 + 8;
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset, uint16_t repOffset) const override
    {
        return GetMatchCost(length, offset);
    }
};

class FormatUE2: public Format
{
public:

    FormatUE2() = delete;

    FormatUE2(FormatOptions options): Format{options}
    {
        mMaxLiteralLength = 0xFFFF;
        mMinMatchLength = 2;
        mMaxMatchLength = 255;
        mMaxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return (1 + 8) * length;
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override
    {
        return 1 + GetElias2Cost(length) + 8;
    }

    uint32_t GetMatchCost(uint16_t length, uint16_t offset, uint16_t repOffset) const override
    {
        return GetMatchCost(length, offset);
    }
};

#endif // FORMATS_H
