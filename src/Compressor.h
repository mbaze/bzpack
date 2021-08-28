// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "BitStream.h"

// "Block" methods encode literal sequences as "block length + flag + bytes".
// "Unary" methods encode every literal as "flag + byte".

enum Format: uint32_t
{
    AlignedLZSS,
    BlockElias1,
    BlockElias2,
    UnaryElias1,
    UnaryElias2,

    // Some formats support tweaks, e.g. shorter decoder vs. bigger offset.

    FlagReverse = 0x80000000,
    FlagAddEndMarker = 0x40000000,
    FlagExtendOffset = 0x20000000,
    FlagExtendLength = 0x10000000,
    Mask = 0x0FFFFFFF
};

bool Compress(uint8_t* pInputStream, size_t inputSize, uint32_t format, BitStream& packedStream);

// Zero inputSize tells the decompressor to expect end of stream marker.
bool Decompress(BitStream& packedStream, uint32_t format, size_t inputSize, std::vector<uint8_t>& outputStream);

#endif // COMPRESSOR_H
