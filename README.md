# bzpack
Bzpack is a data compression utility primarily targeting retrocomputing and demoscene enthusiasts. Given the artificially imposed size limits on productions like 256 B, 512 B or 1 KiB intros, the ability to implement a short decoder is nearly as important as the efficiency of the compression format itself. Bzpack doesn’t claim to be a state-of-the-art, general-purpose archiver. The design goal is to minimize the overall program length with acceptable trade-off between simplicity and efficiency. Special consideration was given to vintage computing platforms like Sinclair ZX Spectrum.

## Format Overview
All supported formats are based on the Lempel–Ziv–Storer–Szymanski algorithm. The compressed stream consists of:
1. strings of uncompressed symbols (literals),
2. reusable symbol sequences represented as offset-length pairs (phrases).

Bzpack encodes offsets as raw 8-bit values. Literals are encoded in two possible ways:
1. as a sequence of bytes preceded by block length (the “block” method),
2. using 1-bit flag for every literal symbol (the “unary” method).

Phrase and literal lengths are encoded using universal codes such as Elias-Gamma.

### A Note on Elias-Gamma Coding
The canonical form of Elias-Gamma code consists of *N* zeroes followed by a *(N + 1)*-bit binary number. For instance, the number 12 is encoded as 000**1100**.

Bzpack uses an alternative form in which the bits are interleaved: **1**0**1**0**0**0**0**.

Assuming that the most significant bit is implicit, the interleaved zeroes can be seen as 1-bit flags indicating whether another significant bit follows. This interleaved form lends itself to a very efficient decoder implementation. The actual format as output by the compressor is (**1**)1**1**1**0**1**0**0.

That is:
1. the most significant bit is not output,
2. subsequent significant bits are preceded by 1 indicating their presence,
3. 0 indicates the end of sequence.

Therefore, the resulting codeword has exactly the same length as its canonical form.

#### Thanks

I have used valuable input from multiple people, including Aleksey "introspec" Pichugin, Slavomir "Busy" Labsky and Pavel "Zilog" Cimbal. Let me also take the opportunity to recognize the work of Einar Saukas.
