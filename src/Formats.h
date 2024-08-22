// Copyright (c) 2024, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef FORMATS_H
#define FORMATS_H

#include "UniversalCodes.h"

struct FormatOptions
{
    uint8_t id: 4;
    uint8_t reverse: 1;
    uint8_t addEndMarker: 1;
    uint8_t extendOffset: 1;
    uint8_t extendLength: 1;
};

struct FormatLimits
{
    uint16_t maxLiteralLength = 0xFFFF;
    uint16_t minMatchLength = 2;
    uint16_t maxMatchLength = 0xFFFF;
    uint16_t maxMatchOffset = 0xFFFF;
};

class Format
{
public:

    Format() = delete;
    Format(FormatOptions options): mOptions(options) {}

    FormatOptions GetOptions() const { return mOptions; }
    FormatLimits GetLimits() const { return mLimits; }
    virtual uint32_t GetLiteralCost(uint16_t length) const = 0;
    virtual uint32_t GetMatchCost(uint16_t offset, uint16_t length) const = 0;

protected:

    FormatOptions mOptions;
    FormatLimits mLimits;
};

class FormatLZ: public Format
{
public:

    FormatLZ(FormatOptions options): Format(options)
    {
        mLimits.maxLiteralLength = options.extendLength ? 128 : 127;
        mLimits.minMatchLength = 2;
        mLimits.maxMatchLength = options.extendLength ? 128 : 127;
        mLimits.maxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return 8 + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t offset, uint16_t length) const override
    {
        return 8 + 8;
    }
};

class FormatE1: public Format
{
public:

    FormatE1(FormatOptions options): Format(options)
    {
        mLimits.maxLiteralLength = 255;
        mLimits.minMatchLength = 2;
        mLimits.maxMatchLength = 256;
        mLimits.maxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return GetElias1Cost(length) + 1 + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t offset, uint16_t length) const override
    {
        return GetElias1Cost(length - 1) + 1 + 8;
    }
};

class FormatE1ZX: public Format
{
public:

    FormatE1ZX(FormatOptions options): Format(options)
    {
        mLimits.maxLiteralLength = 255;
        mLimits.minMatchLength = 2;
        mLimits.maxMatchLength = 256;
        mLimits.maxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return GetElias1Cost(length) + 1 + (length << 3);
    }

    uint32_t GetMatchCost(uint16_t offset, uint16_t length) const override
    {
        return GetElias1Cost(length - 1) + 1 + 8;
    }
};

class FormatUE2: public Format
{
public:

    FormatUE2(FormatOptions options): Format(options)
    {
        mLimits.maxLiteralLength = 0xFFFF;
        mLimits.minMatchLength = 2;
        mLimits.maxMatchLength = 255;
        mLimits.maxMatchOffset = options.extendOffset ? 256 : 255;
    }

    uint32_t GetLiteralCost(uint16_t length) const override
    {
        return (1 + 8) * length;
    }

    uint32_t GetMatchCost(uint16_t offset, uint16_t length) const override
    {
        return 1 + GetElias2Cost(length) + 8;
    }
};

#endif // FORMATS_H
