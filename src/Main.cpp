// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <map>
#include <memory>
#include <string>
#include <functional>
#include <cstdio>
#include "Compressor.h"

enum ErrorId
{
    InvalidOption,
    InputFile,
    OutputFile,
    Compression,
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
        case ErrorId::InvalidOption:
            printf("Invalid option %s.\n", pString);
            break;

        case ErrorId::InputFile:
            printf("Cannot open input file.\n");
            break;

        case ErrorId::OutputFile:
            printf("Cannot create output file.\n");
            break;

        case ErrorId::Compression:
            printf("Compression failed.");
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
    bool noExtendedLength = (format.Id == FormatId::Elias1);
    noExtendedLength |= (format.Id == FormatId::Elias1_ZX);
    noExtendedLength |= (format.Id == FormatId::Elias1_Ext);
    noExtendedLength |= (format.Id == FormatId::Elias1_Rep);
    noExtendedLength |= (format.Id == FormatId::Unary_Elias2);

    if (format.ExtendLength && noExtendedLength)
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
        printf("-e: Add end of stream marker.\n");
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
                PrintError(ErrorId::InvalidOption, args[i]);
                return 0;
            }
        }
    }

    CheckOptions(format);

    FILE* pInputFile = fopen(args[1], "rb");
    if (pInputFile == NULL)
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    if (fseek(pInputFile, 0, SEEK_END))
    {
        fclose(pInputFile);
        PrintError(ErrorId::InputFile);
        return 0;
    }

    size_t inputFileSize = ftell(pInputFile);
    rewind(pInputFile);

    std::unique_ptr<uint8_t[]> spInputStream = std::make_unique<uint8_t[]>(inputFileSize);
    size_t bytesRead = fread(spInputStream.get(), 1, inputFileSize, pInputFile);
    fclose(pInputFile);

    if (bytesRead != inputFileSize)
    {
        PrintError(ErrorId::InputFile);
        return 0;
    }

    BitStream packedStream;
    if (Compress(spInputStream.get(), inputFileSize, format, packedStream) == false)
    {
        PrintError(ErrorId::Compression);
    }

    if (packedStream.IssueCarryWarning())
    {
        PrintWarning(WarningId::ZxExplicitCarry);
    }

    FILE* pOutputFile = fopen(args[2], "wb");
    if (pOutputFile == NULL)
    {
        PrintError(ErrorId::OutputFile);
        return 0;
    }

    size_t bytesWritten = fwrite(packedStream.Data(), 1, packedStream.Size(), pOutputFile);
    fclose(pOutputFile);

    if (bytesWritten != packedStream.Size())
    {
        PrintError(ErrorId::OutputFile);
        return 0;
    }

    return 0;
}
