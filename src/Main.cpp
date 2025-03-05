// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <unordered_map>
#include <memory>
#include <cstdio>
#include <fstream>
#include <functional>
#include "Compressor.h"

enum ErrorId
{
    InvalidParam,
    InputFile,
    OutputFile,
    FileEmpty,
    FileTooBig,
    CompressionFailed
};

enum WarningId
{
    ExtendLength,
    AddEndMarker,
    NoSizeGain,
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

        case ErrorId::FileEmpty:
            printf("Input file empty.\n");
            break;

        case ErrorId::FileTooBig:
            printf("Input file too big.\n");
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

        case WarningId::NoSizeGain:
            printf("No size gain after compression.\n");
            break;

        case WarningId::ZxExplicitCarry:
            printf("Make sure the decompressor sets Carry during fetch.\n");
            break;
    }
}

void CheckOptions(FormatOptions options)
{
    bool noExtLength = (options.id == FormatId::E1);
    noExtLength |= (options.id == FormatId::E1ZX);
    noExtLength |= (options.id == FormatId::UE2);

    if (options.extendLength && noExtLength)
    {
        PrintWarning(WarningId::ExtendLength);
    }

    if (options.id == FormatId::E1ZX && options.addEndMarker)
    {
        PrintWarning(WarningId::AddEndMarker);
    }
}

int main(int argCount, char** args)
{
    if (argCount < 3)
    {
        printf("\nUsage: bzpack.exe <input.raw> <output.bin> [-lz|-e1|-e1zx|-bx2|-ue2] [-r] [-e] [-o] [-l]\n");
        printf("\nOptions:\n\n");
        printf("-lz: Byte-aligned LZSS. Raw 7-bit length, raw 8-bit offset (default).\n");
        printf("-e1: Elias 1..N length, raw 8-bit offset.\n");
        printf("-e1zx: A version of -e1 optimized for Sinclair ZX Spectrum.\n");
        printf("-bx2: Elias 1..N length, raw 8-bit offset or repeat offset.\n");
        printf("-ue2: Unary literal length, Elias 2..N match length, raw 8-bit offset.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Add end-of-stream marker.\n");
        printf("-o: Extend maximum window offset by 1.\n");
        printf("-l: Extend maximum block length by 1.\n");
        return 0;
    }

    FormatOptions options = {FormatId::LZ, 0, 0, 0, 0};

    static const std::unordered_map<std::string, std::function<void()>> optionDispatch =
    {
        {"-lz",   [&]() { options.id = FormatId::LZ; }},
        {"-e1",   [&]() { options.id = FormatId::E1; }},
        {"-e1zx", [&]() { options.id = FormatId::E1ZX; }},
        {"-bx2",  [&]() { options.id = FormatId::BX2; }},
        {"-ue2",  [&]() { options.id = FormatId::UE2; }},
        {"-r",    [&]() { options.reverse = 1; }},
        {"-e",    [&]() { options.addEndMarker = 1; }},
        {"-o",    [&]() { options.extendOffset = 1; }},
        {"-l",    [&]() { options.extendLength = 1; }}
    };

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

    CheckOptions(options);

    std::unique_ptr<Format> pFormat;

    switch (options.id)
    {
    case FormatId::LZ:
        pFormat = std::make_unique<FormatLZ>(options);
        break;

    case FormatId::E1:
        pFormat = std::make_unique<FormatE1>(options);
        break;

    case FormatId::E1ZX:
        pFormat = std::make_unique<FormatE1ZX>(options);
        break;

    case FormatId::BX2:
        pFormat = std::make_unique<FormatBX2>(options);
        break;

    case FormatId::UE2:
        pFormat = std::make_unique<FormatUE2>(options);
        break;
    }

    // Read input file.

    std::basic_ifstream<uint8_t> inputFile(args[1], std::ios::binary | std::ios::ate);

    if (!inputFile)
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    size_t inputFileSize = inputFile.tellg();

    if (inputFileSize == 0)
    {
        PrintError(ErrorId::FileEmpty);
        return 0;
    }

    if (inputFileSize > 0xFFFF)
    {
        PrintError(ErrorId::FileTooBig);
        return 0;
    }

    if (!inputFile.seekg(0, std::ios_base::beg))
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    std::unique_ptr<uint8_t[]> spInputStream = std::make_unique<uint8_t[]>(inputFileSize);

    if (!inputFile.read(spInputStream.get(), inputFileSize))
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    // Compress data.

    BitStream packedStream = Compress(spInputStream.get(), static_cast<uint16_t>(inputFileSize), *pFormat.get());
    if (packedStream.Size() == 0)
    {
        PrintError(ErrorId::CompressionFailed);
        return 0;
    }

    if (packedStream.Size() >= inputFileSize)
    {
        PrintWarning(WarningId::NoSizeGain);
    }

    if (options.id == FormatId::E1ZX && packedStream.IssueCarryWarning())
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
