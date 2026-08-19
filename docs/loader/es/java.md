# `loader/java.c` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `java.c` (line ~1) (line ~1)

**Source File:** `loader/java.c`

> java.c
>
> "Java-side" native method handlers that libgameDSO.so (Zenonia 3, mismo
> motor Gamevil Nexus2/Clet que Zenonia 2) llama de vuelta vía FalsoJNI
> (GetStaticMethodID + CallStaticObjectMethod/CallStaticIntMethod/etc).
> Todo lo que no se registra aca simplemente queda "not found" para
> FalsoJNI (logueado, no fatal para metodos void/Object -- ver
> plan_zenonia3_port.md seccion "Riesgos" y port_progress.md de Zenonia2
> §9.10 para el caso contrario: metodos int/boolean SI son peligrosos si
> no se registran).

---

## `zenonia_resolve_asset_path` (line ~21)

**Source File:** `loader/java.c`

> readAssets/isAssetExist se llaman siempre con el mismo path relativo (el
> motor llama isAssetExist(path) antes de decidir si vale la pena llamar
> readAssets(path)), asi que ambos deben resolver igual. Igual que en
> Zenonia 2: probar primero el path pelado (ux0:data/zenonia3/<name>) y si
> no existe, el prefijado con assets/ (que usan los hooks de dynlib.c para
> todo lo demas) -- sin asumir cual es la convencion real hasta confirmarla
> con un log de consola.

---

## `ZENONIA_DALVIK_REGISTRY_MAX` (line ~36)

**Source File:** `loader/java.c`

> --- Puente para que readAssets sirva a los DOS consumidores distintos que
> tiene el motor ---
>
> El truco de "ArrayObject de Dalvik" (header de 16 bytes + datos crudos,
> ver Zenonia_readAssets) funciona para el consumidor directo por puntero
> (MC_knlGetResource y el resto de CMvResourceMgr) porque ese codigo nunca
> pasa por las funciones estandar de arrays de JNI -- lee offset+16
> directamente. Pero un consumidor DISTINTO (confirmado real: la familia de
> parsers "PZx"/"PZD" -- CGxPZDParser/CGxZeroPZDParser -- usada para
> TouchOemIME.pzx y otros .pzx, ver port_progress.md Fase 3.7) SI llama a
> GetArrayLength/GetByteArrayElements estandar sobre el mismo resultado de
> readAssets. Como nuestro bloque no es un JavaDynArray real (no paso por
> jda_alloc), FalsoJNI no lo encuentra ("Could not find the array") y esos
> parsers reciben NULL/longitud 0 -- causando un crash rio abajo (puntero
> NULL asumido valido tras un "exito" a medias del parser).
>
> Solucion: interceptar GetArrayLength/GetByteArrayElements/
> GetByteArrayRegion/ReleaseByteArrayElements de la tabla de funciones JNI
> (mutable en memoria pese al `const` del tipo publico -- jni_init() la
> aloca con malloc()) para reconocer nuestros propios bloques (llevando un
> registro de punteros devueltos por Zenonia_readAssets) y servirlos
> directamente desde el header Dalvik, cayendo al codigo real de FalsoJNI
> para cualquier otro array (los JavaDynArray genuinos de jda_alloc, usados
> por ejemplo en las funciones GFA_* de la Fase 3.5).

---

## `zenonia_install_array_hooks` (line ~98)

**Source File:** `loader/java.c`

> Llamar DESPUES de jni_init() (main.c) -- necesita que la tabla de
> funciones ya este alocada.

---

## `java.c` (line ~120) (line ~120)

**Source File:** `loader/java.c`

> Typo real del motor (heredado de Zenonia 2, confirmado en Zenonia 3
> con `strings libgameDSO.so`: existen ambas cadenas "readAssets" y
> "readAssete" en el binario). Mismo handler para las dos.

---

## `java.c` (line ~133) (line ~133)

**Source File:** `loader/java.c`

> Typo real del motor (heredado de Zenonia 2, confirmado en Zenonia 3
> con `strings libgameDSO.so`: existen ambas cadenas "readAssets" y
> "readAssete" en el binario). Mismo handler para las dos.

---

## `java.c` (line ~140) (line ~140)

**Source File:** `loader/java.c`

> Typo real del motor (heredado de Zenonia 2, confirmado en Zenonia 3
> con `strings libgameDSO.so`: existen ambas cadenas "readAssets" y
> "readAssete" en el binario). Mismo handler para las dos.

---

## `java.c` (line ~147) (line ~147)

**Source File:** `loader/java.c`

