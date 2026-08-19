# Zenonia 3 Port Progress (PS Vita)

This file serves as a log of the technical problems encountered and the solutions implemented during the port from Android to PS Vita using SoLoader.

## 1. Initial libc/pthread crashes
- **Symptom:** Data Abort on startup within `__cxa_guard_acquire`.
- **Cause:** The game on Android used `Bionic libc`, where `PTHREAD_MUTEX_INITIALIZER` assigns a static value (`0x4000` or `0x0`). VitaSDK treats this value directly as a pointer and fails when trying to dereference it in `pthread_mutex_lock`.
- **Bug Fix:** Implemented `pthread_mutex_lock_wrapper` and `pthread_mutex_unlock_wrapper` in `dynlib.c` to detect whether the mutex is `NULL` or `0x4000` and dynamically initialize it before locking it using `pthread_mutex_init`.

## 2. Game Scaling and Resolution (Small Chart)
- **Symptom:** The Gamevil logo and the game were rendered in a small box on the screen instead of covering the entire screen.
- **Cause:** The engine was trying to draw at 800x480 or other internal resolution, while Vita set its buffer to 960x544, causing the render to look unscaled.
- **Solution:** 
  1. In `main.c`, `NativeInitWithBufferSize` and `NativeResize` were configured to send the engine a resolution of `800x480`.
  2. In `dynlib.c`, `glViewport_wrapper` was intercepted to force the Vita's native full screen (`960x544`) to always be used.
  3. Intercepted `glOrthof_wrapper` so that when the engine requests its 800x480 space (`0, 800, 480, 0`), `vitaGL` will automatically hardware scale it to the forced viewport.

## 3. Data Abort when loading maps (Black Screen)
- **Symptom:** The game crashed with a Data Abort error in `CMvMapModule::DrawScroll` after pressing "Start".
- **Cause (Nexus Engine Bug):** In `CMvLayerData::PreLoad` (address `0x9a7b4`), the engine performs a `ble` check on a map pointer. On the PS Vita, addresses in RAM start at `0x81xxxxxx`, which the processor in `signed` mode interprets as negative. This causes a silent abort of the load, leaving the map array NULL, which causes `DrawScroll` to crash when it tries to read it.
- **Fix:** A dynamic memory patch was injected into `main.c` (via `kuKernelCpuUnrestrictedMemcpy`) at address `text_base + 0x9a7b4`. Replaced Thumb instruction `ble` (`0xdd24`) with `beq` (`0xd024`).

## 4. Textures and Interface in White (or Black Boxes)
- **Symptom:** The title and menu screens were rendered with white boxes (corrupt/incomplete textures) even though the game did not crash.
- **Cause:** The engine requests the color format `GL_UNSIGNED_SHORT_5_6_5` for large textures and, when initialized, the engine does not configure filters for mipmaps using `glTexParameteri`. Since there are no mipmaps or linear/close filters, OpenGL marks the textures as `Incomplete` (drawing white boxes).
- **Fix:** Created wrappers for `glTexImage2D` and `glTexSubImage2D` in `dynlib.c` that inject the `GL_TEXTURE_MIN_FILTER` and `GL_TEXTURE_MAG_FILTER` parameters (set to `GL_LINEAR`) transparently so that vitaGL renders everything normally.

---
**Current Status:**
- Stable VPK compilation and engine running in loop.
- Modifications awaiting final testing to confirm the correct rendering of the HUD and the main map of Zenonia 3.