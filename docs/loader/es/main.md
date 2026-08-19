# `loader/main.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `main.c` (line ~1) (line ~1)

**Source File:** `loader/main.c`

> main.c
>
> ARMv7 Shared Libraries loader. Zenonia 3.
>
> Mismo motor (Gamevil Nexus2/"Clet") que Zenonia 2 -- ver
> ../Zenonia2-vita/loader/main.c y plan_zenonia3_port.md seccion 0 para el
> detalle de que es identico y que cambio de ABI real entre ambos juegos.

---

## `GAME_W` (line ~30)

**Source File:** `loader/main.c`

> Resolucion LOGICA del juego: 400x240 fijo. Confirmado por triple fuente
> (2026-07-10): el Java original pasa gameScreenWidth/Height = 400/240 a
> NativeInitDeviceInfo/NativeInitWithBufferSize (NexusGLActivity.java:85-86,
> jadx renombro la constante 400 como UI_STATUS_PURCHASE_PAGE), getDeviceInfo()
> defaultea di[3]=400 di[4]=0xf0 (out_ghidra.c:148015), y el codigo de dibujo
> del titulo esta hardcodeado a ese espacio (CMvTitleState::DrawZeroGrade
> hace DrawFillRect(0,0,400,240) y centra con (400-w)>>1 -- out_ghidra.c:105397).
> Inicializar a 800x480 hacia que el juego pintara solo el cuadrante superior
> izquierdo del framebuffer (sprites chicos + el resto blanco/negro, visto en
> consola real). El escalado a pantalla completa lo hace el MOTOR: glResize()
> arma el quad con vertices +-w/2 del tamano que le pasemos a NativeResize
> (out_ghidra.c:148107+), asi que NativeResize recibe SCREEN_W/H (960x544).

---

## `gl_active` (line ~41)

**Source File:** `loader/main.c`

> Resolucion LOGICA del juego: 400x240 fijo. Confirmado por triple fuente
> (2026-07-10): el Java original pasa gameScreenWidth/Height = 400/240 a
> NativeInitDeviceInfo/NativeInitWithBufferSize (NexusGLActivity.java:85-86,
> jadx renombro la constante 400 como UI_STATUS_PURCHASE_PAGE), getDeviceInfo()
> defaultea di[3]=400 di[4]=0xf0 (out_ghidra.c:148015), y el codigo de dibujo
> del titulo esta hardcodeado a ese espacio (CMvTitleState::DrawZeroGrade
> hace DrawFillRect(0,0,400,240) y centra con (400-w)>>1 -- out_ghidra.c:105397).
> Inicializar a 800x480 hacia que el juego pintara solo el cuadrante superior
> izquierdo del framebuffer (sprites chicos + el resto blanco/negro, visto en
> consola real). El escalado a pantalla completa lo hace el MOTOR: glResize()
> arma el quad con vertices +-w/2 del tamano que le pasemos a NativeResize
> (out_ghidra.c:148107+), asi que NativeResize recibe SCREEN_W/H (960x544).

---

## `init_log` (line ~50)

**Source File:** `loader/main.c`

> Un archivo de log por corrida, con timestamp, dentro de logs/ -- mantiene
> el historial completo entre pruebas en vez de pisar siempre el mismo
> log.txt (ver psvita-porting skill, hardware_debugging.md).

---

## `main.c` (line ~91) (line ~91)

**Source File:** `loader/main.c`

> Un archivo de log por corrida, con timestamp, dentro de logs/ -- mantiene
> el historial completo entre pruebas en vez de pisar siempre el mismo
> log.txt (ver psvita-porting skill, hardware_debugging.md).

---

## `__android_log_print` (line ~103)

**Source File:** `loader/main.c`

> No confirmado que el .so de Zenonia 3 lo importe (ver Fase 1 del plan: no
> aparece en `objdump -T | grep UND`), pero se deja disponible por si algun
> build futuro lo necesita -- no hace dano tenerlo sin registrar en
> dynlib.c.

---

## `main.c` (line ~122) (line ~122)

**Source File:** `loader/main.c`

> No confirmado que el .so de Zenonia 3 lo importe (ver Fase 1 del plan: no
> aparece en `objdump -T | grep UND`), pero se deja disponible por si algun
> build futuro lo necesita -- no hace dano tenerlo sin registrar en
> dynlib.c.

---

