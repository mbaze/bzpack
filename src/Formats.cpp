// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "Formats.h"
#include "UniversalCodes.h"

std::unique_ptr<Format> Format::Create(const FormatOptions& options)
{
    switch (options.id)
    {
    case FormatId::LZM:
        return std::unique_ptr<Format>(new FormatLZM(options));
    case FormatId::EF8:
        return std::unique_ptr<Format>(new FormatEF8(options));
    case FormatId::BX0:
        return std::unique_ptr<Format>(new FormatBX0(options));
    case FormatId::BX2:
        return std::unique_ptr<Format>(new FormatBX2(options));
    }

    return nullptr;
}

// LZM format.

FormatLZM::FormatLZM(FormatOptions options): Format{options}
{
    mFormatId = FormatId::LZM;
    mSupportsExtendOffset = true;
    mSupportsExtendLength = true;
    mRequiresDijkstra = false;

    mMaxLiteralLength = 127 + options.extendLength;
    mMinMatchLength = 2;
    mMaxMatchLength = 127 + options.extendLength;
    mMaxMatchOffset = 255 + options.extendOffset;
}

uint32_t FormatLZM::GetLiteralCost(uint16_t length) const
{
    return 8 + (length << 3);
}

uint32_t FormatLZM::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return 8 + 8;
}

uint32_t FormatLZM::GetRepMatchCost(uint16_t length) const
{
    return 0xFFFFFFFF;
}

// EF8 format.

FormatEF8::FormatEF8(FormatOptions options): Format{options}
{
    mFormatId = FormatId::EF8;
    mSupportsExtendOffset = true;
    mSupportsExtendLength = false;
    mRequiresDijkstra = false;

    mMaxLiteralLength = 255;
    mMinMatchLength = 2;
    mMaxMatchLength = 256;
    mMaxMatchOffset = 255 + options.extendOffset;
}

uint32_t FormatEF8::GetLiteralCost(uint16_t length) const
{
    return GetEliasCost(length) + 1 + (length << 3);
}

uint32_t FormatEF8::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return GetEliasCost(length - 1) + 1 + 8;
}

uint32_t FormatEF8::GetRepMatchCost(uint16_t length) const
{
    return 0xFFFFFFFF;
}

// BX0 format.

FormatBX0::FormatBX0(FormatOptions options): Format{options}
{
    mFormatId = FormatId::BX0;
    mSupportsExtendOffset = true;
    mSupportsExtendLength = false;
    mRequiresDijkstra = true;

    mMaxLiteralLength = 0xFFFF;
    mMinMatchLength = 2;
    mMaxMatchLength = 0xFFFF;
    mMaxMatchOffset = 0x3FFF + options.extendOffset;
}

uint32_t FormatBX0::GetLiteralCost(uint16_t length) const
{
    return 1 + GetEliasCost(length) + (length << 3);
}

uint32_t FormatBX0::GetMatchCost(uint16_t length, uint16_t offset) const
{
    uint16_t eliasPart = GetEliasPart(offset);
    return 1 + GetEliasCost(eliasPart) + 7 + GetEliasCost(length - 1);
}

uint32_t FormatBX0::GetRepMatchCost(uint16_t length) const
{
    return 1 + GetEliasCost(length);
}

// BX2 format.

FormatBX2::FormatBX2(FormatOptions options): Format{options}
{
    mFormatId = FormatId::BX2;
    mSupportsExtendOffset = false;
    mSupportsExtendLength = false;
    mRequiresDijkstra = true;

    mMaxLiteralLength = 0xFFFF;
    mMinMatchLength = 2;
    mMaxMatchLength = 0xFFFF;
    mMaxMatchOffset = 255;
}

uint32_t FormatBX2::GetLiteralCost(uint16_t length) const
{
    return GetEliasCost(length) + 1 + (length << 3);
}

uint32_t FormatBX2::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return GetEliasCost(length - 1) + 1 + 8;
}

uint32_t FormatBX2::GetRepMatchCost(uint16_t length) const
{
    return GetEliasCost(length) + 1;
}
