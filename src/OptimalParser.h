// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#pragma once

#include "Compressor.h"

struct StreamRef
{
    uint32_t offset;
    uint32_t length;
};

struct Node
{
    uint32_t offset = 0;
    uint32_t length = 0;
    uint32_t cost = UINT32_MAX;
    uint32_t lastOffset = 0;
};

struct FormatLimits
{
    uint32_t minMatchLength = 2;
    uint32_t maxMatchOffset = UINT32_MAX;
    uint32_t maxMatchLength = UINT32_MAX;
    uint32_t maxLiteralLength = UINT32_MAX;
};

bool Parse(const uint8_t* pInputStream, size_t inputSize, FormatOptions format, std::vector<StreamRef>& refs);
