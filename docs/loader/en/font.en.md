# `loader/font.h` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `gfa_font_init` (line ~6)

**Source File:** `loader/font.h`

> Font raster backend for GFA bridge (NexusFont.java) --
> replace android.graphics.Paint/Canvas/Bitmap with stb_truetype above
> app0:font.ttf (NanumGothic, OFL: full Hangul + Latin coverage, the
> game uses Korean strings via GFA_SetString/SetStringFromKSC5601).
>
> Conventions (derived from NexusFont.java, which is the true font):
> - "px" is Paint.setTextSize (EM size in pixels).
> - ascent/descent are POSITIVE and with ceil (GFA_GetAscent = -ceil(ascent()),
> GFA_GetDescent = ceil(descent())).
> - The pixel buffer is 32-bit ARGB; the motor consumes ONLY the channel
> alpha (byte >>24) for glyph cache (CopyPixelsToCharCacheBuffer,
> out_ghidra.c:153389) with stride = width of the GFA_Init bitmap.

---

## `gfa_font_advance` (line ~17)

**Source File:** `loader/font.h`

> Font raster backend for GFA bridge (NexusFont.java) --
> replace android.graphics.Paint/Canvas/Bitmap with stb_truetype above
> app0:font.ttf (NanumGothic, OFL: full Hangul + Latin coverage, the
> game uses Korean strings via GFA_SetString/SetStringFromKSC5601).
>
> Conventions (derived from NexusFont.java, which is the true font):
> - "px" is Paint.setTextSize (EM size in pixels).
> - ascent/descent are POSITIVE and with ceil (GFA_GetAscent = -ceil(ascent()),
> GFA_GetDescent = ceil(descent())).
> - The pixel buffer is 32-bit ARGB; the motor consumes ONLY the channel
> alpha (byte >>24) for glyph cache (CopyPixelsToCharCacheBuffer,
> out_ghidra.c:153389) with stride = width of the GFA_Init bitmap.

---

## `gfa_font_break_text` (line ~24)

**Source File:** `loader/font.h`

> Font raster backend for GFA bridge (NexusFont.java) --
> replace android.graphics.Paint/Canvas/Bitmap with stb_truetype above
> app0:font.ttf (NanumGothic, OFL: full Hangul + Latin coverage, the
> game uses Korean strings via GFA_SetString/SetStringFromKSC5601).
>
> Conventions (derived from NexusFont.java, which is the true font):
> - "px" is Paint.setTextSize (EM size in pixels).
> - ascent/descent are POSITIVE and with ceil (GFA_GetAscent = -ceil(ascent()),
> GFA_GetDescent = ceil(descent())).
> - The pixel buffer is 32-bit ARGB; the motor consumes ONLY the channel
> alpha (byte >>24) for glyph cache (CopyPixelsToCharCacheBuffer,
> out_ghidra.c:153389) with stride = width of the GFA_Init bitmap.

---