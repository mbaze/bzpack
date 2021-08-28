// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#pragma once

#include "Compressor.h"

struct StreamRef
{
    uint32_t offset;
    uint32_t length;
};

struct FormatLimits
{
    uint32_t minMatchLength = 1;
    uint32_t maxMatchOffset = UINT32_MAX;
    uint32_t maxMatchLength = UINT32_MAX;
    uint32_t maxLiteralLength = UINT32_MAX;
};

// Zero offset indicates a literal.

struct Node
{
    uint32_t offset = 0;
    uint32_t length = 1;
    uint32_t cost = UINT32_MAX;
    uint32_t literalCount = 0;
};

bool Parse(const uint8_t* pInputStream, size_t inputSize, uint32_t format, std::vector<StreamRef>& refs);
