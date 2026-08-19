# `loader/main.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `main.c` (line ~1) (line ~1)

**Source File:** `loader/main.c`

> main.c
>
> ARMv7 Shared Libraries loader. Zenonia 3.
>
> Same engine (Gamevil Nexus2/"Clet") as Zenonia 2 -- see
> ../Zenonia2-vita/loader/main.c and plan_zenonia3_port.md section 0 for the
> detail that it is identical and that the real ABI changed between both games.

---

## `GAME_W` (line ~30)

**Source File:** `loader/main.c`

> LOGICAL resolution of the game: 400x240 fixed. Confirmed by triple source
> (2026-07-10): Original Java passes gameScreenWidth/Height = 400/240 to
> NativeInitDeviceInfo/NativeInitWithBufferSize(NexusGLActivity.java:85-86,
> jadx renamed the constant 400 as UI_STATUS_PURCHASE_PAGE), getDeviceInfo()
> defaultea di[3]=400 di[4]=0xf0 (out_ghidra.c:148015), and the drawing code
> of the title is hardcoded to that space (CMvTitleState::DrawZeroGrade
> does DrawFillRect(0,0,400,240) and centers with (400-w)>>1 -- out_ghidra.c:105397).
> Initializing at 800x480 caused the game to paint only the top quadrant
> left of the framebuffer (small sprites + the rest white/black, seen in
> real console). Full screen scaling is done by the ENGINE: glResize()
> assemble the quad with vertices +-w/2 of the size that we pass to NativeResize
> (out_ghidra.c:148107+), so NativeResize receives SCREEN_W/H (960x544).

---

## `gl_active` (line ~41)

**Source File:** `loader/main.c`

> LOGICAL resolution of the game: 400x240 fixed. Confirmed by triple source
> (2026-07-10): Original Java passes gameScreenWidth/Height = 400/240 to
> NativeInitDeviceInfo/NativeInitWithBufferSize(NexusGLActivity.java:85-86,
> jadx renamed the constant 400 as UI_STATUS_PURCHASE_PAGE), getDeviceInfo()
> defaultea di[3]=400 di[4]=0xf0 (out_ghidra.c:148015), and the drawing code
> of the title is hardcoded to that space (CMvTitleState::DrawZeroGrade
> does DrawFillRect(0,0,400,240) and centers with (400-w)>>1 -- out_ghidra.c:105397).
> Initializing at 800x480 caused the game to paint only the top quadrant
> left of the framebuffer (small sprites + the rest white/black, seen in
> real console). Full screen scaling is done by the ENGINE: glResize()
> assemble the quad with vertices +-w/2 of the size that we pass to NativeResize
> (out_ghidra.c:148107+), so NativeResize receives SCREEN_W/H (960x544).

---

## `init_log` (line ~50)

**Source File:** `loader/main.c`

> One log file per run, with timestamp, inside logs/ -- maintains
> the complete history between tests instead of always stepping on the same one
> log.txt (see psvita-porting skill, hardware_debugging.md).

---

## `main.c` (line ~91) (line ~91)

**Source File:** `loader/main.c`

> One log file per run, with timestamp, inside logs/ -- maintains
> the complete history between tests instead of always stepping on the same one
> log.txt (see psvita-porting skill, hardware_debugging.md).

---

## `__android_log_print` (line ~103)

**Source File:** `loader/main.c`

> Not confirmed that the Zenonia 3 .so imports it (see Phase 1 of the plan: no
> appears in `objdump -T | grep UND`), but it is left available in case anyone
> future build needs it -- it doesn't hurt to have it unregistered in
> dynlib.c.

---

## `main.c` (line ~122) (line ~122)

**Source File:** `loader/main.c`

> Not confirmed that the Zenonia 3 .so imports it (see Phase 1 of the plan: no
> appears in `objdump -T | grep UND`), but it is left available in case anyone
> future build needs it -- it doesn't hurt to have it unregistered in
> dynlib.c.

