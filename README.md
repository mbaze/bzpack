# bzpack

Bzpack is a data compression utility which targets retrocomputing and demoscene enthusiasts. Given the artificially imposed size limits on programs like 256 B, 512 B or 1 KiB intros, the ability to implement a short decoder is nearly as important as the efficiency of the compression format itself. Bzpack doesn’t claim to be a state-of-the-art, general purpose packer. The goal is to achieve acceptable trade-off between simplicity and efficiency in order to minimize the overall program length. Special consideration was given to vintage computing platform Sinclair ZX Spectrum.

## Format Overview

All currently supported formats are based on the Lempel–Ziv–Storer–Szymanski algorithm. The compressed stream consists of two block types:
1. Strings of uncompressed bytes (literals).
2. Reusable byte sequences represented as offset-length pairs (phrases).

The way literals and phrases are encoded is different for every format. Each of them makes different trade-offs and the efficiency also depends on the nature of input data. Therefore, it's best to try multiple options.

### A Note on Elias-Gamma Encoding

The canonical form of Elias-Gamma code consists of *N* zeroes followed by a *(N + 1)*-bit binary number. For instance, the number 12 is encoded as 000**1100**. In his paper "Universal codeword sets and representations of the integers", Peter Elias also devised an alternative form in which the bits are interleaved: **1**0**1**0**0**0**0**.

Assuming that the most significant bit is implicit, the interleaved zeroes can be seen as 1-bit flags that indicate whether another significant bit follows. This interleaved form lends itself to a very efficient decoder implementation. Bzpack borrows this approach but the flags are inverted. Therefore, the actual code for number 12 would be 1**1**1**0**1**0**0, that is:

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
Subsequent descriptions use the symbols *E1* for Elias-Gamma code 1..N and *E2* for Elias-Gamma code 2..N.

## Format Description

### LZS

The LZS is a straightforward byte-aligned format which is interpreted as follows:

* `ccccccc1` – Copy the next `ccccccc` bytes to the output.
* `ccccccc0`, `ffffffff` – Copy `ccccccc` bytes from the offset `ffffffff` relative to the current output position.
* `00000000` or `00000001` – End of stream.

The compression ratio is decent but certainly not spectacular. However, the decoder is very short and this advantage becomes apparent at the extreme end of the scale, e.g. while coding 256 B intros.

Supported options:

* Offset increment (1..256 instead of 1..255).
* Size increment (1..128 instead of 1..127).
* End of stream marker.

### E1E1

The E1E1 format encodes literals as byte sequences preceded by *E1* number denoting the block length. Phrases are stored as *E1* length combined with plain 8-bit offset. Each *E1* code is followed by 1-bit flag indicating the block type.

* `E1`, `1` – Copy the next `E1` bytes to the output.
* `E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from the offset `ffffffff` relative to the current output position.
* `E1` > 255 - End of stream.

The compression ratio is significantly improved over the previous format and the decoder still manages to be short.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End of stream marker.

### E1X1

The E1X1 format is similar to E1E1. However, in this format there can be no consecutive literals which means that only a phrase (or end of stream) can follow a literal. When this occurs, the “spare” flag signals extended offset. The format is interpreted as follows:

After a literal:

* `E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from the offset `1ffffffff` relative to the current output position.
* `E1`, `1`, `ffffffff` – Copy `E1 + 1` bytes from the offset `0ffffffff` relative to the current output position.

After a phrase:

* `E1`, `0`, `ffffffff` – Copy `E1 + 1` bytes from the offset `0ffffffff` relative to the current output position.
* `E1`, `1` – Copy the next `E1` bytes to the output.

The limitation of this method is that the compression algorithm will fail if there's a literal exceeding the length of 255. However, this is unlikely and E1X1 usually outperforms E1E1 enough to justify its longer decoder.

Supported options:

* Offset increment (1..256 / 1..512 instead of 1..255 / 1..511).
* End of stream marker.

### UE2

The UE2 format encodes literals on a per-byte basis. For every literal byte there’s a 1-bit flag indicating the byte’s presence. If the flag is not set, there's a phrase which is stored as *E2* length combined with plain 8-bit offset.

* `1`, `bbbbbbbb` – Copy byte `bbbbbbbb` to the output.
* `0`, `E2`, `ffffffff` – Copy `E2` bytes from the offset `ffffffff` relative to the current output position.
* `E2` > 255 - End of stream.

The nature of this format is hit-or-miss. It usually performs better than LZS but worse than the other formats. However, that doesn't automatically rule out its use for certain data blocks.

Supported options:

* Offset increment (1..256 instead of 1..255).
* End of stream marker.

## Comparison

The following chart shows the relative performance of ZX7mini, ZX2, E1E1 and E1X1 formats on a corpus of ZX Spectrum intros. Note that this comparison doesn't take into account the decoder size.

![image](https://user-images.githubusercontent.com/37623188/131914074-9e2bd774-f234-454d-9e48-06e76fa052cd.png)

#### Thanks

I have used valuable input from multiple people, including Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal. Let me also take the opportunity to recognize the work of Einar Saukas.
