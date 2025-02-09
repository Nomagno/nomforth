# nomforth
Just a simple Forth that I'll expand as I want to add more features.

Currently, it has only the most basic getline-based support for input.

Simply compile and link `prims.c` and `forth.c`. When running the program. For use with `runhelper.sh`, ensure the name of the binary executable is `fex`.

You can use `m.` at any point to get a memory dump of the first 4096 cells of the VM. There are only 32-bit cells, and no native byte-level access facilities are provided.

You can change the constants in `prims.h` and `forth.h` to add/remove/configure primitives and change the VM memory layout, respectively.
The rest of primitives are bootstrapped up in `prims.fs`. Make sure to edit it accordingly when touching the rest of the code. Then, building on `prim.fs`, `core.fs` semi-portably bootstraps up to a good part of CORE foth-standard.org word set.

This Forth has a major difference from most other systems: it does not use the HERE pointer natively. All execution tokens contain a header with a cell for their name, and another header cell representing: the highest nibble for auxiliary type info (for the `SEE` disassembler and such), the next nibble for immediacy info, the next byte which will be used for having multi-purpose "word planes" in the future, and the lowest two bytes for the actual size of the word. The main practical consequence is that there is no way to de-allocate dictionary space with negative ALLOT values (yet).

There is no native file I/O at the moment. Because of this, a script called `runhelper.sh` that can be executed along with `prims.fs` and `core.fs` (and any other files to prepend, in the form `file1.fs file2.fs ...`) has been included, that allows to prepend any amount of files to be ran at the beggining of the interactive session.

The current target is to get a minimal port of `mini-oof.fs` (replacing +LOOP with LOOP) from gforth to work. At the moment it almost works but crashes in the vtable lookup phase of method execution.

---

Copyright (c) 2025 Nomagno.

Project released under the MIT license, see the LICENSE file for details.