> Typo real del motor (heredado de Zenonia 2, confirmado en Zenonia 3
> con `strings libgameDSO.so`: existen ambas cadenas "readAssets" y
> "readAssete" en el binario). Mismo handler para las dos.

---

## `java.c` (line ~174) (line ~174)

**Source File:** `loader/java.c`

> No-ops seguros (void): evitan spam de "not found" en el log.

---

## `java.c` (line ~184) (line ~184)

**Source File:** `loader/java.c`

> Nuevos en el UIListener de Zenonia 3 (no existian en Zenonia 2):
> OnVibrate(int) y OnEvent(int). Ambos void, ya serian seguros sin
> registrar (ver nota de arriba), pero se registran para no spamear
> el log y para tener el punto de entrada listo cuando llegue la Fase
> de audio/vibracion.

---

## `java.c` (line ~193) (line ~193)

**Source File:** `loader/java.c`

> Devuelven float[]/int[]/short[] que el motor desreferencia SIN
> chequear NULL -- confirmado con un crash real (Data abort dentro de
> GFA_DrawFont) para DrawFont; DrawText/MeasureText comparten la misma
> estructura de wrapper nativo (ver port_progress.md Fase 3.5). Object,
> NUNCA deben devolver NULL en el camino normal.

---

## `name` (line ~200)

**Source File:** `loader/java.c`

> Devuelven float[]/int[]/short[] que el motor desreferencia SIN
> chequear NULL -- confirmado con un crash real (Data abort dentro de
> GFA_DrawFont) para DrawFont; DrawText/MeasureText comparten la misma
> estructura de wrapper nativo (ver port_progress.md Fase 3.5). Object,
> NUNCA deben devolver NULL en el camino normal.

---

## `name` (line ~206)

**Source File:** `loader/java.c`

> Devuelven float[]/int[]/short[] que el motor desreferencia SIN
> chequear NULL -- confirmado con un crash real (Data abort dentro de
> GFA_DrawFont) para DrawFont; DrawText/MeasureText comparten la misma
> estructura de wrapper nativo (ver port_progress.md Fase 3.5). Object,
> NUNCA deben devolver NULL en el camino normal.

---

## `struct` (line ~230)

**Source File:** `loader/java.c`

> Registrado porque un methodID no encontrado hace que methodIntCall() de
> FalsoJNI devuelva -1 (ver FalsoJNI_ImplBridge.c) -- un valor no-cero que
> el motor interpreta como booleano C "true" (el archivo existe). Ese falso
> positivo fue la causa real de un crash en Zenonia 2 (§9.10 de su
> port_progress.md): el motor seguia adelante cargando un archivo que en
> realidad no se habia resuelto.

---

## `java.c` (line ~263) (line ~263)

**Source File:** `loader/java.c`

> fstat en vez de fseek(SEEK_END)+ftell: en Zenonia 2, ftell() devolvio
> basura (bytes de la propia ruta) para al menos un archivo real y
> corrompio el malloc posterior del motor. fstat no depende de la
> posicion del cursor y evita esa clase de bug de raiz.

---

## `name` (line ~278)

**Source File:** `loader/java.c`

> fstat en vez de fseek(SEEK_END)+ftell: en Zenonia 2, ftell() devolvio
> basura (bytes de la propia ruta) para al menos un archivo real y
> corrompio el malloc posterior del motor. fstat no depende de la
> posicion del cursor y evita esa clase de bug de raiz.

---

## `len` (line ~315)

**Source File:** `loader/java.c`

> Dalvik ArrayObject espera el largo como entero de 32 bits en el offset 8

---

## `len` (line ~323)

**Source File:** `loader/java.c`

> Registrado porque un methodID no encontrado hace que methodIntCall() de
> FalsoJNI devuelva -1 (ver FalsoJNI_ImplBridge.c) -- un valor no-cero que
> el motor interpreta como booleano C "true" (el archivo existe). Ese falso
> positivo fue la causa real de un crash en Zenonia 2 (§9.10 de su
> port_progress.md): el motor seguia adelante cargando un archivo que en
> realidad no se habia resuelto.

---

## `Zenonia_OnSoundPlay` (line ~362)

**Source File:** `loader/java.c`

> Firma real: OnSoundPlay(int sndID, int vol, boolean isLoop) -- el segundo
> parametro es VOLUMEN (0-100, tipico 50/75), el tercero el loop. Los logs
> viejos los etiquetaban al reves (misma correccion que Zenonia 2 §12.1).

---

## `GFA_MAX_FONTS` (line ~394)

