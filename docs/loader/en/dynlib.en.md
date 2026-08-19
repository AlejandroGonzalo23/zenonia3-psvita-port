# `loader/dynlib.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `zenonia_verbose_ui` (line ~27)

**Source File:** `loader/dynlib.c`

> The logo/title/menu (ui_status 0-2, see ZenoniaUIControllerView.java:
> UI_STATUS_LOGO/TITLE/MAINMENU) looks white and then black (confirmed
> by the user), without crash -- CMvTitleState::DrawZeroGrade/DrawTeamLogo
> (out_ghidra.c:105397/105434) make a white DrawFillRect and THEN a
> virtual call that draws the logo/art on top with an alpha fade
> (internal counter < 0x10 -> partial alpha, else -> opaque). Hypotheses without
> confirm yet (static investigation, no real console access):
> the fade never reaches alpha=255/blend opaque, or the blend/textencv active
> leaves the quad invisible. glColorPointer/glTexEnvf/glBlendFunc logs now
> they existed but they were limited to the first 10-20 calls (here it already happened
> the menu) -- the cap is removed WHILE the ui_status is in that range to
> be able to confirm with ONE more log in the real console which of the two hypotheses
> is the correct one. Remove this helper (and the `|| zenonia_verbose_ui()` that support it)
> use) once the underlying bug is resolved and confirmed.

---

## `GVUI_CONTROLLER_ACTIVE_COUNT_OFFSET` (line ~35)

**Source File:** `loader/dynlib.c`

> offset committed in GVUIController::GVUIController() (out_ghidra.c:26369,
> `memset(this+8,0,400); *(int*)(this+0x19c)=0;`) and reused identical by
> Draw()/PointerPress()/PointerMove()/PointerRelease() as loop limit
> about the up to 100 child pointers that start at this+8.

---

## `GVUI_CONTROLLER_ACTIVE_COUNT_OFFSET` (line ~42)

**Source File:** `loader/dynlib.c`

> offset committed in GVUIController::GVUIController() (out_ghidra.c:26369,
> `memset(this+8,0,400); *(int*)(this+0x19c)=0;`) and reused identical by
> Draw()/PointerPress()/PointerMove()/PointerRelease() as loop limit
> about the up to 100 child pointers that start at this+8.

---

## `zenonia_install_hide_dpad_hook` (line ~49)

**Source File:** `loader/dynlib.c`

> Call AFTER so_relocate/so_resolve (you need the module already with
> dynsym/dynstr resolved, see so_symbol()) and BEFORE the first call to
> any Native*() of the game -- GVUIPlayerController is constructed during
> normal engine start.

---

## `zenonia_install_hide_dpad_hook` (line ~60)

**Source File:** `loader/dynlib.c`

> Call AFTER so_relocate/so_resolve (you need the module already with
> dynsym/dynstr resolved, see so_symbol()) and BEFORE the first call to
> any Native*() of the game -- GVUIPlayerController is constructed during
> normal engine start.

---

## `glClearColorx_wrapper` (line ~85)

**Source File:** `loader/dynlib.c`

> Wrappers for fixed point OpenGL (GLES1)

---

## `convert_rgb565_to_rgba8888_neon` (line ~97)

**Source File:** `loader/dynlib.c`

> 4-strategy A/B testing for the RGB565 software framebuffer that the
> motor uploads per texture (see CLAUDE.md / CMakeLists.txt: RGB565_CONVERT_MODE
> -- SCALAR/LUT/NEON convert RGBA8888 to CPU, just change as; NATIVE
> it doesn't convert anything, it uploads the RGB565 as is and lets the GXM sampler do it
> read native). Select only ONE of the 4 (mutually exclusive, they are not
> independent flags).

---

## `p` (line ~109)

**Source File:** `loader/dynlib.c`

> Wrappers for fixed point OpenGL (GLES1)

---

## `convert_rgb565_to_rgba8888` (line ~130)

**Source File:** `loader/dynlib.c`

> Conversion buffer reused between calls: the engine uploads the
> full 800x480 framebuffer before EVERY quad (multiple times per frame),
> and making 1.5 MB malloc/free on each upload was part of the cost per
> frame. The result is valid only until the next call — the calls
> sites consume it immediately in glTexImage2D/glTexSubImage2D.
> It is not called in NATIVE mode (see call sites) -- it is left compiled the same by
> if you need to compare again in runtime.

---

## `new_buf` (line ~142)

**Source File:** `loader/dynlib.c`

