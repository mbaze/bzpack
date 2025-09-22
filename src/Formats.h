// Copyright (c) 2024, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef FORMATS_H
#define FORMATS_H

#include <memory>

enum FormatId
{
    LZ,
    E1,
    E1ZX,
    BX0,
    BX2
};

struct FormatOptions
{
    uint8_t id: 4;
    uint8_t reverse: 1;
    uint8_t endMarker: 1;
    uint8_t extendOffset: 1;
    uint8_t extendLength: 1;
};

class Format
{
public:

    Format() = delete;
    virtual ~Format() = default;

    static std::unique_ptr<Format> Create(const FormatOptions& options);

    virtual uint32_t GetLiteralCost(uint16_t length) const = 0;
    virtual uint32_t GetMatchCost(uint16_t length, uint16_t offset) const = 0;
    virtual uint32_t GetRepMatchCost(uint16_t length) const = 0;

    FormatId Id() const { return mFormatId; }

    uint16_t MaxLiteralLength() const { return mMaxLiteralLength; }
    uint16_t MinMatchLength() const { return mMinMatchLength; }
    uint16_t MaxMatchLength() const { return mMaxMatchLength; }
    uint16_t MaxMatchOffset() const { return mMaxMatchOffset; }

    bool Reverse() const { return mReverse; }
    bool EndMarker() const { return mEndMarker; }
    bool ExtendOffset() const { return mExtendOffset; }
    bool ExtendLength() const { return mExtendLength; }

protected:

    Format(FormatOptions options):
        mFormatId(static_cast<FormatId>(options.id)),
        mReverse(options.reverse),
        mEndMarker(options.endMarker),
        mExtendOffset(options.extendOffset),
        mExtendLength(options.extendLength)
    {}

    FormatId mFormatId;

    // Format limits.

    uint16_t mMaxLiteralLength;
    uint16_t mMinMatchLength;
    uint16_t mMaxMatchLength;
    uint16_t mMaxMatchOffset;

    // Encoding options.

    bool mReverse;
    bool mEndMarker;
    bool mExtendOffset;
    bool mExtendLength;
};

class FormatLZ: public Format
{
    friend class Format;

    FormatLZ() = delete;
    FormatLZ(FormatOptions options);

    uint32_t GetLiteralCost(uint16_t length) const override;
    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override;
    uint32_t GetRepMatchCost(uint16_t length) const override;
};

class FormatE1: public Format
{
    friend class Format;

    FormatE1() = delete;
    FormatE1(FormatOptions options);

    uint32_t GetLiteralCost(uint16_t length) const override;
    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override;
    uint32_t GetRepMatchCost(uint16_t length) const override;
};

class FormatE1ZX: public Format
{
    friend class Format;

    FormatE1ZX() = delete;
    FormatE1ZX(FormatOptions options);

    uint32_t GetLiteralCost(uint16_t length) const override;
    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override;
    uint32_t GetRepMatchCost(uint16_t length) const override;
};

class FormatBX0: public Format
{
    friend class Format;

    FormatBX0() = delete;
    FormatBX0(FormatOptions options);

    uint32_t GetLiteralCost(uint16_t length) const override;
    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override;
    uint32_t GetRepMatchCost(uint16_t length) const override;

public:

    static uint8_t GetRawPart(uint16_t offset) { return offset & 127; }
    static uint16_t GetEliasPart(uint16_t offset) { return (offset >> 7) + 1; }
};

class FormatBX2: public Format
{
    friend class Format;

    FormatBX2() = delete;
    FormatBX2(FormatOptions options);

    uint32_t GetLiteralCost(uint16_t length) const override;
    uint32_t GetMatchCost(uint16_t length, uint16_t offset) const override;
    uint32_t GetRepMatchCost(uint16_t length) const override;
};

#endif // FORMATS_H
