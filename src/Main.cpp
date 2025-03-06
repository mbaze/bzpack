// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <memory>
#include <cstdio>
#include <fstream>
#include <functional>
#include <unordered_map>
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

std::vector<uint8_t> ReadFile(const char* pFileName)
{
    std::basic_ifstream<uint8_t> file(pFileName, std::ios::binary | std::ios::ate);

    if (!file)
    {
        PrintError(ErrorId::InputFile);
        return {};
    }

    std::streampos fileSize = file.tellg();

    if (fileSize < 0)
    {
        PrintError(ErrorId::InputFile);
        return {};
    }

    if (fileSize == 0)
    {
        PrintError(ErrorId::FileEmpty);
        return {};
    }

    if (fileSize > 0xFFFF)
    {
        PrintError(ErrorId::FileTooBig);
        return {};
    }

    if (!file.seekg(0, std::ios_base::beg))
    {
        PrintError(ErrorId::InputFile);
        return {};
    }

    std::vector<uint8_t> bytes(static_cast<size_t>(fileSize));

    if (!file.read(bytes.data(), fileSize))
    {
        PrintError(ErrorId::InputFile);
        return {};
    }

    if (file.gcount() != fileSize)
    {
        PrintError(ErrorId::InputFile);
        return {};
    }

    return bytes;
}

bool WriteFile(const char* pFileName, const uint8_t* pData, size_t size)
{
    std::basic_ofstream<uint8_t> outputFile(pFileName, std::ios::binary);

    if (!outputFile)
    {
        PrintError(ErrorId::OutputFile);
        return false;
    }

    if (!outputFile.write(pData, size))
    {
        PrintError(ErrorId::OutputFile);
        return false;
    }

    return true;
}

int main(int argCount, char** args)
{
    if (argCount < 2)
    {
        printf("\nUsage: bzpack.exe [-lz|-e1|-e1zx|-bx2|-ue2] [-r] [-e] [-o] [-l] <inputFile> [outputFile]\n");
        printf("\nOptions:\n\n");
        printf("-lz: Byte-aligned LZSS. Raw 7-bit length, raw 8-bit offset (default).\n");
        printf("-e1: Elias 1..N length, raw 8-bit offset.\n");
        printf("-e1zx: A version of -e1 optimized for Sinclair ZX Spectrum.\n");
        printf("-bx2: Elias 1..N length, raw 8-bit offset or repeat offset.\n");
        printf("-ue2: Unary literal length, Elias 2..N match length, raw 8-bit offset.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Add end-of-stream marker.\n");
        printf("-o: Extend the offset range by 1.\n");
        printf("-l: Extend the block length by 1.\n");
        return 0;
    }

    std::string inputName;
    std::string outputName = ".lz";
    FormatOptions options = {FormatId::LZ, 0, 0, 0, 0};
    std::unique_ptr<Format> spFormat;

    static const std::unordered_map<std::string, std::function<void()>> actions =
    {
        {"-lz",   [&]() { options.id = FormatId::LZ; outputName = ".lz"; }},
        {"-e1",   [&]() { options.id = FormatId::E1; outputName = ".e1"; }},
        {"-e1zx", [&]() { options.id = FormatId::E1ZX; outputName = ".e1zx"; }},
        {"-bx2",  [&]() { options.id = FormatId::BX2; outputName = ".bx2"; }},
        {"-ue2",  [&]() { options.id = FormatId::UE2; outputName = ".ue2"; }},
        {"-r",    [&]() { options.reverse = 1; }},
        {"-e",    [&]() { options.addEndMarker = 1; }},
        {"-o",    [&]() { options.extendOffset = 1; }},
        {"-l",    [&]() { options.extendLength = 1; }}
    };

    // Process command line arguments.

    for (int i = 1; i < argCount; i++)
    {
        auto iAction = actions.find(args[i]);
        if (iAction != actions.end())
        {
            iAction->second();
        }
        else
        {
            if (args[i][0] == '-')
            {
                PrintError(ErrorId::InvalidParam, args[i]);
                return 1;
            }
            else if (i == argCount - 1)
            {
                inputName = args[i];
                outputName = inputName + outputName;
                break;
            }
            else if (i == argCount - 2)
            {
                inputName = args[i];
                outputName = args[i + 1];
                break;
            }
            else
            {
                PrintError(ErrorId::InvalidParam, args[i + 2]);
                return 1;
            }
        }
    }

    CheckOptions(options);

    switch (options.id)
    {
    case FormatId::LZ:
        spFormat = std::make_unique<FormatLZ>(options);
        break;

    case FormatId::E1:
        spFormat = std::make_unique<FormatE1>(options);
        break;

    case FormatId::E1ZX:
        spFormat = std::make_unique<FormatE1ZX>(options);
        break;

    case FormatId::BX2:
        spFormat = std::make_unique<FormatBX2>(options);
        break;

    case FormatId::UE2:
        spFormat = std::make_unique<FormatUE2>(options);
        break;
    }

    // Read input file.

    std::vector<uint8_t> inputStream = ReadFile(inputName.c_str());
    if (inputStream.size() == 0)
    {
        return 1;
    }

    // Compress the input stream.

    BitStream packedStream = Compress(inputStream.data(), static_cast<uint16_t>(inputStream.size()), *spFormat.get());
    if (packedStream.Size() == 0)
    {
        PrintError(ErrorId::CompressionFailed);
        return 1;
    }

    if (packedStream.Size() >= inputStream.size())
    {
        PrintWarning(WarningId::NoSizeGain);
    }

    if (options.id == FormatId::E1ZX && packedStream.IssueCarryWarning())
    {
        PrintWarning(WarningId::ZxExplicitCarry);
    }

    // Write output file.

    if (!WriteFile(outputName.c_str(), packedStream.Data(), packedStream.Size()))
    {
        return 1;    
    }

    return 0;
}
