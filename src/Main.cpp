// Copyright (c) 2021, Milos "baze" Bazelides
// This code is licensed under the BSD 2-Clause License.

//#define VERIFY

#include <algorithm>
#include <cstdio>
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
    ExtendOffset,
    ExtendLength,
    NoSizeGain
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
        case WarningId::ExtendOffset:
            printf("Option -o is not supported by this format and will be ignored.\n");
            break;

        case WarningId::ExtendLength:
            printf("Option -l is not supported by this format and will be ignored.\n");
            break;

        case WarningId::NoSizeGain:
            printf("No size gain after compression.\n");
            break;
    }
}

void ValidateOptions(FormatOptions options, const Format& format)
{
    if (options.extendOffset && !format.SupportsExtendOffset())
    {
        PrintWarning(WarningId::ExtendOffset);
    }

    if (options.extendLength && !format.SupportsExtendLength())
    {
        PrintWarning(WarningId::ExtendLength);
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
    if (argCount < 2)
    {
        printf("\nUsage: bzpack.exe [-lzm|-ef8|-bx0|-bx2] [-r] [-e] [-o] [-l] [-n] <inputFile> [outputFile]\n");
        printf("\nOptions:\n\n");
        printf("-lzm: Byte-aligned LZSS. Raw 7-bit length, raw 8-bit offset (default).\n");
        printf("-ef8: Elias length, raw 8-bit offset.\n");
        printf("-bx0: Elias length, combined raw/Elias offset or repeat offset.\n");
        printf("-bx2: Elias length, raw 8-bit offset or repeat offset.\n");
        printf("-r: Compress in reverse order.\n");
        printf("-e: Add end-of-stream marker.\n");
        printf("-o: Extend the offset range by 1.\n");
        printf("-l: Extend the block length by 1.\n");
        printf("-n: Produce natural stream without stream-level optimizations.\n");
        return 0;
    }

    static std::string suffix = ".lzm";
    static FormatOptions options = {0};

    static const std::unordered_map<std::string, std::function<void()>> actions =
    {
        {"-lzm", [&]() { options.id = FormatId::LZM; suffix = ".lzm"; }},
        {"-ef8", [&]() { options.id = FormatId::EF8; suffix = ".ef8"; }},
        {"-bx0", [&]() { options.id = FormatId::BX0; suffix = ".bx0"; }},
        {"-bx2", [&]() { options.id = FormatId::BX2; suffix = ".bx2"; }},
        {"-r",   [&]() { options.reverse = 1; }},
        {"-e",   [&]() { options.endMarker = 1; }},
        {"-o",   [&]() { options.extendOffset = 1; }},
        {"-l",   [&]() { options.extendLength = 1; }},
        {"-n",   [&]() { options.naturalStream = 1; }}
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

    std::unique_ptr<Format> spFormat = Format::Create(options);
    if (spFormat == nullptr)
    {
        PrintError(ErrorId::OutOfMemory);
        return 1;
    }

    ValidateOptions(options, *spFormat);

    // Read input file.

    std::vector<uint8_t> inputData = ReadFile(inputName.c_str());
    if (inputData.empty())
    {
        return 1;
    }

    if (spFormat->SupportsRepOffset() && inputData.size() >= 0xFFFF)
    {
        PrintError(ErrorId::FileTooBig);
        return 1;
    }

    if (spFormat->Reverse())
    {
        std::reverse(inputData.begin(), inputData.end());
    }

    // Compress the input stream.

    BitStream packedStream = Compress(inputData.data(), static_cast<uint32_t>(inputData.size()), *spFormat);
    if (packedStream.Size() == 0)
    {
        PrintError(ErrorId::CompressionFailed);
        return 1;
    }

    if (packedStream.Size() >= inputData.size())
    {
        PrintWarning(WarningId::NoSizeGain);
    }

#ifdef VERIFY

    if (spFormat->Reverse())
    {
        std::reverse(inputData.begin(), inputData.end());
    }

    std::vector<uint8_t> unpackedData = Decompress(packedStream, *spFormat, static_cast<uint32_t>(inputData.size()));

    if (unpackedData.size() != inputData.size() || !std::equal(inputData.begin(), inputData.end(), unpackedData.data()))
    {
        printf("Stream verification failed.\n");
    }

#endif // VERIFY

    // Write output file.

    if (spFormat->Reverse())
    {
        packedStream.Reverse();
    }

    if (!WriteFile(outputName.c_str(), packedStream.Data(), packedStream.Size()))
    {
        return 1;    
    }

    return 0;
}
