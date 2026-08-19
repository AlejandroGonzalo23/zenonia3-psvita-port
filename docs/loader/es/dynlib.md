# `loader/dynlib.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `zenonia_verbose_ui` (line ~27)

**Source File:** `loader/dynlib.c`

> El logo/titulo/menu (ui_status 0-2, ver ZenoniaUIControllerView.java:
> UI_STATUS_LOGO/TITLE/MAINMENU) se ve blanco y despues negro (confirmado
> por el usuario), sin crash -- CMvTitleState::DrawZeroGrade/DrawTeamLogo
> (out_ghidra.c:105397/105434) hacen un DrawFillRect blanco y LUEGO una
> llamada virtual que dibuja el logo/arte encima con un fade de alpha
> (contador interno < 0x10 -> alpha parcial, si no -> opaco). Hipotesis sin
> confirmar todavia (investigacion estatica, sin acceso a consola real):
> el fade nunca llega a alpha=255/blend opaco, o el blend/textencv activo
> deja el quad invisible. Los logs de glColorPointer/glTexEnvf/glBlendFunc ya
> existian pero estaban topados a los primeros 10-20 llamados (aca ya paso
> el menu) -- se saca el tope MIENTRAS el ui_status este en ese rango para
> poder confirmar con UN log mas en consola real cual de las dos hipotesis
> es la correcta. Quitar este helper (y los `|| zenonia_verbose_ui()` que lo
> usan) una vez que el bug de fondo este resuelto y confirmado.

---

## `GVUI_CONTROLLER_ACTIVE_COUNT_OFFSET` (line ~35)

**Source File:** `loader/dynlib.c`

> offset confirmado en GVUIController::GVUIController() (out_ghidra.c:26369,
> `memset(this+8,0,400); *(int*)(this+0x19c)=0;`) y reusado identico por
> Draw()/PointerPress()/PointerMove()/PointerRelease() como limite del loop
> sobre los hasta 100 punteros a hijo que arrancan en this+8.

---

## `GVUI_CONTROLLER_ACTIVE_COUNT_OFFSET` (line ~42)

**Source File:** `loader/dynlib.c`

> offset confirmado en GVUIController::GVUIController() (out_ghidra.c:26369,
> `memset(this+8,0,400); *(int*)(this+0x19c)=0;`) y reusado identico por
> Draw()/PointerPress()/PointerMove()/PointerRelease() como limite del loop
> sobre los hasta 100 punteros a hijo que arrancan en this+8.

---

## `zenonia_install_hide_dpad_hook` (line ~49)

**Source File:** `loader/dynlib.c`

> Llamar DESPUES de so_relocate/so_resolve (necesita el modulo ya con
> dynsym/dynstr resueltos, ver so_symbol()) y ANTES de la primera llamada a
> cualquier Native*() del juego -- GVUIPlayerController se construye durante
> el arranque normal del motor.

---

## `zenonia_install_hide_dpad_hook` (line ~60)

**Source File:** `loader/dynlib.c`

> Llamar DESPUES de so_relocate/so_resolve (necesita el modulo ya con
> dynsym/dynstr resueltos, ver so_symbol()) y ANTES de la primera llamada a
> cualquier Native*() del juego -- GVUIPlayerController se construye durante
> el arranque normal del motor.

---

## `glClearColorx_wrapper` (line ~85)

**Source File:** `loader/dynlib.c`

> Wrappers para OpenGL de punto fijo (GLES1)

---

## `convert_rgb565_to_rgba8888_neon` (line ~97)

**Source File:** `loader/dynlib.c`

> Prueba A/B de 4 estrategias para el framebuffer de software RGB565 que el
> motor sube por textura (ver CLAUDE.md / CMakeLists.txt: RGB565_CONVERT_MODE
> -- SCALAR/LUT/NEON convierten a RGBA8888 en CPU, solo cambia como; NATIVE
> no convierte nada, sube el RGB565 tal cual y deja que el sampler de GXM lo
> lea nativo). Selecciona UNA sola de las 4 (mutuamente excluyentes, no son
> flags independientes).

---

## `p` (line ~109)

**Source File:** `loader/dynlib.c`

> Wrappers para OpenGL de punto fijo (GLES1)

---

## `convert_rgb565_to_rgba8888` (line ~130)

**Source File:** `loader/dynlib.c`

> Buffer de conversion reutilizado entre llamadas: el motor sube el
> framebuffer 800x480 completo antes de CADA quad (varias veces por frame),
> y hacer malloc/free de 1.5 MB en cada upload era parte del costo por
> frame. El resultado es valido solo hasta la proxima llamada — los call
> sites lo consumen de inmediato en glTexImage2D/glTexSubImage2D.
> No se llama en modo NATIVE (ver call sites) -- se deja compilada igual por
> si se necesita volver a comparar en runtime.

---

## `new_buf` (line ~142)

**Source File:** `loader/dynlib.c`

