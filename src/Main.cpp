// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <map>
#include <memory>
#include <string>
#include <cstdio>
#include "Compressor.h"

enum Error
{
    InputFile,
    OutputFile,
    Compression,
};

void PrintError(Error error)
{
    printf("Error: ");

    switch (error)
    {
        case Error::InputFile:
            printf("Cannot open input file.\n");
            break;

        case Error::OutputFile:
            printf("Cannot create output file.\n");
            break;

        case Error::Compression:
            printf("Compression failed.");
            break;
    }
}

int main(int argCount, char** args)
{
    static const std::map<std::string, uint32_t> options =
    {
        {"-lzs", Format::Aligned_LZSS},
        {"-e1e1", Format::Elias1_Elias1},
        {"-e1x1", Format::Elias1_Elias1_X},
        {"-e1r1", Format::Elias1_Elias1_R},
        {"-ue2", Format::Unary_Elias2},
        {"-r", Format::FlagReverse},
        {"-e", Format::FlagAddEndMarker},
        {"-o", Format::FlagExtendOffset},
        {"-l", Format::FlagExtendLength}
    };

    if (argCount < 3)
    {
        printf("\nUsage: bzpack.exe <input.bin> <output.bzp> [-lzs|-e1e1|-e1x1|-ue2] [-e] [-o] [-l]\n");
        printf("\nOptions:\n\n");
        printf("-lzs: Byte-aligned LZSS. 7-bit block length, 8-bit offset.\n");
        printf("-e1e1: Elias 1..N literal length, Elias 1..N phrase length, 8-bit offset (default).\n");
        printf("-e1x1: Elias 1..N literal length, Elias 1..N phrase length, 8-bit (or extended) offset.\n");
        printf("-e1r1: Elias 1..N literal length, Elias 1..N phrase length, 8-bit (or reused) offset.\n");
        printf("-ue2: Unary literal length, Elias 2..N phrase length, 8-bit offset.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Add end of stream marker.\n");
        printf("-o: Extend maximum window offset by 1.\n");
        printf("-l: Extend maximum block length by 1.\n");
        return 0;
    }

    uint32_t format = Format::Default;

    if (argCount > 3)
    {
        for (int i = 3; i < argCount; i++)
        {
            auto option = options.find(args[i]);
            if (option != options.end())
            {
                format |= option->second;
            }
        }
    }

    if ((format & Format::Mask) == Format::Default)
    {
        format |= Format::Elias1_Elias1;
    }

    FILE* pInputFile = fopen(args[1], "rb");
    if (pInputFile == NULL)
    {
        PrintError(Error::InputFile);
        return 0;
    }

    if (fseek(pInputFile, 0, SEEK_END))
    {
        fclose(pInputFile);
        PrintError(Error::InputFile);
        return 0;
    }

    size_t inputFileSize = ftell(pInputFile);
    rewind(pInputFile);

    std::unique_ptr<uint8_t[]> spInputStream = std::make_unique<uint8_t[]>(inputFileSize);
    size_t bytesRead = fread(spInputStream.get(), 1, inputFileSize, pInputFile);
    fclose(pInputFile);

    if (bytesRead != inputFileSize)
    {
        PrintError(Error::InputFile);
        return 0;
    }

    BitStream packedStream;
    if (Compress(spInputStream.get(), inputFileSize, format, packedStream) == false)
    {
        PrintError(Error::Compression);
    }

    FILE* pOutputFile = fopen(args[2], "wb");
    if (pOutputFile == NULL)
    {
        PrintError(Error::OutputFile);
        return 0;
    }

    size_t bytesWritten = fwrite(packedStream.Data(), 1, packedStream.Size(), pOutputFile);
    fclose(pOutputFile);

    if (bytesWritten != packedStream.Size())
    {
        PrintError(Error::OutputFile);
        return 0;
    }

    return 0;
}
