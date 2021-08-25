// Copyright (c) 2021, Milos "baze" Bazelides
// This code is released under the terms of the BSD 2-Clause License.

#include <memory>
#include <string>
#include <stdio.h>
#include "Compressor.h"

const char* pInputFileError = "Error: Cannot open input file.\n";
const char* pOutputFileError = "Error: Cannot open output file.\n";
const char* pPackError = "Error: Compression failed.\n";

enum Args { be1, be2, ue1, ue2, ur1, lzs, r, e, o, l, Count };

int main(int argCount, char** args)
{
	if (argCount < 3)
	{
		printf("\nUsage: bzpack.exe <input.bin> <output.bzp> [-be1|-be2|-ue1|-ue2|-ur1] [-e] [-o] [-l]\n");
		printf("\nOptions:\n");
		printf("-be1: Block literals with Elias-Gamma length 1..N (default).\n");
		printf("-be2: Block literals with Elias-Gamma length 2..N.\n");
		printf("-ue1: Unary literals with Elias-Gamma length 1..N.\n");
		printf("-ue2: Unary literals with Elias-Gamma length 2..N.\n");
		printf("-ur1: Unary literals with Rice (K = 1) length 1..N.\n");
		printf("-lzs: Byte-aligned LZSS with raw 8-bit length.\n");
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
		argNames[Args::be1] = "-be1";
		argNames[Args::be2] = "-be2";
		argNames[Args::ue1] = "-ue1";
		argNames[Args::ue2] = "-ue2";
		argNames[Args::ur1] = "-ur1";
		argNames[Args::lzs] = "-lzs";
		argNames[Args::r] = "-r";
		argNames[Args::e] = "-e";
		argNames[Args::o] = "-o";
		argNames[Args::l] = "-l";

		uint32_t argFlags[Args::Count];
		argFlags[Args::be1] = Format::BlockElias1;
		argFlags[Args::be2] = Format::BlockElias2;
		argFlags[Args::ue1] = Format::UnaryElias1;
		argFlags[Args::ue2] = Format::UnaryElias2;
		argFlags[Args::ur1] = Format::UnaryRice;
		argFlags[Args::lzs] = Format::AlignedLZSS;
		argFlags[Args::r] = Format::FlagReverse;
		argFlags[Args::e] = Format::FlagEndMarker;
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

	if ((format & Format::Mask) == 0)
	{
		format |= Format::BlockElias1;
	}

	FILE* pInputFile;
	if (fopen_s(&pInputFile, args[1], "rb"))
	{
		printf(pInputFileError);
		return 0;
	}

	if (fseek(pInputFile, 0, SEEK_END))
	{
		fclose(pInputFile);
		printf(pInputFileError);
		return 0;
	}

	uint32_t inputFileSize = ftell(pInputFile);
	rewind(pInputFile);

	std::unique_ptr<uint8_t[]> spInputStream = std::make_unique<uint8_t[]>(inputFileSize);
	size_t bytesRead = fread_s(spInputStream.get(), inputFileSize, 1, inputFileSize, pInputFile);
	fclose(pInputFile);

	if (bytesRead != inputFileSize)
	{
		printf(pInputFileError);
		return 0;
	}

	BitStream packedStream;
	if (Compress(spInputStream.get(), inputFileSize, format, packedStream) == false)
	{
		printf(pPackError);
	}

	FILE* pOutputFile;
	if (fopen_s(&pOutputFile, args[2], "wb"))
	{
		printf(pOutputFileError);
		return 0;
	}

	size_t bytesWritten = fwrite(packedStream.Data(), 1, packedStream.Size(), pOutputFile);
	fclose(pOutputFile);

	if (bytesWritten != packedStream.Size())
	{
		printf(pOutputFileError);
		return 0;
	}

	return 0;
}