> Conversion buffer reused between calls: the engine uploads the
> full 800x480 framebuffer before EVERY quad (multiple times per frame),
> and making 1.5 MB malloc/free on each upload was part of the cost per
> frame. The result is valid only until the next call — the calls
> sites consume it immediately in glTexImage2D/glTexSubImage2D.
> It is not called in NATIVE mode (see call sites) -- it is left compiled the same by
> if you need to compare again in runtime.

---

## `r5_to_8` (line ~157)

**Source File:** `loader/dynlib.c`

> Conversion buffer reused between calls: the engine uploads the
> full 800x480 framebuffer before EVERY quad (multiple times per frame),
> and making 1.5 MB malloc/free on each upload was part of the cost per
> frame. The result is valid only until the next call — the calls
> sites consume it immediately in glTexImage2D/glTexSubImage2D.
> It is not called in NATIVE mode (see call sites) -- it is left compiled the same by
> if you need to compare again in runtime.

---

## `glTexImage2D_wrapper` (line ~202)

**Source File:** `loader/dynlib.c`

> The engine uploads its internal software framebuffer (400x240, see Phase 1 of the
> plan) in RGB565. SCALAR/LUT/NEON convert it to RGBA8888 in CPU before
> upload it (same bug confirmed on real hardware for Zenonia 2, same
> engine). NATIVE is the variant without conversion: GL_UNSIGNED_SHORT_5_6_5 is
> defined in vitaGL.h (0x8363) and SceGxm supports native R5G6B5 in the
> sampler, so in theory the engine can upload the RGB565 as is and the
> GPU filters it by itself -- UNCONFIRMED IN HARDWARE (the original comment
> inherited from Zenonia 2 said "vitaGL does not handle it the same as the engine
> original wait" without leaving evidence that the passthrough has been tried
> pure; if corrupted texture/shifted color channel appears in the log,
> that's the first suspect).

---

## `glTexSubImage2D_wrapper` (line ~230)

**Source File:** `loader/dynlib.c`

> Force min/mag filters so that the texture is not treated as incomplete due to lack of mipmaps

---

## `pending_fixed_verts` (line ~268)

**Source File:** `loader/dynlib.c`

> Force min/mag filters so that the texture is not treated as incomplete due to lack of mipmaps

---

## `fixed_to_float_neon` (line ~291)

**Source File:** `loader/dynlib.c`

> A/B test against the scalar loop (see CLAUDE.md / CMakeLists.txt:
> OPTIMIZE_NEON_FIXED). Only valid when the array is tightly-packed
> (stride_elems == size, the normal case for these 3 attributes in this
> engine) -- the caller falls to the original scalar loop if it is not, instead of
> assume. Identical bit-by-bit result when scaling: divide by 65536.0f or
> multiply by its reciprocal (1/65536, exact power of 2 in float) no
> introduce extra rounding.

---

## `stride_elems` (line ~320)

**Source File:** `loader/dynlib.c`

> A/B test against the scalar loop (see CLAUDE.md / CMakeLists.txt:
> OPTIMIZE_NEON_FIXED). Only valid when the array is tightly-packed
> (stride_elems == size, the normal case for these 3 attributes in this
> engine) -- the caller falls to the original scalar loop if it is not, instead of
> assume. Identical bit-by-bit result when scaling: divide by 65536.0f or
> multiply by its reciprocal (1/65536, exact power of 2 in float) no
> introduce extra rounding.

---

## `glTexEnvf_wrapper` (line ~413)

**Source File:** `loader/dynlib.c`

> No log limit so far -- the engine calls this for each
> sprite/texture drawn, ALL frames (unlike their neighbors in
> this file, which already covers the first ~10-20 calls). Each
> game_log() does synchronous fprintf+fflush (blocking I/O to disk, same
> cost that ENABLE_VERBOSE_JNI_LOG documents in CLAUDE.md) -- without this cap
> it was one fflush per sprite drawn, enough to throw away the framerate of
> 60 at ~13 fps sustained on real console.

---

## `glBlendFunc_wrapper` (line ~426)

**Source File:** `loader/dynlib.c`

> Without wrapper until now (went directly to vitaGL) -- it is added ONLY for
> log in during logo/title/menu, see zenonia_verbose_ui() note above.

---

## `c` (line ~495)

**Source File:** `loader/dynlib.c`

> struct stat with the bionic layout (Android ARM 32-bit, NDK android-9) --
> It is NOT the one in newlib/vitasdk. The engine directly reads st_mode at the offset
> 16 and st_size at 48 (confirmed in Zenonia 2 by disassembling
> MC_fsFileAttribute -- same engine and same consumer here, see
> out_ghidra.c:150281: the offsets that Ghidra decompiles for the local stat
> match bionic). Passing the struct stat of newlib leaves those offsets
> with stack garbage: in Zenonia 2, when loading a saved game, the
> "size" read was a heap pointer (MALLOC FAILED FOR SIZE 0x81340CE0)
> and the engine crashed -- ported fix BEFORE the symptom appears here
> (Zenonia2 port_progress.md §12.4).

---

## `glOrthof_wrapper` (line ~603)

**Source File:** `loader/dynlib.c`

> Deliberate pass-through: the motor sends a CENTERED projection
> (-400..400, -240..240) and your full screen quad vertices use that
> same system (GL_FIXED -400..400/-240..240, confirmed in the logs).
> Force here a system with origin at the corner (0..800, 480..0) displaces
> that quad to a single quadrant of the screen and invert Y — real regression
> console view (small white box on black, log_1783657108).

---

## `__cxa_begin_cleanup` (line ~622)

**Source File:** `loader/dynlib.c`

> Convert from fixed point (Q16.16) to float

---

## `__cxa_atexit` (line ~639)

**Source File:** `loader/dynlib.c`

> Registering C++ static destructors (normally called
> dlclose()/exit() from a real dynamic library). This loader never
> download the .so or call exit() in an orderly manner -- the process ends
> with sceKernelExitProcess() -- so no need to run the
> registered destructors: suffice with no-ops that return success. were missing
> in the import table despite being confirmed in Phase 1 of the plan
> (objdump -T | grep UND), causing "Unknown symbol" and actual crash on
> console (see port_progress.md).

---

## `pthread_mutex_lock_wrapper` (line ~655)

**Source File:** `loader/dynlib.c`

> pthread_mutex_t/pthread_cond_t in VitaSDK are actually POINERS
> (typedef struct pthread_mutex_t_ * pthread_mutex_t;) to a structure
> internal that allocates pthread_mutex_init(). The .so is compiled against
> Bionic (Android), where a static mutex/condvar declared with
> PTHREAD_MUTEX_INITIALIZER/PTHREAD_COND_INITIALIZER remains at ZEROS without
> never call pthread_mutex_init at runtime -- Bionic is
> designed to treat those zeros as "valid, unlocked mutex"
> native way. In VitaSDK those same zeros are a real NULL pointer, and
> PTE pthread_mutex_lock/unlock (pthreads-embedded) dereference it
> unchecked, crashing (confirmed in real console: Data abort inside
> from pthread_mutex_unlock, called from the internal node-allocator
> libstdc++ on a never initialized static mutex -- see
> port_progress.md). NULL pointer is detected and initialized on-demand
> before using it, instead of passing the raw value to the actual implementation.

---

## `new_path` (line ~733)

**Source File:** `loader/dynlib.c`

> strncpy does not guarantee terminator if in_path >= out_size (256, see
> fopen_hook/stat_hook/access_hook) -- snprintf truncate and always
> ends in '\0', avoiding reading stack garbage as path.

---

## `struct` (line ~748)

**Source File:** `loader/dynlib.c`

> struct stat with the bionic layout (Android ARM 32-bit, NDK android-9) --
> It is NOT the one in newlib/vitasdk. The engine directly reads st_mode at the offset
> 16 and st_size at 48 (confirmed in Zenonia 2 by disassembling
> MC_fsFileAttribute -- same engine and same consumer here, see
> out_ghidra.c:150281: the offsets that Ghidra decompiles for the local stat
> match bionic). Passing the struct stat of newlib leaves those offsets
> with stack garbage: in Zenonia 2, when loading a saved game, the
> "size" read was a heap pointer (MALLOC FAILED FOR SIZE 0x81340CE0)
> and the engine crashed -- ported fix BEFORE the symptom appears here
> (Zenonia2 port_progress.md §12.4).

---

## `dynlib.c` (line ~890) (line ~890)

**Source File:** `loader/dynlib.c`

> Import resolution table. Confirmed 1:1 against
> `objdump -T libgameDSO.so | grep UND` (92 symbols, see Phase 1 of the plan):
> this engine does not import __android_log_print in this build (unlike
> Zenonia 2), so it doesn't register -- if it appears in a future build
> it is enough to add the entry, the function already exists in main.c.

---

## `dynlib.c` (line ~904) (line ~904)

**Source File:** `loader/dynlib.c`

> Import resolution table. Confirmed 1:1 against
> `objdump -T libgameDSO.so | grep UND` (92 symbols, see Phase 1 of the plan):
> this engine does not import __android_log_print in this build (unlike
> Zenonia 2), so it doesn't register -- if it appears in a future build
> it is enough to add the entry, the function already exists in main.c.

---