# `CMakeLists.txt` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `CMakeLists.txt` (line ~3) (línea ~3)

**Source File:** `CMakeLists.txt`

> Esto incluye el toolchain de Vita, debe ir antes de project()

---

## `CMakeLists.txt` (line ~18) (línea ~18)

**Source File:** `CMakeLists.txt`

> ID unico de 9 caracteres. Distinto del de Zenonia 2 (PSVZ00002) para poder
> tener ambos juegos instalados a la vez.

---

## `CMakeLists.txt` (line ~23) (línea ~23)

**Source File:** `CMakeLists.txt`

> Log verbose de cada llamada JNI de FalsoJNI. Imprescindible para el
> primer arranque (Fase 3 del plan: metodologia de "un log a la vez"), pero
> debe apagarse antes de medir rendimiento real -- cada linea implica un
> sceIoOpen+sceIoWrite+sceIoClose (ver Fixes_Log de Prince of Persia
> #12-13, y plan_zenonia3_port.md seccion de riesgos heredados).
> Apagado desde que el juego llega al menu: con el nivel 0, el 79% del log
> eran lineas [JNI][...] (3579 de 4520 en log_1783657108) y el arranque
> tardaba minutos por el fflush por linea. Con nivel 2 (WARN) siguen
> visibles [JNI WARN]/[JNI ERR], que son los que diagnostican bugs reales.
> Reactivar (=0) solo si hace falta rastrear una llamada JNI puntual.

---

## `CMakeLists.txt` (line ~39) (línea ~39)

**Source File:** `CMakeLists.txt`

> Oculta la cruceta nativa + sus hasta 5 botones de accion
> (GVUIPlayerController, la unica instancia de esa clase en todo el juego --
> ver decompiled_so/out_ghidra.c:28609/28466 y la nota completa en
> loader/dynlib.c junto a zenonia_install_hide_dpad_hook()). Se hookea su
> constructor y se pone en 0 el contador interno de "objetos activos" que
> GVUIController::Draw()/PointerPress()/Move()/Release() usan como limite de
> su loop -- no dibuja NI recibe touch, sin tocar ningun otro controller
> (barra de vida/mana, minimapa, etc., cada uno con su propia instancia). La
> Vita ya tiene D-Pad/botones fisicos mapeados 1:1 (ver btn_map en
> loader/main.c), asi que no se pierde ninguna funcionalidad real.
> Para reactivar la cruceta/botones en pantalla: pasar
> -DHIDE_VIRTUAL_GAMEPAD=OFF a cmake (o cambiar el default de abajo a OFF) y
> recompilar.

---

## `CMakeLists.txt` (line ~55) (línea ~55)

**Source File:** `CMakeLists.txt`

> Prueba A/B de rendimiento (ver CLAUDE.md seccion de optimizaciones).
> RGB565_CONVERT_MODE elige UNA de 4 estrategias mutuamente excluyentes para
> el framebuffer de software 400x240 que el motor sube por textura RGB565
> varias veces por frame (loader/dynlib.c, convert_rgb565_to_rgba8888 +
> glTexImage2D_wrapper/glTexSubImage2D_wrapper):
>
> - SCALAR: division entera por pixel (baseline original).
> - LUT: tabla precomputada -- mismo resultado exacto que SCALAR, sin
>   dividir por pixel.
> - NEON: intrinsics NEON (vld1q_u16/vst4_u8) con expansion de bits por
>   replicacion -- mismo enfoque que un sampler de hardware R5G6B5,
>   difiere de SCALAR/LUT por hasta 1 LSB (imperceptible).
> - NATIVE: NO convierte nada -- sube el RGB565 tal cual y deja que el
>   sampler de GXM lo lea nativo. SIN CONFIRMAR EN HARDWARE (ver
>   comentario junto a glTexImage2D_wrapper); si el log muestra
>   textura corrupta/canal de color desplazado, volver a SCALAR/LUT.
>   Default NATIVE + OPTIMIZE_NEON_FIXED=OFF abajo: confirmado en consola real
>   como la combinacion mas estable (ver README, seccion "Performance Tuning"),
>   junto con LOCK_FPS_30=ON de mas abajo -- esta es la config que build.sh
>   produce como build/zenonia_3.vpk (version final).

---

## `CMakeLists.txt` (line ~91) (línea ~91)

**Source File:** `CMakeLists.txt`

> Intrinsics NEON (vld1q_s32/vcvtq_f32_s32/vmulq_f32) en vez del loop escalar
> para la conversion GL_FIXED (Q16.16) -> float de vertex/color/texcoord
> arrays en glDrawArrays_wrapper -- corre por cada vertice de cada draw
> call, cada frame. Resultado bit-a-bit identico al escalar (ver comentario
> junto a fixed_to_float_neon en dynlib.c) -- eje de A/B independiente del
> de RGB565_CONVERT_MODE de arriba.

---

## `CMakeLists.txt` (line ~102) (línea ~102)

**Source File:** `CMakeLists.txt`

> Con RGB565_CONVERT_MODE=NATIVE el frame renderiza tan rapido que cae justo
> en el borde del vblank del panel (60Hz) -- el vsync interno de vitaGL (que
> solo espera 1 vblank) da un framerate que rebota entre ~40 y ~60 en vez de
> estabilizarse. Esta opcion apaga ese vsync (vglWaitVblankStart(GL_FALSE)
> en gl_init()) y fuerza manualmente 2 vblanks por frame
> (sceDisplayWaitVblankStartMulti(2) en el loop principal, main.c) -- cap
> duro a 30fps estable y sin tearing. Inocuo con los otros modos de
> RGB565_CONVERT_MODE (20-24fps ya medidos): si el frame ya tardo mas de 2
> vblanks, la espera manual no hace nada, no empeora el framerate real.

---

## `CMakeLists.txt` (line ~118) (línea ~118)

**Source File:** `CMakeLists.txt`

> ATTRIBUTE2=12 pide el presupuesto de memoria extendido -- sin esto,
> SceShaccCg (compilador de shaders Cg de vitaGL) puede fallar al iniciar.
> Copiado de la config confirmada funcionando en Zenonia 2 / Prince of Persia.

---

## `CMakeLists.txt` (line ~153) (línea ~153)

**Source File:** `CMakeLists.txt`

> Decodificador OGG Vorbis (Tremor, punto fijo) para loader/audio.c

---

## `CMakeLists.txt` (line ~153) (línea ~153)

**Source File:** `CMakeLists.txt`

> Fuente TTF para el rasterizado GFA (loader/font.c): NanumGothic, licencia
> OFL (cobertura Hangul completa + Latin -- el juego setea strings coreanos)
> Overlays Android (logo/titulo/menu) que el .so nativo no dibuja -- ver
> loader/androidui.c -- y el texto de ABOUT/HELP (loader/htmltext.c/
> htmlview.c) usan los PNG/HTML reales del APK original (variantes
> "globales"/en ingles). Esos archivos tienen copyright de Gamevil y YA NO
> se empaquetan aca: se leen en runtime desde
> ux0:data/zenonia3/drawable/ y ux0:data/zenonia3/html/ (memoria externa,
> subidos por FTP con manage_vita.py) en vez de instalarse dentro del VPK
> bajo ux0:app/${VITA_TITLEID}/ -- mantiene el VPK distribuible sin
> material con copyright y evita reinstalar el paquete entero cada vez que
> cambia un PNG. androidui.c/htmlview.c igual mantienen un fallback a
> app0:drawable/ / app0:html/ (adentro del VPK) por si algun build local
> decide empaquetarlos de nuevo a mano.

---
