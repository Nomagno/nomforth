# nomforth
Just a simple Forth that I'll expand as I want to add more features.

Currently, it has only the most basic getline-based support for input.

Simply compile and link `prims.c` and `forth.c`. When running the program.

You can use `m.` at any point to get a memory dump of the first 4096 cells of the VM, which in its default configuration are basically all that will be used in normal program execution anyways. There are only 32-bit cells, and no native byte-level access facilities are provided.

You can change the constants in `prims.h` and `forth.h` to add/remove/configure primitives and change the VM memory layout, respectively.

There is no native file I/O at the moment. Because of this, a script called `runhelper.sh` that can be executed along with `core.fs` (and any other files to prepend, in the form `file1.fs file2.fs ...`) has been included, that allows to prepend any amount of files to be ran at the beggining of the interactive session.

The current target is to get a minimal port of `mini-oof.fs` (replacing +LOOP with LOOP and aliasing ?DO as DO) from gforth to work. At the moment it almost works but crashes in the vtable lookup phase of method execution.

---

Copyright (c) 2025 Nomagno.

Project released under the MIT license, see the LICENSE file for details.