## `MH_KEY_PRESSEVENT` (line ~134)

**Source File:** `loader/main.c`

> --- Input: protocolo pendiente de reconfirmar contra el Java real de
> Zenonia 3 (Fase 5 del plan -- decompilar ZenoniaUIControllerView y el
> NexusHal equivalente, no asumir que es igual a Zenonia2UIControllerView).
> Se arranca con los mismos codigos MH_* y keycodes HAL de Zenonia 2 porque
> son constantes del MOTOR (Nexus2/Clet), no de la capa de juego -- pero
> deben confirmarse con un log real antes de dar esto por bueno.
>
> Diferencia real de ABI: no existe setInputEvent en este .so. Solo hay
> handleCletEvent, que ahora toma 4 ints (antes 3) -- ver Fase 1 del plan y
> out_ghidra.c:147900. Se sigue encolando un evento por frame y entregandolo
> justo antes de NativeRender (mismo orden que NexusGLRenderer.drawFrame en
> Zenonia 2), sin la entrega inmediata que hacia setInputEvent, porque acá
> no existe ese canal.

---

## `NEXUS_HAL_REPLY_YESNO` (line ~155)

**Source File:** `loader/main.c`

> Constantes reales de NexusHal.java -- el boton "Continuar" del menu no
> manda un press/release de tecla como los demas, manda un UNICO evento con
> este tipo/parametro (ver Natives.java: img_menu_continue.setOnClickListener
> -> handleCletEvent(NexusHal.REPLY_YESNO, NexusHal.FIRST_MOVE_REPLY_PAGE, 0, 0)).

---

## `NEXUS_HAL_YES_MOVE_REPLY_PAGE` (line ~161)

**Source File:** `loader/main.c`

> Los botones "escribir resena"/"mas tarde" de la propia pantalla de
> REPLY_PAGE (ver Natives.java: showReplyMoveComponent(), img_btn_write/
> img_btn_later OnClickListener) mandan el mismo tipo REPLY_YESNO con estos
> otros dos parametros.

---

## `zenonia_install_array_hooks` (line ~203)

**Source File:** `loader/main.c`

> NOTA: a diferencia de Zenonia 2, aca NO se porta ningun parche binario
> (apply_so_patches). El bug de Zenonia2 (puntero de heap tratado como
> signed en CMvLayerData::PreLoad) es un patron de motor viejo que puede
> reaparecer en este .so, pero en un offset DISTINTO -- no reusar el numero
> 0xaec38 a ciegas. Si aparece el mismo sintoma (NULL "imposible" en datos
> que deberian haberse cargado), re-derivar el offset con vita-parse-core +
> objdump -d sobre ESTE binario (ver Fase 7 del plan).

---

## `main.c` (line ~274) (line ~274)

**Source File:** `loader/main.c`

> El vsync interno de vitaGL solo espera 1 vblank (panel a 60Hz) -- con
> RGB565_CONVERT_MODE=NATIVE el frame a veces entra en ese vblank y a
> veces no, y el resultado es un framerate que rebota entre ~60 y ~40
> (el jitter/stutter tipico de estar justo en el borde del vblank). Se
> apaga el vsync de vitaGL y se toma control manual del pacing en el
> loop principal (sceDisplayWaitVblankStartMulti(2)) para forzar
> siempre 2 vblanks por frame -- 30fps estable y sin tearing en vez de
> "hasta 60 pero irregular".

---

## `main.c` (line ~279) (line ~279)

**Source File:** `loader/main.c`

> El vsync interno de vitaGL solo espera 1 vblank (panel a 60Hz) -- con
> RGB565_CONVERT_MODE=NATIVE el frame a veces entra en ese vblank y a
> veces no, y el resultado es un framerate que rebota entre ~60 y ~40
> (el jitter/stutter tipico de estar justo en el borde del vblank). Se
> apaga el vsync de vitaGL y se toma control manual del pacing en el
> loop principal (sceDisplayWaitVblankStartMulti(2)) para forzar
> siempre 2 vblanks por frame -- 30fps estable y sin tearing en vez de
> "hasta 60 pero irregular".

---

## `raise_clocks` (line ~285)

**Source File:** `loader/main.c`

> Clocks conservadores de fabrica (CPU 333MHz / bus 166MHz / GPU 111MHz) --
> subir a los maximos estables conocidos en homebrew de Vita es standard
> practice (mismo approach usado en la mayoria de ports vitaGL) y no toca
> ninguna logica del juego, solo la frecuencia real del hardware.