**Source File:** `loader/java.c`

> --- Puente GFA (fuentes) -- replica de NexusFont.java con rasterizado REAL
> via loader/font.c (stb_truetype + app0:font.ttf). La fuente de verdad de
> cada semantica es NexusFont.java (jadx); el formato de pixeles que consume
> el motor esta confirmado en out_ghidra.c (CopyPixelsToCharCacheBuffer usa
> SOLO el canal alfa, stride = ancho del bitmap de GFA_Init).

---

## `g_gfa_str` (line ~410)

**Source File:** `loader/java.c`

> --- Puente GFA (fuentes) -- replica de NexusFont.java con rasterizado REAL
> via loader/font.c (stb_truetype + app0:font.ttf). La fuente de verdad de
> cada semantica es NexusFont.java (jadx); el formato de pixeles que consume
> el motor esta confirmado en out_ghidra.c (CopyPixelsToCharCacheBuffer usa
> SOLO el canal alfa, stride = ancho del bitmap de GFA_Init).

---

## `gfa_decode_utf8` (line ~416)

**Source File:** `loader/java.c`

> --- Decodificadores de string (NexusFont recibe UTF-8 (jstring), UTF-16LE
> o EUC-KR/KSC5601 segun la funcion) ---

---

## `gfa_decode_utf8` (line ~424)

**Source File:** `loader/java.c`

> --- Decodificadores de string (NexusFont recibe UTF-8 (jstring), UTF-16LE
> o EUC-KR/KSC5601 segun la funcion) ---

---

## `gfa_char_advance` (line ~485)

**Source File:** `loader/java.c`

> Metricas con fallback si la fuente no cargo (mismas aproximaciones que los
> stubs de la Fase 3.4, para que el juego no pierda el layout por completo).

---

## `gfa_break_text` (line ~500)

**Source File:** `loader/java.c`

> Paint.breakText(text, true, maxWidth): caracteres desde el inicio cuyo
> avance acumulado entra en maxWidth.

---

## `gfa_word_break_length` (line ~511)

**Source File:** `loader/java.c`

> BreakIterator de palabras, aproximado: una frontera despues de cada corrida
> de espacios, y cada caracter CJK/Hangul es su propia "palabra" (igual que
> el BreakIterator real para ideografos). Devuelve la cantidad de caracteres
> hasta la ultima frontera de palabra que entra en fit_chars (breakLength del
> Java); 0 si ninguna frontera entra.

---

## `count` (line ~539)

**Source File:** `loader/java.c`

> BreakIterator de palabras, aproximado: una frontera despues de cada corrida
> de espacios, y cada caracter CJK/Hangul es su propia "palabra" (igual que
> el BreakIterator real para ideografos). Devuelve la cantidad de caracteres
> hasta la ultima frontera de palabra que entra en fit_chars (breakLength del
> Java); 0 si ninguna frontera entra.

---

## `java.c` (line ~562) (line ~562)

**Source File:** `loader/java.c`

> Asegura los buffers de pixeles persistentes con el tamano actual de
> GFA_Init (si el motor re-inicializa con otro tamano, se liberan y realocan
> -- jda_free deja el slot de la tabla reutilizable sin mover el resto).

---

## `Zenonia_GFA_SetTextSize` (line ~580)

**Source File:** `loader/java.c`

> JNI promueve float a double al pasarlo por un va_list variadico (regla del
> lenguaje C, independiente de la ABI de punto flotante del target) -- por
> eso se lee con va_arg(args, double) y se castea a float, no al reves.

---

## `style` (line ~589)

**Source File:** `loader/java.c`

> Asegura los buffers de pixeles persistentes con el tamano actual de
> GFA_Init (si el motor re-inicializa con otro tamano, se liberan y realocan
> -- jda_free deja el slot de la tabla reutilizable sin mover el resto).

---

## `maxWidth` (line ~629)

**Source File:** `loader/java.c`

> Replica la reutilizacion de slots de NexusFont.GFA_CreateFont: hasta
> GFA_MAX_FONTS familias distintas, devuelve el mismo handle si la familia ya
> esta registrada, -1 si no quedan slots libres (igual que el original).

---

## `java.c` (line ~662) (line ~662)

**Source File:** `loader/java.c`