---

## `MH_KEY_PRESSEVENT` (line ~134)

**Source File:** `loader/main.c`

> --- Input: protocol pending reconfirmation against the real Java of
> Zenonia 3 (Phase 5 of the plan -- decompile ZenoniaUIControllerView and the
> NexusHal equivalent, don't assume it is equal to Zenonia2UIControllerView).
> It starts with the same MH_* codes and HAL keycodes of Zenonia 2 because
> they are constants from the ENGINE (Nexus2/Clet), not from the game layer -- but
> they must be confirmed with a real log before accepting this as good.
>
> Actual ABI difference: there is no setInputEvent in this .so. There is only
> handleCletEvent, which now takes 4 ints (was 3) -- see Phase 1 of the plan and
> out_ghidra.c:147900. An event is still queued per frame and delivered
> just before NativeRender (same order as NexusGLRenderer.drawFrame in
> Zenonia 2), without the immediate delivery that setInputEvent did, because here
> that channel does not exist.

---

## `NEXUS_HAL_REPLY_YESNO` (line ~155)

**Source File:** `loader/main.c`

> Actual NexusHal.java constants -- the "Continue" menu button does not
> sends a key press/release like the others, sends a SINGLE event with
> this type/parameter (see Natives.java: img_menu_continue.setOnClickListener
> -> handleCletEvent(NexusHal.REPLY_YESNO, NexusHal.FIRST_MOVE_REPLY_PAGE, 0, 0)).

---

## `NEXUS_HAL_YES_MOVE_REPLY_PAGE` (line ~161)

**Source File:** `loader/main.c`

> The "write review"/"later" buttons on the review screen itself
> REPLY_PAGE (see Natives.java: showReplyMoveComponent(), img_btn_write/
> img_btn_later OnClickListener) send the same type REPLY_YESNO with these
> two other parameters.

---

## `zenonia_install_array_hooks` (line ~203)

**Source File:** `loader/main.c`

> NOTE: unlike Zenonia 2, no binary patch is carried here
> (apply_so_patches). The Zenonia2 bug (heap pointer treated as
> signed in CMvLayerData::PreLoad) is an old engine pattern that can
> reappear in this .so, but at a DIFFERENT offset -- do not reuse the number
> 0xaec38 blindly. If the same symptom appears (NULL "impossible" in data
> which should have been loaded), re-derive the offset with vita-parse-core +
> objdump -d on THIS binary (see Phase 7 of the plan).

---

## `main.c` (line ~274) (line ~274)

**Source File:** `loader/main.c`

> vitaGL's internal vsync only expects 1 vblank (panel at 60Hz) -- with
> RGB565_CONVERT_MODE=NATIVE the frame sometimes enters that vblank and sometimes
> sometimes not, and the result is a framerate that bounces between ~60 and ~40
> (the typical jitter/stutter of being right on the edge of the vblank). If
> turns off the vitaGL vsync and takes manual control of the pacing in the
> main loop (sceDisplayWaitVblankStartMulti(2)) to force
> always 2 vblanks per frame -- stable 30fps without tearing instead of
> "up to 60 but irregular."

---

## `main.c` (line ~279) (line ~279)

**Source File:** `loader/main.c`

> vitaGL's internal vsync only expects 1 vblank (panel at 60Hz) -- with
> RGB565_CONVERT_MODE=NATIVE the frame sometimes enters that vblank and sometimes
> sometimes not, and the result is a framerate that bounces between ~60 and ~40
> (the typical jitter/stutter of being right on the edge of the vblank). If
> turns off the vitaGL vsync and takes manual control of the pacing in the
> main loop (sceDisplayWaitVblankStartMulti(2)) to force
> always 2 vblanks per frame -- stable 30fps without tearing instead of
> "up to 60 but irregular."

---

## `raise_clocks` (line ~285)

**Source File:** `loader/main.c`

> Conservative factory clocks (CPU 333MHz / bus 166MHz / GPU 111MHz) --
> going up to the stable maximums known in Vita homebrew is standard
> practice (same approach used in most vitaGL ports) and does not touch
> no game logic, just the actual hardware frequency.

---

## `raise_clocks` (line ~294)

**Source File:** `loader/main.c`

> Conservative factory clocks (CPU 333MHz / bus 166MHz / GPU 111MHz) --
> going up to the stable maximums known in Vita homebrew is standard
> practice (same approach used in most vitaGL ports) and does not touch
> no game logic, just the actual hardware frequency.

---

## `text_base` (line ~324)

**Source File:** `loader/main.c`

> Conservative factory clocks (CPU 333MHz / bus 166MHz / GPU 111MHz) --
> going up to the stable maximums known in Vita homebrew is standard
> practice (same approach used in most vitaGL ports) and does not touch
> no game logic, just the actual hardware frequency.

---

## `main.c` (line ~361) (line ~361)

**Source File:** `loader/main.c`

> Boot sequence -- replaces the single NativeInit()
> Zenonia 2 (which does not exist here).
>
> ORDER CONFIRMED WITH A REAL CRASH (see port_progress.md Phase 3,
> bug #2): NativeInitWithBufferSize MUST be called BEFORE
> NativeInitDeviceInfo. NativeInitWithBufferSize fires
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), which creates the
> engine's OWN memory pool (a custom slab type allocator,
> separate from malloc/new) on which Gcx_MM_Alloc/Calloc depends.
> NativeInitDeviceInfo uses that same allocator for the buffer
> internal pixels (via getDeviceInfo() -> Gcx_MM_Calloc) -- if
> calls first, pool does not exist yet, Gcx_MM_Alloc returns
> NULL, and the subsequent memset(NULL,0,n) does Data Abort (seen
> in real console, solved with vita-parse-core + objdump -d against
> libgameDSO.so: LR falls at Gcx_MM_Calloc+0x13, just after the
> blx to memset@plt).