> Buffer de conversion reutilizado entre llamadas: el motor sube el
> framebuffer 800x480 completo antes de CADA quad (varias veces por frame),
> y hacer malloc/free de 1.5 MB en cada upload era parte del costo por
> frame. El resultado es valido solo hasta la proxima llamada — los call
> sites lo consumen de inmediato en glTexImage2D/glTexSubImage2D.
> No se llama en modo NATIVE (ver call sites) -- se deja compilada igual por
> si se necesita volver a comparar en runtime.

---

## `r5_to_8` (line ~157)

**Source File:** `loader/dynlib.c`

> Buffer de conversion reutilizado entre llamadas: el motor sube el
> framebuffer 800x480 completo antes de CADA quad (varias veces por frame),
> y hacer malloc/free de 1.5 MB en cada upload era parte del costo por
> frame. El resultado es valido solo hasta la proxima llamada — los call
> sites lo consumen de inmediato en glTexImage2D/glTexSubImage2D.
> No se llama en modo NATIVE (ver call sites) -- se deja compilada igual por
> si se necesita volver a comparar en runtime.

---

## `glTexImage2D_wrapper` (line ~202)

**Source File:** `loader/dynlib.c`

> El motor sube su framebuffer interno de software (400x240, ver Fase 1 del
> plan) en RGB565. SCALAR/LUT/NEON lo convierten a RGBA8888 en CPU antes de
> subirlo (mismo bug confirmado en hardware real para Zenonia 2, mismo
> motor). NATIVE es la variante sin conversion: GL_UNSIGNED_SHORT_5_6_5 esta
> definido en vitaGL.h (0x8363) y SceGxm soporta R5G6B5 nativo en el
> sampler, asi que en teoria el motor puede subir el RGB565 tal cual y la
> GPU lo filtra sola -- SIN CONFIRMAR EN HARDWARE (el comentario original
> heredado de Zenonia 2 decia "vitaGL no lo maneja igual que el motor
> original espera" sin dejar evidencia de que se haya probado el passthrough
> puro; si aparece textura corrupta/canal de color desplazado en el log,
> ese es el primer sospechoso).

---

## `glTexSubImage2D_wrapper` (line ~230)

**Source File:** `loader/dynlib.c`

> Forzar filtros min/mag para que la textura no se trate como incompleta por falta de mipmaps

---

## `pending_fixed_verts` (line ~268)

**Source File:** `loader/dynlib.c`

> Forzar filtros min/mag para que la textura no se trate como incompleta por falta de mipmaps

---

## `fixed_to_float_neon` (line ~291)

**Source File:** `loader/dynlib.c`

> Prueba A/B contra el loop escalar (ver CLAUDE.md / CMakeLists.txt:
> OPTIMIZE_NEON_FIXED). Solo valido cuando el array es tightly-packed
> (stride_elems == size, el caso normal para estos 3 atributos en este
> motor) -- el llamador cae al loop escalar original si no lo es, en vez de
> asumir. Resultado bit-a-bit identico al escalar: dividir por 65536.0f o
> multiplicar por su reciproco (1/65536, potencia de 2 exacta en float) no
> introduce redondeo extra.

---

## `stride_elems` (line ~320)

**Source File:** `loader/dynlib.c`

> Prueba A/B contra el loop escalar (ver CLAUDE.md / CMakeLists.txt:
> OPTIMIZE_NEON_FIXED). Solo valido cuando el array es tightly-packed
> (stride_elems == size, el caso normal para estos 3 atributos en este
> motor) -- el llamador cae al loop escalar original si no lo es, en vez de
> asumir. Resultado bit-a-bit identico al escalar: dividir por 65536.0f o
> multiplicar por su reciproco (1/65536, potencia de 2 exacta en float) no
> introduce redondeo extra.

---

## `glTexEnvf_wrapper` (line ~413)

**Source File:** `loader/dynlib.c`

> Sin limite de log hasta ahora -- el motor llama esto por cada
> sprite/textura dibujada, TODOS los frames (a diferencia de sus vecinos en
> este archivo, que ya capan a las primeras ~10-20 llamadas). Cada
> game_log() hace fprintf+fflush sincronico (I/O bloqueante a disco, mismo
> costo que ENABLE_VERBOSE_JNI_LOG documenta en CLAUDE.md) -- sin este cap
> era un fflush por sprite dibujado, suficiente para tirar el framerate de
> 60 a ~13 fps sostenidos en consola real.

---

## `glBlendFunc_wrapper` (line ~426)

**Source File:** `loader/dynlib.c`

> Sin wrapper hasta ahora (pasaba directo a vitaGL) -- se agrega SOLO para
> loguear durante logo/titulo/menu, ver nota de zenonia_verbose_ui() arriba.

---

## `c` (line ~495)

**Source File:** `loader/dynlib.c`

> struct stat con el layout de bionic (Android ARM 32-bit, NDK android-9) --
> NO es el de newlib/vitasdk. El motor lee directamente st_mode en el offset
> 16 y st_size en el 48 (confirmado en Zenonia 2 desensamblando
> MC_fsFileAttribute -- mismo motor y mismo consumidor aqui, ver
> out_ghidra.c:150281: los offsets que Ghidra decompila para el stat local
> coinciden con bionic). Pasarle el struct stat de newlib deja esos offsets
> con basura de stack: en Zenonia 2, al cargar una partida guardada, el
> "tamano" leido era un puntero del heap (MALLOC FAILED FOR SIZE 0x81340CE0)
> y el motor crasheaba -- fix portado ANTES de que el sintoma aparezca aqui
> (Zenonia2 port_progress.md §12.4).