> (F[I)I -- maxWidth, wwPositions[]. Replica exacta del loop de
> NexusFont.GFA_GetWordwrapPositionEx: mientras el resto no entre en
> maxWidth, corta en breakText(maxWidth) caracteres, acumula la posicion y
> la escribe en wwPositions. Devuelve la cantidad de cortes.

---

## `java.c` (line ~670) (line ~670)

**Source File:** `loader/java.c`

> (F[I)I -- maxWidth, wwPositions[]. Replica exacta del loop de
> NexusFont.GFA_GetWordwrapPositionEx: mientras el resto no entre en
> maxWidth, corta en breakText(maxWidth) caracteres, acumula la posicion y
> la escribe en wwPositions. Devuelve la cantidad de cortes.

---

## `java.c` (line ~679) (line ~679)

**Source File:** `loader/java.c`

> Igual que NexusFont.GFA_CharHeight(): literalmente el tamano de texto
> actual, sin redondeo especial.

---

## `Zenonia_GFA_SetString` (line ~706)

**Source File:** `loader/java.c`

> (Ljava/lang/String;I)V -- string (jstring = char* UTF-8 crudo en FalsoJNI),
> nChars (0 = largo completo). Java: g_strConv = string.substring(0, nChars).

---

## `Zenonia_GFA_SetStringFromKSC5601` (line ~719)

**Source File:** `loader/java.c`

> ([B)V -- el parametro llega como un jbyteArray real (alocado por el motor
> via NewByteArray + SetByteArrayRegion, confirmado en el log) -- en FalsoJNI
> eso es un JavaDynArray*, no un puntero crudo (a diferencia de jstring, que
> FalsoJNI SI pasa como char* crudo -- no confundir los dos casos).
> Java: new String(data, "KSC5601") -- EUC-KR via tabla generada (cp949).

---

## `Zenonia_GFA_SetStringFromUnicode` (line ~735)

**Source File:** `loader/java.c`

> Java: new String(data, "UTF-16LE").

---

## `gfa_clear_canvas` (line ~759)

**Source File:** `loader/java.c`

> Limpia el canvas GFA (equivalente al clear de g_gfaIntBuf en Java).

---

## `f` (line ~770)

**Source File:** `loader/java.c`

> Limpia el canvas GFA (equivalente al clear de g_gfaIntBuf en Java).

---

## `x` (line ~801)

**Source File:** `loader/java.c`

> ()[F -- NexusFont.GFA_DrawFont(): limpia el bitmap, dibuja el string actual
> en (0, charH - descent + 1) y devuelve {0, 0, anchoMedido, charH + 1}.
> El motor luego pide GFA_GetPixels32 y usa ceil(rect[2]) x ceil(rect[3])
> pixeles con SOLO el canal alfa (drawCharToCharCacheBuffer).

---

## `maxWidth` (line ~861)

**Source File:** `loader/java.c`

> (IF)[F -- nChars, maxWidth -> {anchoMax, altoTotal}. Mismo loop que
> DrawText pero solo midiendo (NexusFont.GFA_MeasureText).

---

## `java.c` (line ~908) (line ~908)

**Source File:** `loader/java.c`

> GFA_GetPixels32/16 SI son seguros por construccion aunque devuelvan un
> array vacio: el codigo nativo que los llama copia via
> GetArrayLength+GetIntArrayRegion/GetShortArrayRegion (no desreferencia un
> puntero crudo como GFA_DrawFont/DrawText/MeasureText) -- confirmado
> leyendo el desensamblado de GFA_GetPixels32/16 en out_ghidra.c. Se
> registran igual, con un buffer real (en cero, sin rasterizado todavia) en
> vez de depender de que ese camino tolere un array NULL/vacio sin probarlo.
> Los pixeles ya quedaron rasterizados en el buffer por el ultimo
> GFA_DrawFont/DrawText -- aca solo se devuelve el array (el motor copia con
> GetIntArrayRegion y usa el canal alfa).

---

## `count` (line ~918)

**Source File:** `loader/java.c`

> GFA_GetPixels32/16 SI son seguros por construccion aunque devuelvan un
> array vacio: el codigo nativo que los llama copia via
> GetArrayLength+GetIntArrayRegion/GetShortArrayRegion (no desreferencia un
> puntero crudo como GFA_DrawFont/DrawText/MeasureText) -- confirmado
> leyendo el desensamblado de GFA_GetPixels32/16 en out_ghidra.c. Se
> registran igual, con un buffer real (en cero, sin rasterizado todavia) en
> vez de depender de que ese camino tolere un array NULL/vacio sin probarlo.
> Los pixeles ya quedaron rasterizados en el buffer por el ultimo
> GFA_DrawFont/DrawText -- aca solo se devuelve el array (el motor copia con
> GetIntArrayRegion y usa el canal alfa).

---