---

## `pad` (line ~371)

**Source File:** `loader/main.c`

> Boot sequence -- replaces the single NativeInit()
> Zenonia 2 (which does not exist here).
>
> ORDER CONFIRMED WITH A REAL CRASH (see port_progress.md Phase 3,
> bug #2): NativeInitWithBufferSize MUST be called BEFORE
> NativeInitDeviceInfo. NativeInitWithBufferSize fires
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), which creates the
> engine's OWN memory pool (a custom slab type allocator,
> separate from malloc/new) on which Gcx_MM_Alloc/Calloc depends.
> NativeInitDeviceInfo uses that same allocator for the buffer
> internal pixels (via getDeviceInfo() -> Gcx_MM_Calloc) -- if
> calls first, pool does not exist yet, Gcx_MM_Alloc returns
> NULL, and the subsequent memset(NULL,0,n) does Data Abort (seen
> in real console, solved with vita-parse-core + objdump -d against
> libgameDSO.so: LR falls at Gcx_MM_Calloc+0x13, just after the
> blx to memset@plt).

---

## `fps_count` (line ~392)

**Source File:** `loader/main.c`

> Boot sequence -- replaces the single NativeInit()
> Zenonia 2 (which does not exist here).
>
> ORDER CONFIRMED WITH A REAL CRASH (see port_progress.md Phase 3,
> bug #2): NativeInitWithBufferSize MUST be called BEFORE
> NativeInitDeviceInfo. NativeInitWithBufferSize fires
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), which creates the
> engine's OWN memory pool (a custom slab type allocator,
> separate from malloc/new) on which Gcx_MM_Alloc/Calloc depends.
> NativeInitDeviceInfo uses that same allocator for the buffer
> internal pixels (via getDeviceInfo() -> Gcx_MM_Calloc) -- if
> calls first, pool does not exist yet, Gcx_MM_Alloc returns
> NULL, and the subsequent memset(NULL,0,n) does Data Abort (seen
> in real console, solved with vita-parse-core + objdump -d against
> libgameDSO.so: LR falls at Gcx_MM_Calloc+0x13, just after the
> blx to memset@plt).

