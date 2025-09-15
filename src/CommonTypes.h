// Copyright (c) 2025, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <cstdint>

struct Match
{
    Match(uint16_t length, uint16_t offset):
        length{length}, offset{offset}
    {}

    uint16_t length;
    uint16_t offset;
};

struct ParseStep
{
    ParseStep(uint16_t length, uint16_t offset):
        length{length}, offset{offset}
    {}

    uint16_t length;
    uint16_t offset;
};

#endif // COMMON_TYPES_H
