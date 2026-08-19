# `loader/htmltext.h` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `HTMLTEXT_STYLE_HEADER` (line ~4)

**Source File:** `loader/htmltext.h`

> Extractor de texto plano para los 2 HTML reales del APK original que
> Android mostraba en un WebView (about.html/help_eng.html, ver
> zenonia3/assets/html/) -- NO es un parser HTML general: alcanza con lo que
> usan esas 2 paginas (tablas simples <tr><td>texto</td></tr>, algunos
> <span>/<img>/<p> inline, un puñado de entidades basicas). Cada </td>,
> </tr>, </p> o <br> con contenido visible antes cierra una linea (\n) en la
> salida; las celdas vacias (<td height="5"></td>, usadas como espaciador
> decorativo en el HTML original) no generan lineas en blanco. Bytes no-ASCII
> (bullets en euc-kr/mojibake del HTML original) se descartan -- estas 2
> paginas son basicamente texto en ingles llano.
>
> Marcadores de estilo (1 byte de control, NO imprimible) antepuestos a una
> linea de salida cuando la celda de origen tiene class="style3" (encabezado
> de seccion, ej. "Credits"/"Game Introduction" -- CSS real: negrita, color
> #00b0f0, con regla horizontal arriba/abajo) o class="style2b" (etiqueta,
> ej. "Executive Producer" -- CSS real: negrita, color #FD9E35). Sin marcador
> = texto de cuerpo normal (class="style2"). htmlview.c los interpreta para
> elegir color y dibujar las reglas -- se quitan del texto antes de rasterizar.

---

## `htmltext_extract` (line ~12)

**Source File:** `loader/htmltext.h`

> Extractor de texto plano para los 2 HTML reales del APK original que
> Android mostraba en un WebView (about.html/help_eng.html, ver
> zenonia3/assets/html/) -- NO es un parser HTML general: alcanza con lo que
> usan esas 2 paginas (tablas simples <tr><td>texto</td></tr>, algunos
> <span>/<img>/<p> inline, un puñado de entidades basicas). Cada </td>,
> </tr>, </p> o <br> con contenido visible antes cierra una linea (\n) en la
> salida; las celdas vacias (<td height="5"></td>, usadas como espaciador
> decorativo en el HTML original) no generan lineas en blanco. Bytes no-ASCII
> (bullets en euc-kr/mojibake del HTML original) se descartan -- estas 2
> paginas son basicamente texto en ingles llano.
>
> Marcadores de estilo (1 byte de control, NO imprimible) antepuestos a una
> linea de salida cuando la celda de origen tiene class="style3" (encabezado
> de seccion, ej. "Credits"/"Game Introduction" -- CSS real: negrita, color
> #00b0f0, con regla horizontal arriba/abajo) o class="style2b" (etiqueta,
> ej. "Executive Producer" -- CSS real: negrita, color #FD9E35). Sin marcador
> = texto de cuerpo normal (class="style2"). htmlview.c los interpreta para
> elegir color y dibujar las reglas -- se quitan del texto antes de rasterizar.

---
