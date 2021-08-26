// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef UNIVERSAL_CODES_H
#define UNIVERSAL_CODES_H

#include "BitStream.h"

uint32_t GetElias1Cost(uint32_t value);
void EncodeElias1(BitStream& stream, uint32_t value);
uint32_t DecodeElias1(BitStream& stream);

uint32_t GetElias2Cost(uint32_t value);
void EncodeElias2(BitStream& stream, uint32_t value);
uint32_t DecodeElias2(BitStream& stream);

uint32_t GetUnaryCost(uint32_t value);
void EncodeUnary(BitStream& stream, uint32_t value);
uint32_t DecodeUnary(BitStream& stream);

uint32_t GetRiceCost(uint32_t value);
void EncodeRice(BitStream& stream, uint32_t value);
uint32_t DecodeRice(BitStream& stream);

#endif // UNIVERSAL_CODES_H
