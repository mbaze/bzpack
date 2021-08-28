// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <cstdio>
#include <memory>
#include <string>
#include <algorithm>
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
        printf("Compression algorithm failed.");
        break;
    }
}

int main(int argCount, char** args)
{
    enum Args { lzs, be1, be2, ue1, ue2, r, e, o, l, Count };

    if (argCount < 3)
    {
        printf("\nUsage: bzpack.exe <input.bin> <output.bzp> [-lzs|-be1|-be2|-ue1|-ue2] [-e] [-o] [-l]\n");
        printf("\nOptions:\n");
        printf("-lzs: Byte-aligned LZSS with raw 8-bit length.\n");
        printf("-be1: Block literals with Elias-Gamma length 1..N (default).\n");
        printf("-be2: Block literals with Elias-Gamma length 2..N.\n");
        printf("-ue1: Unary literals with Elias-Gamma length 1..N.\n");
        printf("-ue2: Unary literals with Elias-Gamma length 2..N.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Append end of stream marker.\n");
        printf("-o: Extend maximum window offset by 1.\n");
        printf("-l: Extend block length by 1.\n");
        return 0;
    }

    uint32_t format = 0;

    if (argCount > 3)
    {
        std::vector<std::string> argNames(Args::Count);
        argNames[Args::lzs] = "-lzs";
        argNames[Args::be1] = "-be1";
        argNames[Args::be2] = "-be2";
        argNames[Args::ue1] = "-ue1";
        argNames[Args::ue2] = "-ue2";
        argNames[Args::r] = "-r";
        argNames[Args::e] = "-e";
        argNames[Args::o] = "-o";
        argNames[Args::l] = "-l";

        uint32_t argFlags[Args::Count];
        argFlags[Args::lzs] = Format::AlignedLZSS;
        argFlags[Args::be1] = Format::BlockElias1;
        argFlags[Args::be2] = Format::BlockElias2;
        argFlags[Args::ue1] = Format::UnaryElias1;
        argFlags[Args::ue2] = Format::UnaryElias2;
        argFlags[Args::r] = Format::FlagReverse;
        argFlags[Args::e] = Format::FlagAddEndMarker;
        argFlags[Args::o] = Format::FlagExtendOffset;
        argFlags[Args::l] = Format::FlagExtendLength;

        for (int i = 3; i < argCount; i++)
        {
            auto param = std::find(argNames.begin(), argNames.end(), args[i]);
            if (param != argNames.end())
            {
                format |= argFlags[param - argNames.begin()];
            }
        }
    }

    if ((format & Format::Mask) == Format::Default)
    {
        format |= Format::BlockElias1;
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
