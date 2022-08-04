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

uint32_t GetVbinCost(uint32_t value);
void EncodeVbin(BitStream& stream, uint32_t value);
uint32_t DecodeVbin(BitStream& stream);

void EncodeRaw(BitStream& stream, uint32_t value, uint32_t bits);
uint32_t DecodeRaw(BitStream& stream, uint32_t bits);

// These methods are only required by the E1ZX format.

void EncodeElias1Neg(BitStream& stream, uint32_t value);
uint32_t DecodeElias1Neg(BitStream& stream);

#endif // UNIVERSAL_CODES_H
