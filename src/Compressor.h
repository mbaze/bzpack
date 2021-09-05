// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "BitStream.h"

enum Format: uint32_t
{
    Default,
    Aligned_LZSS,
    Elias1_Elias1,
    Elias1_Elias1_X,
    Elias1_Elias1_R,
    Unary_Elias2,

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
