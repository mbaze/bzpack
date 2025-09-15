// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef UNIVERSAL_CODES_H
#define UNIVERSAL_CODES_H

#include "BitStream.h"

uint32_t GetEliasCost(uint32_t value);
void EncodeElias(BitStream& stream, uint32_t value);
uint32_t DecodeElias(BitStream& stream, uint32_t value = 1);

uint32_t GetUnaryCost(uint32_t value);
void EncodeUnary(BitStream& stream, uint32_t value);
uint32_t DecodeUnary(BitStream& stream);

uint32_t GetRiceCost(uint32_t value);
void EncodeRice(BitStream& stream, uint32_t value);
uint32_t DecodeRice(BitStream& stream);

uint32_t GetVbinCost(uint32_t value);
void EncodeVbin(BitStream& stream, uint32_t value);
uint32_t DecodeVbin(BitStream& stream);

void EncodeRaw(BitStream& stream, uint32_t value, uint32_t bitCount);
uint32_t DecodeRaw(BitStream& stream, uint32_t bitCount);

// Only used by the E1ZX format.

void EncodeEliasNeg(BitStream& stream, uint32_t value);
uint32_t DecodeEliasNeg(BitStream& stream);

// Only used by the BX0 format.

bool EncodeEliasWithoutFlag(BitStream& stream, uint32_t value);
uint32_t DecodeEliasWithFlag(BitStream& stream, bool flag);

#endif // UNIVERSAL_CODES_H
