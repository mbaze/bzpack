# bzpack

Bzpack is a data compression utility designed for retrocomputing and demoscene enthusiasts. Given the stringent size limits
on programs like 256-byte, 512-byte, or 1024-byte intros, implementing a compact decoder is almost as crucial as the efficiency
of the compression format itself. While Bzpack isn't intended to be a general-purpose packer like Einar Saukas' excellent
[ZX0](https://github.com/einar-saukas/ZX0), its goal is to balance simplicity and efficiency to minimize overall program size.
Special consideration has been given to vintage computing platform Sinclair ZX Spectrum.

## Usage

Bzpack is a command-line utility with the following usage format:

`bzpack.exe [-lz|-e1|-e1zx|-bx2|-ue2] [-r] [-e] [-o] [-l] <inputFile> [outputFile]`

For example, to compress a file called "demo.bin" using the BX2 format with the end-of-stream marker, the command would be:

`bzpack.exe -bx2 -e demo.bin demo.bx2`

A complete list of options can be accessed from the command line by running bzpack without any parameters.

## Format Overview

All supported formats are based on the Lempel–Ziv–Storer–Szymanski algorithm. The compressed stream consists of two block types:

1. *Literals:* Strings of uncompressed bytes stored directly in the stream.
2. *Matches:* Repeated byte sequences represented as offset-length pairs, where the offset is relative to the output position.

The encoding methods for literals and matches vary between formats, and their efficiency depends on the structure of the input
data. Therefore, trying multiple formats is recommended to determine the best fit. Generally, numbers are represented either as
raw bytes or as Elias-Gamma values, read from a bit stream that works independently of natural byte boundaries.

### A Note on Elias-Gamma Encoding

The canonical form of the Elias-Gamma code consists of `N` leading zeroes followed by a `(N + 1)`-bit binary number. For
example, the number 12 is encoded as 000**1100**. In his paper "Universal codeword sets and representations of the integers",
Peter Elias also proposed an alternative representation in which the bits are interleaved: **1**0**1**0**0**0**0**. In this
format, the most significant bit is always present, and the zeroes act as 1-bit flags indicating whether another significant
bit follows. This representation is particularly well-suited for efficient decoder implementation in assembly language.Bzpack
adopts this approach with one minor tweak: the flags are inverted. As a result, the actual code for the number 12 becomes
1**1**1**0**1**0**0, where:

* The most significant bit is implicitly assumed.
* Each subsequent significant bit is preceded by a 1, indicating its presence.
* A 0 marks the end of the sequence.

#### Elias-Gamma 1..N vs 2..N

The Elias-Gamma code represents positive integers in the range 1..N. However, by shifting the range to 2..N, the resulting
codewords can sometimes be optimized for the most common match lengths.

Standard Elias-Gamma code for the range 1..N:
```
1: 0
2: 100
3: 110
4: 10100
5: 10110
6: 11100
7: 11110
```
Offset Elias-Gamma code for the range 2..N:
```
2: 00
3: 10
4: 0100
5: 0110
6: 1100
7: 1110
8: 010100
```
In the following text, the symbol `E1` represents the Elias-Gamma code for the range 1..N, while `E2` represents the
Elias-Gamma code for 2..N.

## Description of Supported Formats

### LZ

LZ is a straightforward, byte-aligned format interpreted as follows:

* `ccccccc1` – Copy the next `ccccccc` bytes to the output.
* `ccccccc0`, `ffffffff` – Copy `ccccccc` bytes from an offset of `ffffffff`, relative to the current output position.
* `00000000` or `00000001` – End of stream.

The compression ratio is decent but not exceptional. However, the decoder is extremely compact, making it usable in highly
constrained scenarios, such as 256-byte intros. While other methods may produce a shorter compressed stream, the combined size
of the stream and decoder can make LZ the better choice.

Supported options:

* Offset increment (1..256 instead of 1..255).
* Size increment (1..128 instead of 1..127).
* End-of-stream marker.

### E1

The E1 format encodes block length as an `E1` value, followed by a 1-bit flag indicating the block type:

* `E1`, `1` – Copy the next `E1` bytes to the output.
* `E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from an offset of `ffffffff`, relative to the current output position.
* `E1` > 255 - End of stream.

The compression ratio is significantly improved over the previous format and the decoder still manages to be short.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End-of-stream marker.

### E1ZX

E1ZX is an optimized variant of E1, specifically designed for the Sinclair ZX Spectrum. While the stream length remains
unchanged, certain values are stored as their complements, simplifying decompressor initialization and further reducing code
size. This format is primarily intended for 512-byte and 1024-byte intros.

Supported options:

* Offset increment (1..256 instead of 1..255).

### BX2

BX2 is a slight modification of Einar Saukas' [ZX2](https://github.com/einar-saukas/ZX2) that allows for a more efficient
decoder. The format disallows consecutive literals and this implicit constraint frees up one bit of information, allowing
a distinction between a regular match and a 'repeat match' that reuses the most recent offset. Blocks are encoded as follows:

* `E1`, `1` – Copy the next `E1` bytes to the output.
* `E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from an offset of `ffffffff`, relative to the current output position.
An offset of 0 indicates the end of the stream.
* `E1`, `1` – If following a literal, copy `E1` bytes from the most recent offset.

The format employs an experimental exhaustive parser that, in theory, achieves a globally optimal encoding. However,
compression may take even several minutes for blocks of 8 KiB or higher.

### UE2

The UE2 format encodes literals on a per-byte basis, using 1-bit flag for each byte. If the flag is not set, it indicates
a match of length `E2` combined with plain 8-bit offset.

* `1`, `bbbbbbbb` – Copy byte `bbbbbbbb` to the output.
* `0`, `E2`, `ffffffff` – Copy `E2` bytes from an offset of `ffffffff`, relative to the current output position.
* `E2` > 255 - End of stream.

This format tends to be hit-or-miss. It usually outperforms LZ but falls short compared to other formats. However, it can still
be useful for data blocks where it happens to be a good fit.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End-of-stream marker.

#### Acknowledgments

I would like to acknowledge the contributions of Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and
Pavel "Zilog" Cimbal. I would also like to take this opportunity to recognize the work of Einar Saukas.
