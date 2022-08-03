// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <map>
#include <memory>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <functional>
#include "Compressor.h"

enum ErrorId
{
    InvalidParam,
    InputFile,
    OutputFile,
    EmptyFile,
    CompressionFailed
};

enum WarningId
{
    ExtendLength,
    AddEndMarker,
    ZxExplicitCarry
};

void PrintError(ErrorId error, const char* pString = nullptr)
{
    printf("Error: ");

    switch (error)
    {
        case ErrorId::InvalidParam:
            printf("Invalid parameter %s.\n", pString);
            break;

        case ErrorId::InputFile:
            printf("Cannot open input file.\n");
            break;

        case ErrorId::OutputFile:
            printf("Cannot create output file.\n");
            break;

        case ErrorId::EmptyFile:
            printf("Nothing to compress.\n");
            break;

        case ErrorId::CompressionFailed:
            printf("Compression failed.\n");
            break;
    }
}

void PrintWarning(WarningId warning)
{
    printf("Warning: ");

    switch (warning)
    {
        case WarningId::ExtendLength:
            printf("Option -l is not supported by this format.\n");
            break;

        case WarningId::AddEndMarker:
            printf("Option -e is not supported by this format.\n");
            break;

        case WarningId::ZxExplicitCarry:
            printf("Make sure that decompressor sets Carry during fetch.\n");
            break;
    }
}

void CheckOptions(FormatOptions format)
{
    bool noExtLength = (format.Id == FormatId::Elias1);
    noExtLength |= (format.Id == FormatId::Elias1_ZX);
    noExtLength |= (format.Id == FormatId::Elias1_Ext);
    noExtLength |= (format.Id == FormatId::Elias1_Rep);
    noExtLength |= (format.Id == FormatId::Unary_Elias2);

    if (format.ExtendLength && noExtLength)
    {
        PrintWarning(WarningId::ExtendLength);
    }

    if (format.Id == FormatId::Elias1_ZX && format.AddEndMarker)
    {
        PrintWarning(WarningId::AddEndMarker);
    }
}

int main(int argCount, char** args)
{
    FormatOptions format = {0};
    format.Id = FormatId::Elias1;

    static const std::map<std::string, std::function<void()>> optionDispatch =
    {
        { "-lzs",  [&]() { format.Id = FormatId::Aligned_LZSS; } },
        { "-e1",   [&]() { format.Id = FormatId::Elias1; } },
        { "-e1zx", [&]() { format.Id = FormatId::Elias1_ZX; } },
        { "-e1x",  [&]() { format.Id = FormatId::Elias1_Ext; } },
        { "-e1r",  [&]() { format.Id = FormatId::Elias1_Rep; } },
        { "-ue2",  [&]() { format.Id = FormatId::Unary_Elias2; } },
        { "-r",    [&]() { format.Reverse = 1; } },
        { "-e",    [&]() { format.AddEndMarker = 1; } },
        { "-o",    [&]() { format.ExtendOffset = 1; } },
        { "-l",    [&]() { format.ExtendLength = 1; } }
    };

    if (argCount < 3)
    {
        printf("\nUsage: bzpack.exe <input.raw> <output.bin> [-lzs|-e1|-e1zx|-e1x|-e1r|-ue2] [-r] [-e] [-o] [-l]\n");
        printf("\nOptions:\n\n");
        printf("-lzs: Byte-aligned LZSS. Raw 7-bit block length, raw 8-bit offset.\n");
        printf("-e1: Elias 1..N phrase / literal length, raw 8-bit offset (default).\n");
        printf("-e1zx: A version of the former slightly optimized for Sinclair ZX Spectrum.\n");
        printf("-e1x: Elias 1..N phrase / literal length, raw 8-bit (or extended) offset.\n");
        printf("-e1r: Elias 1..N phrase / literal length, raw 8-bit (or reused) offset.\n");
        printf("-ue2: Unary literal length, Elias 2..N phrase length, raw 8-bit offset.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Add end-of-stream marker.\n");
        printf("-o: Extend maximum window offset by 1 byte.\n");
        printf("-l: Extend maximum block length by 1 byte.\n");
        return 0;
    }

    if (argCount > 3)
    {
        for (int i = 3; i < argCount; i++)
        {
            auto option = optionDispatch.find(args[i]);
            if (option != optionDispatch.end())
            {
                option->second();
            }
            else
            {
                PrintError(ErrorId::InvalidParam, args[i]);
                return 0;
            }
        }
    }

    CheckOptions(format);

    // Read input file.

    std::basic_ifstream<uint8_t> inputFile(args[1], std::ios::binary);
    if (!inputFile)
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    size_t inputFileSize = std::filesystem::file_size(args[1]);
    if (inputFileSize == 0)
    {
        PrintError(ErrorId::EmptyFile);
        return 0;
    }

    std::unique_ptr<uint8_t[]> spInputStream = std::make_unique<uint8_t[]>(inputFileSize);
    if (!inputFile.read(spInputStream.get(), inputFileSize))
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    // Compress data.

    BitStream packedStream;
    if (!Compress(spInputStream.get(), inputFileSize, format, packedStream))
    {
        PrintError(ErrorId::CompressionFailed);
        return 0;
    }

    if (format.Id == FormatId::Elias1_ZX && packedStream.IssueCarryWarning())
    {
        PrintWarning(WarningId::ZxExplicitCarry);
    }

    // Write output file.

    std::basic_ofstream<uint8_t> outputFile(args[2], std::ios::binary);
    if (!outputFile)
    {
        PrintError(ErrorId::OutputFile);
        return 0;
    }

    if (!outputFile.write(packedStream.Data(), packedStream.Size()))
    {
        PrintError(ErrorId::OutputFile);
        return 0;
    }

    return 0;
}
