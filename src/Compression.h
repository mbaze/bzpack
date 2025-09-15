// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "BitStream.h"
#include "Formats.h"

BitStream Compress(uint8_t* pInput, uint16_t inputSize, const Format& format);
std::vector<uint8_t> Decompress(BitStream& stream, const Format& format, uint16_t inputSize);

#endif // COMPRESSION_H
