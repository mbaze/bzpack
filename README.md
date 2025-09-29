# bzpack

Bzpack is a data compression tool for sizecoding and retrocomputing enthusiasts. In tiny demoscene programs like 256-byte,
512-byte, or 1024-byte intros, a minimal decoder is just as crucial as compression efficiency, since any compression gains can
easily be offset by the extra bytes a more complex decoder requires. Bzpack therefore prioritizes extremely compact decoding
over maximum compression ratios, aiming for a tradeoff suited for sizecoding. Special consideration has been given to the
Sinclair ZX Spectrum.

## Usage

Bzpack is a command-line utility with the following usage format:

`bzpack.exe [-lzm|-ef8|-bx0|-bx2] [-r] [-e] [-o] [-l] [-n] <inputFile> [outputFile]`

For example, to compress a file named *"demo.bin"* in reverse direction using the BX2 format with the end-of-stream marker, the
command would be:

`bzpack.exe -bx2 -r -e demo.bin demo.bx2`

Here’s a list of supported command-line options (not including compression format identifiers `-lzm`, `-ef8`, `-bx0`, and
`-bx2`):

* `-r`: Compress in reverse direction. This option helps reduce the decoder size.
* `-e`: Add an end-of-stream marker. Useful for general-purpose decompression, but often unnecessary for minimalist programs.
* `-o`: Extend the offset range by 1. Supported by some formats; can produce a shorter stream at the cost of a larger decoder.
* `-l`: Extend the block length by 1. Supported by some formats; can shorten the stream, but requires a larger decoder.
* `-n`: Produce natural stream without stream-level optimizations (some formats use bitwise inversion to optimize decoding on
the Z80).

## Compression Format Structure

All supported formats are based on the Lempel–Ziv–Storer–Szymanski (LZSS) algorithm. The compressed stream consists of two types
of blocks:

* **Literals** - strings of uncompressed bytes stored directly in the stream.
* **Matches** - repeated byte sequences represented as offset-length pairs, where the offset points back to already decompressed
data relative to the current output position.

Compression efficiency varies between the supported formats, due to differences in how they encode literals and matches. Their
performance depends on the structure and statistical properties of the input, such as the frequency and distribution of match
lengths and offsets. Therefore, it is a good idea to try multiple formats to determine the best fit. In general, numbers are
represented either as raw bytes or as Elias-Gamma values, read from a bit stream that operates independently of natural byte
boundaries.

### Elias-Gamma Encoding

Most supported compression formats use Elias-Gamma encoding for offsets and lengths. The canonical Elias-Gamma code has *N*
leading zeroes followed by an *(N + 1)*-bit binary number. For example, 12 is encoded as 000**1100**. In his paper *"Universal
codeword sets and representations of the integers,"* Peter Elias also proposed an alternative representation in which the bits
are interleaved. In this format, the most significant bit is implicitly assumed, and each subsequent significant bit is preceded
by a flag bit indicating its presence. This representation is well-suited for efficient decoder implementation in assembly
language, and Bzpack adopts this approach. As a result, the actual code for the number 12 becomes 1**1**1**0**1**0**0, where:

* The most significant bit is not stored.
* Each subsequent significant bit is preceded by a 1.
* A 0 marks the end of the sequence.

Elias-Gamma codes represent positive nonzero integers as follows:

```
1: 0
2: 100
3: 110
4: 10100
5: 10110
6: 11100
7: 11110
8: 1010100
...
```

In the following text, we denote the Elias-Gamma code of integer *N* as `Elias(N)`, and bit patterns are written as `%nnnnnnnn`.

## Description of Supported Formats

The descriptions below refer to stream interpretation at the logical level, that is, how values appear in CPU registers at
runtime. However, in all formats except LZM, the physical representation in the stream differs. Unaligned bits are inverted to
optimize byte-level fetching in the Z80 decoder. This behavior can be suppressed with the `-n` command line option.

All formats use globally optimal parsers, staying true to the sizecoding spirit where every byte matters. For the BX0 and BX2
formats, the parser must explore all possible coding paths using all available repeat offsets, giving it roughly quadratic
computational complexity in the size of the input block. For blocks around 1 KiB, the process is essentially instantaneous.
However, for blocks larger than ~16 KiB, it might take even several minutes to complete.

### LZM

LZM is a straightforward, byte-aligned format interpreted as follows:

* `%nnnnnnn1` – Copy the next `%nnnnnnn` bytes to the output.
* `%nnnnnnn0`, `%oooooooo` – Copy `%nnnnnnn` bytes from an offset of `%oooooooo`.
* `%00000000` or `%00000001` – End of stream.

The main strength of this format is its extremely compact decoder, making it ideal for highly constrained scenarios such as
256-byte intros. While other methods may yield a smaller compressed stream, the combined size of stream and decoder can make LZM
the better choice.

Supported options:

* Offset range extension (1..256 instead of 1..255).
* Block size extension (1..128 instead of 1..127).
* End-of-stream marker.

### EF8

The EF8 format encodes block lengths as Elias-Gamma values, followed by a 1-bit flag indicating the block type:

* `Elias(N)`, `1` – Copy the next `N` bytes to the output.
* `Elias(N)`, `0`, `%oooooooo` – Copy `N + 1` bytes from an offset of `%oooooooo`.
* `Elias(N)` where `N > 255` - End of stream.

The compression ratio is significantly better than in the LZM format, and the decoder remains relatively compact.

Supported options:

* Offset range extension (1..256 instead of 1..255).
* End-of-stream marker.

### BX0

BX0 is a variant of Einar Saukas' popular [ZX0](https://github.com/einar-saukas/ZX0) with additional size optimizations. The
format disallows consecutive literals, and this implicit constraint frees up one bit of information, allowing a distinction
between a regular match and a "repeat match" that uses the most recent offset. Blocks are encoded as follows:

* `1`, `Elias(N)` – If following a match, copy the next `N` bytes to the output. If following a literal, copy `N` bytes from
the most recent offset. The flag bit for the very first literal is not stored in the stream.
* `0`, `Elias(O)`, `%ooooooo`, `Elias(N)` – Copy `N + 1` bytes from an offset of `(O << 7) | %ooooooo`. The leading flag bit of
`Elias(N)` is stored as the least significant bit of the byte containing `%ooooooo`. An offset of 16512 or greater indicates the
end of the stream.

Supported options:

* Offset range extension (1..16384 instead of 1..16383).
* End-of-stream marker.

### BX2

BX2 is a modification of [ZX2](https://github.com/einar-saukas/ZX2) that allows for a more compact decoder. It is essentially a
lightweight BX0 variant with raw 8-bit offset encoding. Blocks are encoded as follows:

* `Elias(N)`, `1` – If following a match, copy the next `N` bytes to the output. If following a literal, copy `N` bytes from the
most recent offset.
* `Elias(N)`, `0`, `%oooooooo` – Copy `N + 1` bytes from an offset of `%oooooooo`.
An offset of 0 indicates the end of the stream.

Supported options:

* Offset range extension (1..256 instead of 1..255).
* End-of-stream marker.

#### Disclaimer

This project was created for personal exploration and enjoyment. While I hope others may find it useful, I cannot guarantee its
fitness for any particular use case, backwards compatibility, long-term maintenance, or multi-platform support. The code is
released under a liberal license, so feel free to fork it and adapt it to your needs.

#### Acknowledgments

I would like to acknowledge the contributions of Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal,
and also take this opportunity to recognize the work of Einar Saukas.
