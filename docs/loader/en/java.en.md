# `loader/java.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `java.c` (line ~1) (line ~1)

**Source File:** `loader/java.c`

> java.c
>
> "Java-side" native method handlers that libgameDSO.so (Zenonia 3, same
> Gamevil Nexus2/Clet engine Zenonia 2) calls back via FalsoJNI
> (GetStaticMethodID + CallStaticObjectMethod/CallStaticIntMethod/etc).
> Everything that is not registered here simply remains "not found" for
> FalseJNI (logged, not fatal for void/Object methods -- see
> plan_zenonia3_port.md "Risks" section and port_progress.md of Zenonia2
> §9.10 for the opposite case: int/boolean SI methods are dangerous if
> are not registered).

---

## `zenonia_resolve_asset_path` (line ~21)

**Source File:** `loader/java.c`

> readAssets/isAssetExist are always called with the same relative path (the
> engine calls isAssetExist(path) before deciding if it's worth calling
> readAssets(path)), so both should resolve the same. Same as in
> Zenonia 2: try the bare path first (ux0:data/zenonia3/<name>) and if
> does not exist, the one prefixed with assets/ (which the dynlib.c hooks use to
> everything else) -- without assuming what the real convention is until confirming it
> with a console log.

---

## `ZENONIA_DALVIK_REGISTRY_MAX` (line ~36)

**Source File:** `loader/java.c`