---

## `raise_clocks` (line ~294)

**Source File:** `loader/main.c`

> Clocks conservadores de fabrica (CPU 333MHz / bus 166MHz / GPU 111MHz) --
> subir a los maximos estables conocidos en homebrew de Vita es standard
> practice (mismo approach usado en la mayoria de ports vitaGL) y no toca
> ninguna logica del juego, solo la frecuencia real del hardware.

---

## `text_base` (line ~324)

**Source File:** `loader/main.c`

> Clocks conservadores de fabrica (CPU 333MHz / bus 166MHz / GPU 111MHz) --
> subir a los maximos estables conocidos en homebrew de Vita es standard
> practice (mismo approach usado en la mayoria de ports vitaGL) y no toca
> ninguna logica del juego, solo la frecuencia real del hardware.

---

## `main.c` (line ~361) (line ~361)

**Source File:** `loader/main.c`

> Secuencia de arranque -- reemplaza al NativeInit() unico de
> Zenonia 2 (que no existe aca).
>
> ORDEN CONFIRMADO CON UN CRASH REAL (ver port_progress.md Fase 3,
> bug #2): NativeInitWithBufferSize DEBE llamarse ANTES que
> NativeInitDeviceInfo. NativeInitWithBufferSize dispara
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), que crea el
> pool de memoria PROPIO del motor (un allocator custom tipo slab,
> separado de malloc/new) del que depende Gcx_MM_Alloc/Calloc.
> NativeInitDeviceInfo usa ese mismo allocator para el buffer de
> pixeles interno (via getDeviceInfo() -> Gcx_MM_Calloc) -- si se
> llama primero, el pool todavia no existe, Gcx_MM_Alloc devuelve
> NULL, y el memset(NULL,0,n) subsiguiente hace Data Abort (visto
> en consola real, resuelto con vita-parse-core + objdump -d contra
> libgameDSO.so: LR cae en Gcx_MM_Calloc+0x13, justo despues del
> blx a memset@plt).

---

## `pad` (line ~371)

**Source File:** `loader/main.c`

> Secuencia de arranque -- reemplaza al NativeInit() unico de
> Zenonia 2 (que no existe aca).
>
> ORDEN CONFIRMADO CON UN CRASH REAL (ver port_progress.md Fase 3,
> bug #2): NativeInitWithBufferSize DEBE llamarse ANTES que
> NativeInitDeviceInfo. NativeInitWithBufferSize dispara
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), que crea el
> pool de memoria PROPIO del motor (un allocator custom tipo slab,
> separado de malloc/new) del que depende Gcx_MM_Alloc/Calloc.
> NativeInitDeviceInfo usa ese mismo allocator para el buffer de
> pixeles interno (via getDeviceInfo() -> Gcx_MM_Calloc) -- si se
> llama primero, el pool todavia no existe, Gcx_MM_Alloc devuelve
> NULL, y el memset(NULL,0,n) subsiguiente hace Data Abort (visto
> en consola real, resuelto con vita-parse-core + objdump -d contra
> libgameDSO.so: LR cae en Gcx_MM_Calloc+0x13, justo despues del
> blx a memset@plt).

---

## `fps_count` (line ~392)

**Source File:** `loader/main.c`

> Secuencia de arranque -- reemplaza al NativeInit() unico de
> Zenonia 2 (que no existe aca).
>
> ORDEN CONFIRMADO CON UN CRASH REAL (ver port_progress.md Fase 3,
> bug #2): NativeInitWithBufferSize DEBE llamarse ANTES que
> NativeInitDeviceInfo. NativeInitWithBufferSize dispara
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), que crea el
> pool de memoria PROPIO del motor (un allocator custom tipo slab,
> separado de malloc/new) del que depende Gcx_MM_Alloc/Calloc.
> NativeInitDeviceInfo usa ese mismo allocator para el buffer de
> pixeles interno (via getDeviceInfo() -> Gcx_MM_Calloc) -- si se
> llama primero, el pool todavia no existe, Gcx_MM_Alloc devuelve
> NULL, y el memset(NULL,0,n) subsiguiente hace Data Abort (visto
> en consola real, resuelto con vita-parse-core + objdump -d contra
> libgameDSO.so: LR cae en Gcx_MM_Calloc+0x13, justo despues del
> blx a memset@plt).

