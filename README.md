# bzpack

Bzpack is a data compression tool for sizecoding and retrocomputing enthusiasts. For tiny demoscene programs like 256-byte,
512-byte, or 1024-byte intros, a compact decoder is as important as compression efficiency. Bzpack isn't intended as a
general-purpose packer like [ZX0](https://github.com/einar-saukas/ZX0). Rather, it aims to strike a balance between size and
efficiency that's ideal for sizecoding, since complex encoding schemes may not produce a stream short enough to justify a larger
decoder. Special consideration has been given to the Sinclair ZX Spectrum.

## Usage

Bzpack is a command-line utility with the following usage format:

`bzpack.exe [-lz|-e1|-e1zx|-bx0|-bx2] [-r] [-e] [-o] [-l] <inputFile> [outputFile]`

For example, to compress a file named *"demo.bin"* in reverse direction using the BX2 format with the end-of-stream marker, the
command would be:

`bzpack.exe -bx2 -r -e demo.bin demo.bx2`

Here’s a list of supported command-line options (not including compression format identifiers):

* `-r` - Compress in reverse direction. In practice, this option helps reduce the decoder size.
* `-e` - Add an end-of-stream marker. Useful for general-purpose decompression, but often unnecessary for minimalist programs.
* `-o` - Extend the offset range by 1. Supported by some formats; can produce a slightly shorter stream at the cost of a larger
decoder.
* `-l` - Extend the block length by 1. Supported by some formats; similarly, can result in a shorter stream, but requires a
larger decoder.

## Compression Format Structure

All supported formats are based on the Lempel–Ziv–Storer–Szymanski (LZSS) algorithm. The compressed stream consists of two types
of blocks:

* **Literals** - strings of uncompressed bytes stored directly in the stream.
* **Matches** - repeated byte sequences represented as offset-length pairs, where the offset points back to already decompressed
data relative to the current output position.

The encoding methods for literals and matches vary between supported formats, and their efficiency depends on the structure and
statistical properties of the input, such as the frequency and distribution of match lengths and offsets. Therefore, it's a good
idea to try multiple formats to determine the best fit. In general, numbers are represented either as raw bytes or as
Elias-Gamma values, read from a bit stream that operates independently of natural byte boundaries.

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

In the following text, we denote the Elias-Gamma code of integer *N* as `Elias(N)`.

## Description of Supported Formats

### LZ

LZ is a straightforward, byte-aligned format interpreted as follows:

* `nnnnnnn1` – Copy the next `nnnnnnn` bytes to the output.
* `nnnnnnn0`, `oooooooo` – Copy `nnnnnnn` bytes from an offset of `oooooooo`, relative to the current output position.
* `00000000` or `00000001` – End of stream.

The compression ratio is decent but not exceptional. However, the decoder is extremely compact, making it usable in highly
constrained scenarios, such as 256-byte intros. While other methods may produce a shorter compressed stream, the combined size
of the stream and decoder can make LZ the better choice.

Supported options:

* Offset increment (1..256 instead of 1..255).
* Size increment (1..128 instead of 1..127).
* End-of-stream marker.

### E1

The E1 format encodes block lengths as Elias-Gamma values, followed by a 1-bit flag indicating the block type:

* `Elias(N)`, `1` – Copy the next `N` bytes to the output.
* `Elias(N)`, `0`, `oooooooo` – Copy `N + 1` bytes from an offset of `oooooooo`, relative to the current output position.
* `Elias(N)` where `N > 255` - End of stream.

The compression ratio is significantly better than in the LZ format, and the decoder remains relatively compact.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End-of-stream marker.

### E1ZX

E1ZX is an optimized variant of E1, specifically designed for the Sinclair ZX Spectrum. While the stream length remains
unchanged, certain values are stored as their complements, simplifying decoder initialization and further reducing code size.
This format is primarily intended for 512-byte and 1024-byte intros.

Supported options:

* Offset increment (1..256 instead of 1..255).

### BX0

BX0 is a variant of Einar Saukas' popular [ZX0](https://github.com/einar-saukas/ZX0) with additional size optimizations. The
format disallows consecutive literals, and this implicit constraint frees up one bit of information, allowing a distinction
between a regular match and a "repeat match" that uses the most recent offset. Blocks are encoded as follows:

* `1`, `Elias(N)` – If following a match, copy the next `N` bytes to the output. If following a literal, copy `N` bytes from
the most recent offset. The flag bit for the very first literal is not stored in the stream.
* `0`, `Elias(O)`, `ooooooo`, `Elias(N)` – Copy `N + 1` bytes from an offset of `(O << 7) | ooooooo`. The leading flag bit of
`Elias(N)` is stored as the least significant bit of the byte containing `ooooooo`. An offset of 16384 or greater indicates the
end of the stream.

The format employs an experimental exhaustive parser that achieves a globally optimal encoding, but compression may take even
several minutes for blocks of 4 KiB or higher.

Supported options:

* Offset increment (1..16384 instead of 1..16383).
* End-of-stream marker.

### BX2

BX2 is a modification of [ZX2](https://github.com/einar-saukas/ZX2) that allows for a more compact decoder. It is essentially a
lightweight BX0 variant with raw offset encoding. Blocks are encoded as follows:

* `Elias(N)`, `1` – If following a match, copy the next `N` bytes to the output. If following a literal, copy `N` bytes from the
most recent offset.
* `Elias(N)`, `0`, `oooooooo` – Copy `N + 1` bytes from an offset of `oooooooo`, relative to the current output position.
An offset of 0 indicates the end of the stream.

Like BX0, the format uses a slower, globally optimal exhaustive parser.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End-of-stream marker.

#### Acknowledgments

I would like to acknowledge the contributions of Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal,
and also take this opportunity to recognize the work of Einar Saukas.
