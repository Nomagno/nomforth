# nomforth
Just a simple indirect-threaded Forth with C primitives that I'll expand as I want to add more features.

Currently, it has only the most basic getline-based support for input, as well as a `GETC` to get a single character from the user at run-time.

To compile: `gcc -o fex core/forth.c primitives/prims.c primitives/oa.c`
For use with `runhelper.sh`, the name of the binary executable is `fex`.

There is no native file I/O at the moment. Because of this, a script called `runhelper.sh` is provided. It allows to prepend any amount of files to be ran at the beggining of the interactive session. Run it like `sh runhelper.sh forth_libs/bootstrap.fs forth_libs/utilities.fs ...`. Always load `forth_libs/bootstrap.fs` and `forth_libs/utilities.fs` first of all, as they build the core language.

To test out a full example, run `sh runhelper.sh forth_libs/bootstrap.fs forth_libs/utilities.fs forth_libs/disassembler.fs examples/example.fs`

You can use `HEX 10 FFFF 0 m. DECIMAL` at any point to get a memory dump of the first 64KiBs of the VM. There are only 32-bit cells, and no native byte-level access facilities are provided. Apart from traditional forth words like `MARKER`, for dynamic memory management the primitives `ALLOCATE` and `FREE` are provided for a simple heap.

The nomenclature for memory regions is a bit different: all strings are by default (unless somehow transferred with a custom-made word that contains `MOVE` and decrements the pad pointer) stored in a section known as the pad, that works with a regular arena allocator juts like the Forth dictionary itself. What regular Forth implementations call the scratch-pad, what is called here the "user memory", is a region of memory that the programmer can use however they want and that the "standard library" vows never to manipulate. It starts at the address returned by `USERMEM`

You can change the constants in `prims.h` and `forth.h` to add/remove/configure primitives and change the VM memory layout, respectively.
The rest of primitives are bootstrapped up in `bootstrap.fs`. Make sure to edit it accordingly when touching the rest of the code. Then, and `utilities.fs` semi-portably bootstraps up to a good part of the CORE foth-standard.org word set, plus some goodies such as local registers and anonymous words. However, the implementation does not claim to, nor attempt to be, standard compliant.

This Forth has a major difference from most other systems: it does not use the HERE pointer natively. All execution tokens contain a header with a cell for their name, and another header cell representing: the highest nibble for auxiliary type info (for the `SEE` disassembler and such), the next nibble for user-defined info, the next byte which will be used for having multi-purpose "word planes" in the future, and the lowest two bytes for the actual size of the word. The main practical consequence is that there is no way to de-allocate dictionary space with negative ALLOT values (yet).

A major difference from standard forth is that all of the standard library words accept and return (where it makes sense! for instant /string by definition works on arrays rather than strings, so it still accepts and returns addr+size) string addresses which must be `COUNT`ed to obtain the address+size representation, for the sake of consistency. This mostly applies to `PARSE` and `PARSE-NAME`, which have had their behaviour altered as described. There is also an `UNCOUNT` function that allows to revert a (important: non-manipulated) string from the address+size representation to just its prefixed string address.

---

Copyright (c) 2025 Nomagno.

Project released under the MIT license, see the LICENSE file for details.
