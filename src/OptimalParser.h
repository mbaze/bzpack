// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#ifndef OPTIMAL_PARSER_H
#define OPTIMAL_PARSER_H

#include "Formats.h"
#include "MatchFinder.h"

struct PathNode
{
    uint16_t offset = 0;
    uint16_t length = 0;
    uint32_t cost = 0xFFFFFFFF;
};

bool Parse(std::vector<Match>& parse, const uint8_t* pInput, uint16_t inputSize, const Format& format);

#endif // OPTIMAL_PARSER_H
