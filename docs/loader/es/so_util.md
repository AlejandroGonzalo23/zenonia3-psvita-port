# `loader/so_util.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `game_log` (line ~1)

**Source File:** `loader/so_util.c`

> so_util.c -- utils to load and hook .so modules
>
> Copyright (C) 2021 Andy Nguyen
>
> This software may be modified and distributed under the terms
> of the MIT license.	See the LICENSE file for details.

---

## `ku_memcpy` (line ~19)

**Source File:** `loader/so_util.c`

> Real hardware: unprivileged sceKernelAllocMemBlock() cannot create
> executable memory (W^X enforced by the MMU) -- kuKernelAllocMemBlock is
> kubridge's kernel-level allocator that can, and kuKernelCpuUnrestrictedMemcpy/
> kuKernelFlushCaches are needed to write into and sync that memory. Without
> this, the text segment ends up RW-only and any attempt to execute code from
> it faults with a Prefetch Abort exactly at the first instruction fetched.

---

## `ku_flush_caches` (line ~25)

**Source File:** `loader/so_util.c`

> Nor does it implement kuKernelFlushCaches. Vita3K's CPU emulation always
> reads fresh memory (no real instruction cache to keep coherent), so this is
> a safe no-op under EMULATOR_BUILD.

---

## `ku_memcpy` (line ~31)

**Source File:** `loader/so_util.c`

> Real hardware: unprivileged sceKernelAllocMemBlock() cannot create
> executable memory (W^X enforced by the MMU) -- kuKernelAllocMemBlock is
> kubridge's kernel-level allocator that can, and kuKernelCpuUnrestrictedMemcpy/
> kuKernelFlushCaches are needed to write into and sync that memory. Without
> this, the text segment ends up RW-only and any attempt to execute code from
> it faults with a Prefetch Abort exactly at the first instruction fetched.

---

## `emu_patch_size` (line ~149)

**Source File:** `loader/so_util.c`

> Vita3K does not implement kuKernelAllocMemBlock (fixed-address allocation),
> which the code below normally relies on to place the patch/text/data blocks
> at exact, contiguous addresses (mirroring a single mmap of the whole module
> image, like a real ELF loader would do). Since we can't request specific
> addresses under Vita3K, reserve ONE big block up front sized to fit the
> whole image contiguously, and sub-allocate patch/text/data regions from it
> via pointer arithmetic instead of separate fixed-address OS allocations.

---

## `opt` (line ~184)

**Source File:** `loader/so_util.c`

> Allocate arena for code patches, trampolines, etc
> Sits exactly under the desired allocation space

---

## `so_util.c` (line ~226) (line ~226)

**Source File:** `loader/so_util.c`

> Use the .text segment padding as a code cave
> Word-align it to make it simpler for instruction arena allocation

---

## `so_util.c` (line ~336) (line ~336)

**Source File:** `loader/so_util.c`

> patch/text/data_blockid[] all alias the single emu_blockid arena here,
> so only free it once instead of once per alias.

---

## `so_resolve` (line ~482)

**Source File:** `loader/so_util.c`

> Ooops, this shouldn't have happened.

---

## `inrange` (line ~627)

**Source File:** `loader/so_util.c`

> alloc_arena: allocates space on either patch or cave arenas,
> range: maximum range from allocation to dst (ignored if NULL)
> dst: destination address

---

## `so_util.c` (line ~638) (line ~638)

**Source File:** `loader/so_util.c`

> alloc_arena: allocates space on either patch or cave arenas,
> range: maximum range from allocation to dst (ignored if NULL)
> dst: destination address

---

## `so_util.c` (line ~667) (line ~667)

**Source File:** `loader/so_util.c`

> If the register we're reading the offset from is the same as the one we're writing,
> delay it to the very end so that the base pointer isn't clobbered

---

## `trampoline_sz` (line ~679)

**Source File:** `loader/so_util.c`

> Perform the delayed load if needed

---

## `index` (line ~695)

**Source File:** `loader/so_util.c`

> Create sign extended relative address rel_addr

---

## `idx` (line ~714)

**Source File:** `loader/so_util.c`

> This is meant to work around crashes due to unaligned accesses (SIGBUS :/) due to certain
> kernels not having the fault trap enabled, e.g. certain RK3326 Odroid Go Advance clone distros.

---

## `so_util.c` (line ~727) (line ~727)

**Source File:** `loader/so_util.c`

> This is meant to work around crashes due to unaligned accesses (SIGBUS :/) due to certain
> kernels not having the fault trap enabled, e.g. certain RK3326 Odroid Go Advance clone distros.

---
