// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "BitStream.h"

// "Block" methods encode literals as "block length + flag + data".
// "Unary" methods encode every literal byte as "flag + data".

enum Format: uint32_t
{
    BlockElias1,
    BlockElias2,
    UnaryElias1,
    UnaryElias2,
    UnaryRice,
    AlignedLZSS,

    // Some formats support tweaks, e.g. shorter decoder vs. bigger offset.

    FlagReverse = 0x80000000,
    FlagEndMarker = 0x40000000,
    FlagExtendOffset = 0x20000000,
    FlagExtendLength = 0x10000000,
    Mask = 0x0FFFFFFF
};

bool Compress(uint8_t* pInputStream, size_t inputSize, uint32_t format, BitStream& packedStream);

#endif // COMPRESSOR_H
