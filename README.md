# nomforth
Just a simple Forth that I'll expand as I want to add more features.

Currently, it has only the most basic getline-based support for input.

Simply compile and link `prims.c` and `forth.c`. When running the program.

You can use `m.` at any point to get a memory dump of the first 4096 cells of the VM, which in its default configuration are basically all that will be used in normal program execution anyways. There are only 32-bit cells, and no native byte-level access facilities are provided.

You can change the constants in `prims.h` and `forth.h` to add/remove/configure primitives and change the VM memory layout, respectively.

It is recommended to copy paste `core.fs` to bootstrap some more basic but non-primitive words. File and other I/O facilities are coming in the near future.

The current target is to get a minimal port of `mini-oof.fs` (replacing +LOOP with LOOP and aliasing ?DO as DO) from gforth to work. At the moment it almost works but crashes in the vtable lookup phase of method execution.

---

Copyright (c) 2025 Nomagno.

Project released under the MIT license, see the LICENSE file for details.
