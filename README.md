# bzpack
Bzpack is a data compression utility that primarily targets retrocomputing and demoscene enthusiasts. Given the artificially imposed size limits on programs like 256 B, 512 B or 1 KiB intros, the ability to implement a short decoder is nearly as important as the efficiency of the compression format itself. Bzpack doesn’t claim to be a state-of-the-art, general-purpose compressor. The design goal is to minimize the overall program length with acceptable trade-off between simplicity and efficiency. Special consideration was given to vintage computing platforms like Sinclair ZX Spectrum.

## Format Overview
All supported formats are based on the Lempel–Ziv–Storer–Szymanski algorithm. The compressed stream consists of:
1. strings of uncompressed bytes (literals),
2. reusable byte sequences represented as offset-length pairs (phrases).

Bzpack encodes offsets as raw 8-bit values. Literals are encoded in two possible ways:
1. as a sequence of bytes preceded by block length (the “block” method),
2. using 1-bit flag for every literal symbol (the “unary” method).

Phrase and literal lengths are encoded using universal codes such as Elias-Gamma.

### A Note on Elias-Gamma Coding
The canonical form of Elias-Gamma code consists of *N* zeroes followed by a *(N + 1)*-bit binary number. For instance, the number 12 is encoded as 000**1100**. Peter Elias devised also an alternative form in which the bits are interleaved: **1**0**1**0**0**0**0**.

Assuming that the most significant bit is implicit, the interleaved zeroes can be seen as 1-bit flags that indicate whether another significant bit follows. This interleaved form lends itself to a very efficient decoder implementation. The actual format as output by the compressor is (**1**)1**1**1**0**1**0**0.

That is:
1. the most significant bit is not stored,
2. subsequent significant bits are preceded by 1 indicating their presence,
3. 0 marks the end of sequence.

Therefore, the resulting codeword has exactly the same length as the canonical form.

### Selecting Between 1..N and 2..N Codes

Elias-Gamma code is capable of representing arbitrary integer values 1..N. However, the code can be offset to 2..N and we can assign shorter codewords to lengths 2..7 which are statistically common. Let's see the difference:

Regular 1..N code:
```
1: 0
2: 100
3: 110
4: 10100
5: 10110
6: 11100
7: 11110
```
Offset 2..N code:
```
2: 00
3: 10
4: 0100
5: 0110
6: 1100
7: 1110
8: 10100
```
Sometimes, the alternative coding might be a better fit to the distribution of block lengths inside the input stream.

#### Thanks

I have used valuable input from multiple people, including Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal. Let me also take the opportunity to recognize the work of Einar Saukas.
