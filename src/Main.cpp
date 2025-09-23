// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <unordered_map>
#include "Compression.h"

enum ErrorId
{
    InvalidParam,
    InputFileError,
    OutputFileError,
    FileEmpty,
    FileTooBig,
    CompressionFailed,
    OutOfMemory
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

        case ErrorId::InputFileError:
            printf("Unable to open the input file.\n");
            break;

        case ErrorId::OutputFileError:
            printf("Unable to create the output file.\n");
            break;

        case ErrorId::FileEmpty:
            printf("The input file is empty.\n");
            break;

        case ErrorId::FileTooBig:
            printf("The input file is too large.\n");
            break;

        case ErrorId::CompressionFailed:
            printf("Compression failed.\n");
            break;

        case ErrorId::OutOfMemory:
            printf("Out of memory.\n");
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
    noExtLength |= (options.id == FormatId::BX2);

    if (options.extendLength && noExtLength)
    {
        PrintWarning(WarningId::ExtendLength);
    }

    if (options.id == FormatId::E1ZX && options.endMarker)
    {
        PrintWarning(WarningId::AddEndMarker);
    }
}

std::vector<uint8_t> ReadFile(const char* pFileName)
{
    std::basic_ifstream<uint8_t> file(pFileName, std::ios::binary | std::ios::ate);

    if (!file)
    {
        PrintError(ErrorId::InputFileError);
        return {};
    }

    std::streampos fileSize = file.tellg();

    if (fileSize < 0)
    {
        PrintError(ErrorId::InputFileError);
        return {};
    }

    if (fileSize == 0)
    {
        PrintError(ErrorId::FileEmpty);
        return {};
    }

    if (fileSize > 0xFFFE)
    {
        PrintError(ErrorId::FileTooBig);
        return {};
    }

    if (!file.seekg(0, std::ios_base::beg))
    {
        PrintError(ErrorId::InputFileError);
        return {};
    }

    std::vector<uint8_t> bytes(static_cast<size_t>(fileSize));

    if (!file.read(bytes.data(), fileSize))
    {
        PrintError(ErrorId::InputFileError);
        return {};
    }

    if (file.gcount() != fileSize)
    {
        PrintError(ErrorId::InputFileError);
        return {};
    }

    return bytes;
}

bool WriteFile(const char* pFileName, const uint8_t* pData, size_t size)
{
    std::basic_ofstream<uint8_t> outputFile(pFileName, std::ios::binary);

    if (!outputFile)
    {
        PrintError(ErrorId::OutputFileError);
        return false;
    }

    if (!outputFile.write(pData, size))
    {
        PrintError(ErrorId::OutputFileError);
        return false;
    }

    return true;
}

int main(int argCount, char** args)
{
    std::set_new_handler(
        [] {
            PrintError(ErrorId::OutOfMemory);
            std::abort();
        }
    );

    if (argCount < 2)
    {
        printf("\nUsage: bzpack.exe [-lzm|-e1|-e1zx|-bx0|-bx2] [-r] [-e] [-o] [-l] <inputFile> [outputFile]\n");
        printf("\nOptions:\n\n");
        printf("-lzm: Byte-aligned LZSS. Raw 7-bit length, raw 8-bit offset (default).\n");
        printf("-e1: Elias length, raw 8-bit offset.\n");
        printf("-e1zx: A version of -e1 optimized for the Sinclair ZX Spectrum.\n");
        printf("-bx0: Elias length, combined raw/Elias offset or repeat offset.\n");
        printf("-bx2: Elias length, raw 8-bit offset or repeat offset.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Add end-of-stream marker.\n");
        printf("-o: Extend the offset range by 1.\n");
        printf("-l: Extend the block length by 1.\n");
        return 0;
    }

    static std::string suffix = ".lzm";
    static FormatOptions options{FormatId::LZM, 0, 0, 0, 0};

    static const std::unordered_map<std::string, std::function<void()>> actions =
    {
        {"-lzm",  [&]() { options.id = FormatId::LZM; suffix = ".lzm"; }},
        {"-e1",   [&]() { options.id = FormatId::E1; suffix = ".e1"; }},
        {"-e1zx", [&]() { options.id = FormatId::E1ZX; suffix = ".e1zx"; }},
        {"-bx0",  [&]() { options.id = FormatId::BX0; suffix = ".bx0"; }},
        {"-bx2",  [&]() { options.id = FormatId::BX2; suffix = ".bx2"; }},
        {"-r",    [&]() { options.reverse = 1; }},
        {"-e",    [&]() { options.endMarker = 1; }},
        {"-o",    [&]() { options.extendOffset = 1; }},
        {"-l",    [&]() { options.extendLength = 1; }}
    };

    // Process command line arguments.

    std::string inputName, outputName;

    for (int i = 1; i < argCount; i++)
    {
        if (inputName.empty())
        {
            if (args[i][0] == '-')
            {
                auto iAction = actions.find(args[i]);
                if (iAction != actions.end())
                {
                    iAction->second();
                }
                else
                {
                    PrintError(ErrorId::InvalidParam, args[i]);
                    return 1;
                }
            }
            else
            {
                inputName = args[i];
            }
        }
        else if (outputName.empty())
        {
            outputName = args[i];
        }
        else
        {
            PrintError(ErrorId::InvalidParam, args[i]);
            return 1;
        }
    }

    if (inputName.empty())
    {
        PrintError(ErrorId::InputFileError);
        return 1;
    }

    if (outputName.empty())
    {
        outputName = inputName + suffix;
    }

    CheckOptions(options);

    std::unique_ptr<Format> spFormat = Format::Create(options);
    if (spFormat == nullptr)
    {
        PrintError(ErrorId::CompressionFailed);
        return 1;
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
