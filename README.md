# bzpack

Bzpack is a data compression utility designed for retrocomputing and demoscene enthusiasts. Given the stringent size limits on programs like 256-byte, 512-byte, or 1 KiB intros, the ability to implement a compact decoder is nearly as important as the efficiency of the compression format itself. While Bzpack doesn't aim to be a state-of-the-art, general-purpose packer like Einar Saukas' excellent [ZX0](https://github.com/einar-saukas/ZX0), its goal is to strike a balance between simplicity and efficiency to minimize the overall program size. Special consideration has been given to vintage computing platform Sinclair ZX Spectrum.

## Format Overview

All currently supported formats are based on the Lempel–Ziv–Storer–Szymanski algorithm. The compressed stream is composed of two block types:
1. Strings of uncompressed bytes (literals).
2. Reusable byte sequences represented as offset-length pairs (phrases).

The encoding methods for literals and phrases vary across formats, and the efficiency depends on the structure of the input data. Therefore, it is recommended to try multiple formats to determine the best fit.

### A Note on Elias-Gamma Encoding

The canonical form of Elias-Gamma code consists of *N* zeroes followed by a *(N + 1)*-bit binary number. For instance, the number 12 is encoded as 000**1100**. In his paper "Universal codeword sets and representations of the integers", Peter Elias devised also an alternative form in which the bits are interleaved: **1**0**1**0**0**0**0**.

Assuming that the most significant bit is implicit, the interleaved zeroes can be seen as 1-bit flags which indicate whether another significant bit follows. This interleaved form lends itself to an efficient decoder implementation in assembly language. Bzpack borrows this approach except that the flags are inverted. Therefore, the actual code for number 12 would be 1**1**1**0**1**0**0. That is:

* The most significant bit is not stored.
* Subsequent significant bits are preceded by 1 indicating their presence.
* 0 marks the end of sequence.

### Elias-Gamma 1..N vs 2..N

Elias-Gamma code is able to represent positive integers 1..N. However, the code can be offset to 2..N which leads to codewords that favor the most common phrase lengths 2..7:

Regular Elias-Gamma code 1..N:
```
1: 0
2: 100
3: 110
4: 10100
5: 10110
6: 11100
7: 11110
```
Offset Elias-Gamma code 2..N:
```
2: 00
3: 10
4: 0100
5: 0110
6: 1100
7: 1110
8: 010100
```
In the following text symbol *E1* denotes Elias-Gamma code 1..N and *E2* denotes Elias-Gamma code 2..N.

## Format Description

### LZ

LZ is a straightforward byte-aligned format which is interpreted as follows:

* `ccccccc1` – Copy the next `ccccccc` bytes to the output.
* `ccccccc0`, `ffffffff` – Copy `ccccccc` bytes from the offset `ffffffff` relative to the current output position.
* `00000000` or `00000001` – End of stream.

The compression ratio is decent but certainly not spectacular. However, the decoder is very short and this advantage becomes apparent at the extreme end of the scale, e.g. while coding 256-byte intros. While other methods might produce a shorter stream, the combined length of stream and decoder mostly favors LZS.

Supported options:

* Offset increment (1..256 instead of 1..255).
* Size increment (1..128 instead of 1..127).
* End of stream marker.

### E1

The E1 format encodes literals as byte sequences preceded by *E1* number representing the block length. Phrases are stored as *E1* length followed by raw 8-bit offset. There's a 1-bit flag after each *E1* code indicating the block type.

* `E1`, `1` – Copy the next `E1` bytes to the output.
* `E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from the offset `ffffffff` relative to the current output position.
* `E1` > 255 - End of stream.

The compression ratio is significantly improved over the previous format and the decoder still manages to be short.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End of stream marker.

### E1ZX

E1ZX is an optimized form of E1 which specifically targets Sinclair ZX Spectrum. The stream is exactly the same length but certain values are stored as their complements. This allows simplification of the decompressor's initialization sequence, resulting in even shorter code. 512-byte or 1 KiB demos is where the format is most useful.

Supported options:

* Offset increment (1..256 instead of 1..255).

### UE2

The UE2 format encodes literals on a per-byte basis. For every literal byte there’s a 1-bit flag indicating the byte’s presence. If the flag is not set, there's a phrase which is stored as *E2* length combined with plain 8-bit offset.

* `1`, `bbbbbbbb` – Copy byte `bbbbbbbb` to the output.
* `0`, `E2`, `ffffffff` – Copy `E2` bytes from the offset `ffffffff` relative to the current output position.
* `E2` > 255 - End of stream.

The nature of this format is hit-or-miss. It usually performs better than LZS but worse than the other formats. However, that doesn't automatically rule out its use for particularly fitting data blocks.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End of stream marker.

#### Thanks

I have used valuable input from Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal. Let me also take the opportunity to recognize the work of Einar Saukas.
