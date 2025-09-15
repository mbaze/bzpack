// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef OPTIMAL_PARSER_H
#define OPTIMAL_PARSER_H

#include <vector>
#include "CommonTypes.h"
#include "Formats.h"

std::vector<ParseStep> Parse(const uint8_t* pInput, uint16_t inputSize, const Format& format);

#endif // OPTIMAL_PARSER_H
