// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "BitStream.h"
#include "Formats.h"

enum FormatId
{
    LZ,
    E1,
    E1ZX,
    UE2,
};

bool Compress(uint8_t* pInput, uint16_t inputSize, const Format& format, BitStream& packedStream);
bool Decompress(BitStream& packedStream, FormatOptions options, uint16_t inputSize, std::vector<uint8_t>& output);

#endif // COMPRESSOR_H