---

## `pressed` (line ~407)

**Source File:** `loader/main.c`

> Secuencia de arranque -- reemplaza al NativeInit() unico de
> Zenonia 2 (que no existe aca).
>
> ORDEN CONFIRMADO CON UN CRASH REAL (ver port_progress.md Fase 3,
> bug #2): NativeInitWithBufferSize DEBE llamarse ANTES que
> NativeInitDeviceInfo. NativeInitWithBufferSize dispara
> startClet() -> GxCreateGlobalHeap() -> Gcx_MM_Init(), que crea el
> pool de memoria PROPIO del motor (un allocator custom tipo slab,
> separado de malloc/new) del que depende Gcx_MM_Alloc/Calloc.
> NativeInitDeviceInfo usa ese mismo allocator para el buffer de
> pixeles interno (via getDeviceInfo() -> Gcx_MM_Calloc) -- si se
> llama primero, el pool todavia no existe, Gcx_MM_Alloc devuelve
> NULL, y el memset(NULL,0,n) subsiguiente hace Data Abort (visto
> en consola real, resuelto con vita-parse-core + objdump -d contra
> libgameDSO.so: LR cae en Gcx_MM_Calloc+0x13, justo despues del
> blx a memset@plt).

---

## `pressed` (line ~413)

**Source File:** `loader/main.c`

> El tamano de PANTALLA, no el del buffer -- igual que Android, donde
> NativeResize recibe el tamano de la GLSurfaceView real y el motor
> arma solo el quad de escalado (ver nota en GAME_W/GAME_H).

---

## `main.c` (line ~422) (line ~422)

**Source File:** `loader/main.c`

> El tamano de PANTALLA, no el del buffer -- igual que Android, donde
> NativeResize recibe el tamano de la GLSurfaceView real y el motor
> arma solo el quad de escalado (ver nota en GAME_W/GAME_H).

---

## `x` (line ~441)

**Source File:** `loader/main.c`

> El tamano de PANTALLA, no el del buffer -- igual que Android, donde
> NativeResize recibe el tamano de la GLSurfaceView real y el motor
> arma solo el quad de escalado (ver nota en GAME_W/GAME_H).

---

## `sx` (line ~450)

**Source File:** `loader/main.c`

> Contador de FPS real (para comparar builds A/B de las
> optimizaciones RGB565_LUT/NEON_FIXED contra un mismo punto de
> referencia en el log, en vez de "a ojo"). Ventana de ~2s en vez de
> por-frame para no agregar overhead de log al loop caliente.

---

## `main.c` (line ~458) (line ~458)

**Source File:** `loader/main.c`

> Contador de FPS real (para comparar builds A/B de las
> optimizaciones RGB565_LUT/NEON_FIXED contra un mismo punto de
> referencia en el log, en vez de "a ojo"). Ventana de ~2s en vez de
> por-frame para no agregar overhead de log al loop caliente.

---

## `main.c` (line ~502) (line ~502)

**Source File:** `loader/main.c`

> Natives.showTitleComponent(): gb_titleImg (fondo,
> pantalla completa) manda BACK press+release ante
> CUALQUIER toque.

---

## `main.c` (line ~515) (line ~515)

**Source File:** `loader/main.c`

> Natives.showTitleComponent(): gb_titleImg (fondo,
> pantalla completa) manda BACK press+release ante
> CUALQUIER toque.

---

## `main.c` (line ~564) (line ~564)

**Source File:** `loader/main.c`

> Mientras el motor este en logo/titulo (estados de UI de Java,
> invisibles en el loader nativo -- pendiente confirmar los
> numeros de estado reales de Zenonia 3 en Fase 5), tapar con
> el splash.

---

## `main.c` (line ~570) (line ~570)

**Source File:** `loader/main.c`

> Mientras el motor este en logo/titulo (estados de UI de Java,
> invisibles en el loader nativo -- pendiente confirmar los
> numeros de estado reales de Zenonia 3 en Fase 5), tapar con
> el splash.

---

## `now` (line ~577)

**Source File:** `loader/main.c`

> Mientras el motor este en logo/titulo (estados de UI de Java,
> invisibles en el loader nativo -- pendiente confirmar los
> numeros de estado reales de Zenonia 3 en Fase 5), tapar con
> el splash.

---
