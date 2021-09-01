# bzpack
Bzpack is a data compression utility which targets retrocomputing and demoscene enthusiasts. Given the artificially imposed size limits on programs like 256 B, 512 B or 1 KiB intros, the ability to implement a short decoder is nearly as important as the efficiency of the compression format itself. Bzpack doesn’t claim to be a state-of-the-art, general purpose packer. The goal is to achieve acceptable trade-off between simplicity and efficiency in order to minimize the overall program length. Special consideration was given to vintage computing platforms like Sinclair ZX Spectrum.

## Format Overview
All currently supported formats are based on the Lempel–Ziv–Storer–Szymanski algorithm. The compressed stream consists of two kinds of blocks:
1. strings of uncompressed bytes (literals),
2. reusable byte sequences represented as offset-length pairs (phrases).

The way literals and phrases are represented is different for every format. Each of them makes a different set of trade-offs and the efficiency also depends on the nature of input data.

### A Note on Elias-Gamma Encoding
The canonical form of Elias-Gamma code consists of *N* zeroes followed by a *(N + 1)*-bit binary number. For instance, the number 12 is encoded as 000**1100**. In his 1975 paper "Universal codeword sets and representations of the integers", Peter Elias devised also an alternative form in which the bits are interleaved: **1**0**1**0**0**0**0**.

Assuming that the most significant bit is implicit, the interleaved zeroes can be seen as 1-bit flags that indicate whether another significant bit follows. This interleaved form lends itself to a very efficient decoder implementation. Bzpack borrows this approach but uses 1s instead of 0s. Therefore, the actual output would be (**1**)1**1**1**0**1**0**0, that is:

1. the most significant bit is not stored,
2. subsequent significant bits are preceded by 1 indicating their presence,
3. 0 marks the end of sequence.

### Elias-Gamma 1..N vs 2..N

Elias-Gamma code is able to represent positive integers 1..N. However, the code can be offset to 2..N resulting in codewords that favor the most common phrase lengths 2..7:

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
Subsequent descriptions use the symbols *E1* for Elias-Gamma code 1..N and *E2* for Elias-Gamma code 2..N.

## Format Description

### LZS

The LZS is a straightforward byte-aligned format which is interpreted as follows:

`ccccccc1` – Copy the next `ccccccc` bytes to the output.

`ccccccc0`, `ffffffff` – Copy `ccccccc` bytes from the offset `ffffffff` relative to the current output position.

`00000000` or `00000001` – End of stream.

The compression ratio is decent but certainly not spectacular. However, the decoder is very short and this advantage becomes apparent at the extreme end of the scale, e.g. while coding 256 B intros. Optionally, the format supports incremented block sizes and/or offsets at the expense of additional opcodes in the decoder.

### E1E1

The E1E1 format encodes literals as byte sequences preceded by *E1* code denoting the block length. Phrases are stored as *E1* length combined with plain 8-bit offset. There’s a 1-bit flag after each *E1* code indicating the block type.

`E1`, `1` – Copy the next `E1` bytes to the output.

`E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from the offset `ffffffff` relative to the current output position.

`E1` > 255 - End of stream.

The compression ratio is significantly improved over the previous format and the decoder still manages to be short. The format optionally supports incremented offset lengths.

### UE2

The UE2 format encodes literals on a per-byte basis. For every literal byte there’s a 1-bit flag indicating the byte’s presence. The opposite value of the flag is used to mark a phrase which is stored as *E2* length combined with plain 8-bit offset.

`1`, `bbbbbbbb` – Copy byte `bbbbbbbb` to the output.

`0`, `E2`, `ffffffff` – Copy `E2` bytes from the offset `ffffffff` relative to the current output position.

`E2` > 255 - End of stream.

The nature of this format is hit-or-miss. It usually performs better than LZS but worse than the other formats. However, that doesn't automatically rule out its use for particularly fitting data blocks. There's the option to increment the offset.

#### Thanks

I have used valuable input from multiple people, including Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal. Let me also take the opportunity to recognize the work of Einar Saukas.
