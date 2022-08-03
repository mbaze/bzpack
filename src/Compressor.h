// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "BitStream.h"

enum FormatId
{
    Aligned_LZSS,
    Elias1,
    Elias1_ZX,
    Elias1_Ext,
    Elias1_Rep,
    Unary_Elias2,
};

struct FormatOptions
{
    uint8_t Id: 4;
    uint8_t Reverse: 1;
    uint8_t AddEndMarker: 1;
    uint8_t ExtendOffset: 1;
    uint8_t ExtendLength: 1;
};

bool Compress(uint8_t* pInputStream, size_t inputSize, FormatOptions format, BitStream& packedStream);
bool Decompress(BitStream& packedStream, FormatOptions format, size_t inputSize, std::vector<uint8_t>& outputStream);

#endif // COMPRESSOR_H
