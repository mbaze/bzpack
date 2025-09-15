// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include "Formats.h"
#include "UniversalCodes.h"

std::unique_ptr<Format> Format::Create(const FormatOptions& options)
{
    switch (options.id)
    {
    case FormatId::LZ:
        return std::unique_ptr<Format>(new FormatLZ(options));
    case FormatId::E1:
        return std::unique_ptr<Format>(new FormatE1(options));
    case FormatId::E1ZX:
        return std::unique_ptr<Format>(new FormatE1ZX(options));
    case FormatId::BX0:
        return std::unique_ptr<Format>(new FormatBX0(options));
    case FormatId::BX2:
        return std::unique_ptr<Format>(new FormatBX2(options));
    }

    return nullptr;
}

// LZ format.

FormatLZ::FormatLZ(FormatOptions options): Format{options}
{
    mMaxLiteralLength = options.extendLength ? 128 : 127;
    mMinMatchLength = 2;
    mMaxMatchLength = options.extendLength ? 128 : 127;
    mMaxMatchOffset = options.extendOffset ? 256 : 255;
}

uint32_t FormatLZ::GetLiteralCost(uint16_t length) const
{
    return 8 + (length << 3);
}

uint32_t FormatLZ::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return 8 + 8;
}

uint32_t FormatLZ::GetRepMatchCost(uint16_t length) const
{
    return 0xFFFFFFFF;
}

// E1 format.

FormatE1::FormatE1(FormatOptions options): Format{options}
{
    mMaxLiteralLength = 255;
    mMinMatchLength = 2;
    mMaxMatchLength = 256;
    mMaxMatchOffset = options.extendOffset ? 256 : 255;
}

uint32_t FormatE1::GetLiteralCost(uint16_t length) const
{
    return GetElias1Cost(length) + 1 + (length << 3);
}

uint32_t FormatE1::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return GetElias1Cost(length - 1) + 1 + 8;
}

uint32_t FormatE1::GetRepMatchCost(uint16_t length) const
{
    return 0xFFFFFFFF;
}

// E1ZX format.

FormatE1ZX::FormatE1ZX(FormatOptions options): Format{options}
{
    mMaxLiteralLength = 255;
    mMinMatchLength = 2;
    mMaxMatchLength = 256;
    mMaxMatchOffset = options.extendOffset ? 256 : 255;
}

uint32_t FormatE1ZX::GetLiteralCost(uint16_t length) const
{
    return GetElias1Cost(length) + 1 + (length << 3);
}

uint32_t FormatE1ZX::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return GetElias1Cost(length - 1) + 1 + 8;
}

uint32_t FormatE1ZX::GetRepMatchCost(uint16_t length) const
{
    return 0xFFFFFFFF;
}

// BX0 format.

FormatBX0::FormatBX0(FormatOptions options): Format{options}
{
    mMaxLiteralLength = 0xFFFF;
    mMinMatchLength = 2;
    mMaxMatchLength = 0xFFFF;
    mMaxMatchOffset = 0x3FFF;
}

uint32_t FormatBX0::GetLiteralCost(uint16_t length) const
{
    return 1 + GetElias1Cost(length) + (length << 3);
}

uint32_t FormatBX0::GetMatchCost(uint16_t length, uint16_t offset) const
{
    uint16_t eliasPart = GetEliasPart(offset);
    return 1 + GetElias1Cost(eliasPart) + 7 + GetElias1Cost(length - 1);
}

uint32_t FormatBX0::GetRepMatchCost(uint16_t length) const
{
    return 1 + GetElias1Cost(length);
}

// BX2 format.

FormatBX2::FormatBX2(FormatOptions options): Format{options}
{
    mMaxLiteralLength = 0xFFFF;
    mMinMatchLength = 2;
    mMaxMatchLength = 0xFFFF;
    mMaxMatchOffset = 255;
}

uint32_t FormatBX2::GetLiteralCost(uint16_t length) const
{
    return GetElias1Cost(length) + 1 + (length << 3);
}

uint32_t FormatBX2::GetMatchCost(uint16_t length, uint16_t offset) const
{
    return GetElias1Cost(length - 1) + 1 + 8;
}

uint32_t FormatBX2::GetRepMatchCost(uint16_t length) const
{
    return GetElias1Cost(length) + 1;
}