---

## `glOrthof_wrapper` (line ~603)

**Source File:** `loader/dynlib.c`

> Pass-through deliberado: el motor manda una proyeccion CENTRADA
> (-400..400, -240..240) y sus vertices de quad de pantalla completa usan ese
> mismo sistema (GL_FIXED -400..400/-240..240, confirmado en los logs).
> Forzar aqui un sistema con origen en la esquina (0..800, 480..0) desplaza
> ese quad a un solo cuadrante de la pantalla e invierte Y — regresion real
> vista en consola (cuadro chico blanco sobre negro, log_1783657108).

---

## `__cxa_begin_cleanup` (line ~622)

**Source File:** `loader/dynlib.c`

> Convert from fixed point (Q16.16) to float

---

## `__cxa_atexit` (line ~639)

**Source File:** `loader/dynlib.c`

> Registro de destructores estaticos de C++ (llamados normalmente al
> dlclose()/exit() de una libreria dinamica real). Este loader nunca
> descarga el .so ni llama exit() de forma ordenada -- el proceso termina
> con sceKernelExitProcess() -- asi que no hace falta ejecutar los
> destructores registrados: alcanza con no-ops que devuelvan éxito. Faltaban
> en la tabla de imports pese a estar confirmados en la Fase 1 del plan
> (objdump -T | grep UND), causando "Unknown symbol" y crash real en
> consola (ver port_progress.md).

---

## `pthread_mutex_lock_wrapper` (line ~655)

**Source File:** `loader/dynlib.c`

> pthread_mutex_t/pthread_cond_t en VitaSDK son en realidad PUNTEROS
> (typedef struct pthread_mutex_t_ * pthread_mutex_t;) a una estructura
> interna que aloca pthread_mutex_init(). El .so esta compilado contra
> Bionic (Android), donde un mutex/condvar estatico declarado con
> PTHREAD_MUTEX_INITIALIZER/PTHREAD_COND_INITIALIZER queda en CEROS sin
> llamar nunca a pthread_mutex_init en tiempo de ejecucion -- Bionic esta
> disenado para tratar esos ceros como "mutex valido, sin lockear" de
> forma nativa. En VitaSDK esos mismos ceros son un puntero NULL real, y
> pthread_mutex_lock/unlock de PTE (pthreads-embedded) lo desreferencian
> sin chequear, crasheando (confirmado en consola real: Data abort dentro
> de pthread_mutex_unlock, llamado desde el node-allocator interno de
> libstdc++ sobre un mutex estatico nunca inicializado -- ver
> port_progress.md). Se detecta el puntero NULL y se inicializa on-demand
> antes de usarlo, en vez de pasar el valor crudo a la implementacion real.

---

## `new_path` (line ~733)

**Source File:** `loader/dynlib.c`

> strncpy no garantiza terminador si in_path >= out_size (256, ver
> fopen_hook/stat_hook/access_hook) -- snprintf trunca y siempre
> termina en '\0', evitando leer basura de stack como path.

---

## `struct` (line ~748)

**Source File:** `loader/dynlib.c`

> struct stat con el layout de bionic (Android ARM 32-bit, NDK android-9) --
> NO es el de newlib/vitasdk. El motor lee directamente st_mode en el offset
> 16 y st_size en el 48 (confirmado en Zenonia 2 desensamblando
> MC_fsFileAttribute -- mismo motor y mismo consumidor aqui, ver
> out_ghidra.c:150281: los offsets que Ghidra decompila para el stat local
> coinciden con bionic). Pasarle el struct stat de newlib deja esos offsets
> con basura de stack: en Zenonia 2, al cargar una partida guardada, el
> "tamano" leido era un puntero del heap (MALLOC FAILED FOR SIZE 0x81340CE0)
> y el motor crasheaba -- fix portado ANTES de que el sintoma aparezca aqui
> (Zenonia2 port_progress.md §12.4).

---

## `dynlib.c` (line ~890) (line ~890)

**Source File:** `loader/dynlib.c`

> Tabla de resolucion de imports. Confirmada 1:1 contra
> `objdump -T libgameDSO.so | grep UND` (92 simbolos, ver Fase 1 del plan):
> este motor no importa __android_log_print en este build (a diferencia de
> Zenonia 2), asi que no se registra -- si aparece en un build futuro
> alcanza con agregar la entrada, la funcion ya existe en main.c.

---

## `dynlib.c` (line ~904) (line ~904)

**Source File:** `loader/dynlib.c`

> Tabla de resolucion de imports. Confirmada 1:1 contra
> `objdump -T libgameDSO.so | grep UND` (92 simbolos, ver Fase 1 del plan):
> este motor no importa __android_log_print en este build (a diferencia de
> Zenonia 2), asi que no se registra -- si aparece en un build futuro
> alcanza con agregar la entrada, la funcion ya existe en main.c.

---