---

## `pressed` (line ~407)

**Source File:** `loader/main.c`

> Boot sequence -- replaces the single NativeInit()
> Zenonia 2 (which does not exist here).
>
> ORDER CONFIRMED WITH A REAL CRASH (see port_progress.md Phase 3,
> bug #2): NativeInitWithBufferSize MUST be called BEFORE
> NativeInitDeviceInfo. NativeInitWithBufferSize fires
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), which creates the
> engine's OWN memory pool (a custom slab type allocator,
> separate from malloc/new) on which Gcx_MM_Alloc/Calloc depends.
> NativeInitDeviceInfo uses that same allocator for the buffer
> internal pixels (via getDeviceInfo() -> Gcx_MM_Calloc) -- if
> calls first, pool does not exist yet, Gcx_MM_Alloc returns
> NULL, and the subsequent memset(NULL,0,n) does Data Abort (seen
> in real console, solved with vita-parse-core + objdump -d against
> libgameDSO.so: LR falls at Gcx_MM_Calloc+0x13, just after the
> blx to memset@plt).

---

## `pressed` (line ~413)

**Source File:** `loader/main.c`

> The SCREEN size, not the buffer size -- just like Android, where
> NativeResize receives the size of the actual GLSurfaceView and the engine
> assemble only the scaling quad (see note in GAME_W/GAME_H).

---

## `main.c` (line ~422) (line ~422)

**Source File:** `loader/main.c`

> The SCREEN size, not the buffer size -- just like Android, where
> NativeResize receives the size of the actual GLSurfaceView and the engine
> assemble only the scaling quad (see note in GAME_W/GAME_H).

---

## `x` (line ~441)

**Source File:** `loader/main.c`

> The SCREEN size, not the buffer size -- just like Android, where
> NativeResize receives the size of the actual GLSurfaceView and the engine
> assemble only the scaling quad (see note in GAME_W/GAME_H).

---

## `sx` (line ~450)

**Source File:** `loader/main.c`

> Real FPS counter (to compare A/B builds of the
> RGB565_LUT/NEON_FIXED optimizations against the same point of
> reference in the log, instead of "by eye"). ~2s window instead of
> per-frame to not add log overhead to the hot loop.

---

## `main.c` (line ~458) (line ~458)

**Source File:** `loader/main.c`

> Real FPS counter (to compare A/B builds of the
> RGB565_LUT/NEON_FIXED optimizations against the same point of
> reference in the log, instead of "by eye"). ~2s window instead of
> per-frame to not add log overhead to the hot loop.

---

## `main.c` (line ~502) (line ~502)

**Source File:** `loader/main.c`

> Natives.showTitleComponent(): gb_titleImg (background,
> full screen) commands BACK press+release before
> ANY touch.

---

## `main.c` (line ~515) (line ~515)

**Source File:** `loader/main.c`

> Natives.showTitleComponent(): gb_titleImg (background,
> full screen) commands BACK press+release before
> ANY touch.

---

## `main.c` (line ~564) (line ~564)

**Source File:** `loader/main.c`

> While the engine is in logo/title (Java UI states,
> invisible in the native loader -- pending confirmation
> real status numbers of Zenonia 3 in Phase 5), cover with
> the splash.

---

## `main.c` (line ~570) (line ~570)

**Source File:** `loader/main.c`

> While the engine is in logo/title (Java UI states,
> invisible in the native loader -- pending confirmation
> real status numbers of Zenonia 3 in Phase 5), cover with
> the splash.

---

## `now` (line ~577)

**Source File:** `loader/main.c`

> While the engine is in logo/title (Java UI states,
> invisible in the native loader -- pending confirmation
> real status numbers of Zenonia 3 in Phase 5), cover with
> the splash.

---