> --- Bridge so that readAssets serves the TWO different consumers that
> has the engine ---
>
> "Dalvik's ArrayObject" trick (16 byte header + raw data,
>see Zenonia_readAssets) works for direct consumer by pointer
> (MC_knlGetResource and the rest of CMvResourceMgr) because that code never
> goes through the standard JNI array functions -- reads offset+16
> directly. But a DIFFERENT consumer (confirmed real: the family of
> parsers "PZx"/"PZD" -- CGxPZDParser/CGxZeroPZDParser -- used for
> TouchOemIME.pzx and other .pzx, see port_progress.md Phase 3.7) IF calls
> Standard GetArrayLength/GetByteArrayElements on the same result
> readAssets. Since our block is not a real JavaDynArray (I don't go through
> jda_alloc), FalseJNI does not find it ("Could not find the array") and those
> parsers receive NULL/length 0 -- causing a downstream crash (pointer
> NULL assumed valid after a half-parser "success".
>
> Solution: intercept GetArrayLength/GetByteArrayElements/
> GetByteArrayRegion/ReleaseByteArrayElements from JNI function table
> (mutable in memory despite the `const` of the public type -- jni_init() the
> allocates with malloc()) to recognize our own blocks (carrying a
> register pointers returned by Zenonia_readAssets) and serve them
> directly from the Dalvik header, dropping to the real FalsoJNI code
> for any other array (the genuine JavaDynArray from jda_alloc, used
> for example in the GFA_* functions of Phase 3.5).

---

## `zenonia_install_array_hooks` (line ~98)

**Source File:** `loader/java.c`

> Call AFTER jni_init() (main.c) -- needs the table
> functions are already crazy.

---

## `java.c` (line ~120) (line ~120)

**Source File:** `loader/java.c`

> Real engine type (inherited from Zenonia 2, confirmed in Zenonia 3
> with `strings libgameDSO.so`: both strings "readAssets" and
> "readAssete" in the binary). Same handler for both.

---

## `java.c` (line ~133) (line ~133)

**Source File:** `loader/java.c`

> Real engine type (inherited from Zenonia 2, confirmed in Zenonia 3
> with `strings libgameDSO.so`: both strings "readAssets" and
> "readAssete" in the binary). Same handler for both.

---

## `java.c` (line ~140) (line ~140)

**Source File:** `loader/java.c`

> Real engine type (inherited from Zenonia 2, confirmed in Zenonia 3
> with `strings libgameDSO.so`: both strings "readAssets" and
> "readAssete" in the binary). Same handler for both.

---

## `java.c` (line ~147) (line ~147)

**Source File:** `loader/java.c`

> Real engine type (inherited from Zenonia 2, confirmed in Zenonia 3
> with `strings libgameDSO.so`: both strings "readAssets" and
> "readAssete" in the binary). Same handler for both.

---

## `java.c` (line ~174) (line ~174)

**Source File:** `loader/java.c`

> Safe no-ops (void): prevent "not found" spam in the log.

---

## `java.c` (line ~184) (line ~184)

**Source File:** `loader/java.c`

> New in the Zenonia 3 UIListener (they did not exist in Zenonia 2):
> OnVibrate(int) and OnEvent(int). Both voids would already be safe without
> register (see note above), but they register so as not to spam
> the log and to have the entry point ready when the Phase arrives
> audio/vibration.

---

## `java.c` (line ~193) (line ~193)

**Source File:** `loader/java.c`

> They return float[]/int[]/short[] which the engine dereferences WITHOUT
> check for NULL -- confirmed with a real crash (Data abort inside
> GFA_DrawFont) for DrawFont; DrawText/MeasureText share the same
> native wrapper structure (see port_progress.md Phase 3.5). object,
> They should NEVER return NULL in the normal path.

---

## `name` (line ~200)

**Source File:** `loader/java.c`

> They return float[]/int[]/short[] which the engine dereferences WITHOUT
> check for NULL -- confirmed with a real crash (Data abort inside
> GFA_DrawFont) for DrawFont; DrawText/MeasureText share the same
> native wrapper structure (see port_progress.md Phase 3.5). object,
> They should NEVER return NULL in the normal path.

---

## `name` (line ~206)

**Source File:** `loader/java.c`

> They return float[]/int[]/short[] which the engine dereferences WITHOUT
> check for NULL -- confirmed with a real crash (Data abort inside
> GFA_DrawFont) for DrawFont; DrawText/MeasureText share the same
> native wrapper structure (see port_progress.md Phase 3.5). object,
> They should NEVER return NULL in the normal path.

---

## `struct` (line ~230)

**Source File:** `loader/java.c`

> Logged because a not found methodID causes methodIntCall() to
> FalseJNI returns -1 (see FalseJNI_ImplBridge.c) -- a non-zero value that
> the engine interprets as boolean C "true" (the file exists). That fake
> positive was the real cause of a crash on Zenonia 2 (§9.10 of its
> port_progress.md): the engine continued forward loading a file that in
>it had not really been resolved.

---

## `java.c` (line ~263) (line ~263)

**Source File:** `loader/java.c`

> fstat instead of fseek(SEEK_END)+ftell: in Zenonia 2, ftell() returned
> garbage (bytes of the path itself) for at least one real file and
> corrupted the rear malloc of the engine. fstat does not depend on
> cursor position and avoid that kind of root bug.

---

## `name` (line ~278)

**Source File:** `loader/java.c`

> fstat instead of fseek(SEEK_END)+ftell: in Zenonia 2, ftell() returned
> garbage (bytes of the path itself) for at least one real file and
> corrupted the rear malloc of the engine. fstat does not depend on
> cursor position and avoid that kind of root bug.

---

## `len` (line ~315)

**Source File:** `loader/java.c`

Dalvik ArrayObject expects the length as a 32-bit integer at offset 8

---

## `len` (line ~323)

**Source File:** `loader/java.c`

> Logged because a not found methodID causes methodIntCall() to
> FalseJNI returns -1 (see FalseJNI_ImplBridge.c) -- a non-zero value that
> the engine interprets as boolean C "true" (the file exists). That fake
> positive was the real cause of a crash on Zenonia 2 (§9.10 of its
> port_progress.md): the engine continued forward loading a file that in
>it had not really been resolved.

---

## `Zenonia_OnSoundPlay` (line ~362)

**Source File:** `loader/java.c`

> Actual signature: OnSoundPlay(int sndID, int vol, boolean isLoop) -- the second
> parameter is VOLUME (0-100, typical 50/75), the third is the loop. The logs
> old people labeled them backwards (same correction as Zenonia 2 §12.1).

---

## `GFA_MAX_FONTS` (line ~394)

**Source File:** `loader/java.c`

> --- GFA bridge (fonts) -- NexusFont.java replica with REAL rasterization
> via loader/font.c (stb_truetype + app0:font.ttf). The source of truth of
> each semantic is NexusFont.java (jadx); the pixel format it consumes
> the engine is committed in out_ghidra.c (CopyPixelsToCharCacheBuffer uses
> ONLY the alpha channel, stride = GFA_Init bitmap width).

---

## `g_gfa_str` (line ~410)

**Source File:** `loader/java.c`

> --- GFA bridge (fonts) -- NexusFont.java replica with REAL rasterization
> via loader/font.c (stb_truetype + app0:font.ttf). The source of truth of
> each semantic is NexusFont.java (jadx); the pixel format it consumes
> the engine is committed in out_ghidra.c (CopyPixelsToCharCacheBuffer uses
> ONLY the alpha channel, stride = GFA_Init bitmap width).

---

## `gfa_decode_utf8` (line ~416)

**Source File:** `loader/java.c`

> --- String decoders (NexusFont receives UTF-8 (jstring), UTF-16LE
> or EUC-KR/KSC5601 depending on the function) ---

---

## `gfa_decode_utf8` (line ~424)

**Source File:** `loader/java.c`

> --- String decoders (NexusFont receives UTF-8 (jstring), UTF-16LE
> or EUC-KR/KSC5601 depending on the function) ---

---

## `gfa_char_advance` (line ~485)

**Source File:** `loader/java.c`

> Metrics with fallback if the source did not load (same approximations as the
> stubs from Phase 3.4, so that the game does not lose the layout completely).

---

## `gfa_break_text` (line ~500)

**Source File:** `loader/java.c`

> Paint.breakText(text, true, maxWidth): characters from the start whose
> accumulated advance goes into maxWidth.

---

## `gfa_word_break_length` (line ~511)

**Source File:** `loader/java.c`

> Word BreakIterator, approximate: a boundary after each run
> of spaces, and each CJK/Hangul character is its own "word" (just like
> the actual BreakIterator for ideographs). Returns the number of characters
> until the last word boundary that goes into fit_chars (breakLength of the
> Java); 0 if no border enters.

---

## `count` (line ~539)

**Source File:** `loader/java.c`

> Word BreakIterator, approximate: a boundary after each run
> of spaces, and each CJK/Hangul character is its own "word" (just like
> the actual BreakIterator for ideographs). Returns the number of characters
> until the last word boundary that goes into fit_chars (breakLength of the
> Java); 0 if no border enters.

---

## `java.c` (line ~562) (line ~562)

**Source File:** `loader/java.c`

> Ensure persistent pixel buffers with the current pixel size
> GFA_Init (if the engine re-initializes with another size, they are released and reallocated
> -- jda_free leaves the table slot reusable without moving the rest).

---

## `Zenonia_GFA_SetTextSize` (line ~580)

**Source File:** `loader/java.c`

> JNI promotes float to double when passing it through a variadic va_list (rule of
> C language, independent of the target's floating point ABI) -- by
> that is read with va_arg(args, double) and cast to float, not the other way around.

---

## `style` (line ~589)

**Source File:** `loader/java.c`

> Ensure persistent pixel buffers with the current pixel size
> GFA_Init (if the engine re-initializes with another size, they are released and reallocated
> -- jda_free leaves the table slot reusable without moving the rest).

---

## `maxWidth` (line ~629)

**Source File:** `loader/java.c`

> Replicates slot reuse of NexusFont.GFA_CreateFont: up to
> GFA_MAX_FONTS different families, returns the same handle if the family already
> is registered, -1 if there are no free slots left (same as the original).

---

## `java.c` (line ~662) (line ~662)

**Source File:** `loader/java.c`

> (F[I)I -- maxWidth, wwPositions[]. Exact replica of the loop
> NexusFont.GFA_GetWordwrapPositionEx: as long as the rest does not enter
> maxWidth, breaks into breakText(maxWidth) characters, accumulates the position and
> writes it to wwPositions. Returns the number of cuts.

---

## `java.c` (line ~670) (line ~670)

**Source File:** `loader/java.c`

> (F[I)I -- maxWidth, wwPositions[]. Exact replica of the loop
> NexusFont.GFA_GetWordwrapPositionEx: as long as the rest does not enter
> maxWidth, breaks into breakText(maxWidth) characters, accumulates the position and
> writes it to wwPositions. Returns the number of cuts.

---

## `java.c` (line ~679) (line ~679)

**Source File:** `loader/java.c`

> Same as NexusFont.GFA_CharHeight(): literally the text size
> current, without special rounding.

---

## `Zenonia_GFA_SetString` (line ~706)

**Source File:** `loader/java.c`

> (Ljava/lang/String;I)V -- string (jstring = raw UTF-8 char* in FalseJNI),
> nChars (0 = full length). Java: g_strConv = string.substring(0, nChars).

---

## `Zenonia_GFA_SetStringFromKSC5601` (line ~719)

**Source File:** `loader/java.c`

> ([B)V -- the parameter arrives as a real jbyteArray (allocated by the engine
> via NewByteArray + SetByteArrayRegion, confirmed in log) -- in FalseJNI
> that's a JavaDynArray*, not a raw pointer (unlike jstring, which
> FalseJNI IF passed as raw char* -- do not confuse the two cases).
> Java: new String(data, "KSC5601") -- EUC-KR via generated table (cp949).

---

## `Zenonia_GFA_SetStringFromUnicode` (line ~735)

**Source File:** `loader/java.c`

> Java: new String(data, "UTF-16LE").

---

## `gfa_clear_canvas` (line ~759)

**Source File:** `loader/java.c`

> Clear the GFA canvas (equivalent to clearing g_gfaIntBuf in Java).

---

## `f` (line ~770)

**Source File:** `loader/java.c`

> Clear the GFA canvas (equivalent to clearing g_gfaIntBuf in Java).

---

## `x` (line ~801)

**Source File:** `loader/java.c`

> ()[F -- NexusFont.GFA_DrawFont(): clear the bitmap, draw the current string
> in (0, charH - descent + 1) and returns {0, 0, widthMeasured, charH + 1}.
> The engine then asks for GFA_GetPixels32 and uses ceil(rect[2]) x ceil(rect[3])
> pixels with ONLY the alpha channel (drawCharToCharCacheBuffer).

---

## `maxWidth` (line ~861)

**Source File:** `loader/java.c`

> (IF)[F -- nChars, maxWidth -> {maxwidth, totalheight}. Same loop as
> DrawText but only measuring (NexusFont.GFA_MeasureText).

---

## `java.c` (line ~908) (line ~908)

**Source File:** `loader/java.c`

> GFA_GetPixels32/16 YES they are safe by construction even if they return a
> empty array: the native code that calls them copy via
> GetArrayLength+GetIntArrayRegion/GetShortArrayRegion (does not dereference a
> raw pointer as GFA_DrawFont/DrawText/MeasureText) -- confirmed
> reading the disassembly of GFA_GetPixels32/16 in out_ghidra.c. If
> they register the same, with a real buffer (at zero, without rasterization yet) in
> instead of depending on whether that path tolerates a NULL/empty array without testing it.
> The pixels were already rasterized in the buffer for the last
> GFA_DrawFont/DrawText -- here only the array is returned (the engine copies with
> GetIntArrayRegion and use the alpha channel).

---

## `count` (line ~918)

**Source File:** `loader/java.c`

> GFA_GetPixels32/16 YES they are safe by construction even if they return a
> empty array: the native code that calls them copy via
> GetArrayLength+GetIntArrayRegion/GetShortArrayRegion (does not dereference a
> raw pointer as GFA_DrawFont/DrawText/MeasureText) -- confirmed
> reading the disassembly of GFA_GetPixels32/16 in out_ghidra.c. If
> they register the same, with a real buffer (at zero, without rasterization yet) in
> instead of depending on whether that path tolerates a NULL/empty array without testing it.
> The pixels were already rasterized in the buffer for the last
> GFA_DrawFont/DrawText -- here only the array is returned (the engine copies with
> GetIntArrayRegion and use the alpha channel).

---