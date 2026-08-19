# `loader/androidui.h` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `androidui_load` (line ~4)

**Source File:** `loader/androidui.h`

> Que boton del menu (si alguno) cae bajo (sx,sy) -- coordenadas de PANTALLA
> (mismo espacio 0..screen_w/0..screen_h que androidui_draw(), NO el espacio
> de juego 400x240 que usa el resto del touch en main.c). Solo tiene sentido
> llamarlo con ui_status==2 (MAINMENU). Ver la nota en main.c: en el APK
> original estos son ImageButton de Android que consumen el toque ANTES de
> que llegue al GLSurfaceView -- por eso hace falta interceptarlos aca en
> vez de dejar que main.c reenvie el toque crudo al motor.

---

## `androidui.h` (line ~12) (line ~12)

**Source File:** `loader/androidui.h`

> Que boton del menu (si alguno) cae bajo (sx,sy) -- coordenadas de PANTALLA
> (mismo espacio 0..screen_w/0..screen_h que androidui_draw(), NO el espacio
> de juego 400x240 que usa el resto del touch en main.c). Solo tiene sentido
> llamarlo con ui_status==2 (MAINMENU). Ver la nota en main.c: en el APK
> original estos son ImageButton de Android que consumen el toque ANTES de
> que llegue al GLSurfaceView -- por eso hace falta interceptarlos aca en
> vez de dejar que main.c reenvie el toque crudo al motor.

---

## `androidui.h` (line ~29) (line ~29)

**Source File:** `loader/androidui.h`

> ui_status==5000 (UI_STATUS_REPLY_PAGE): popup de "valorar la app" que
> Natives.showReplyMoveComponent() muestra al tocar "Continuar" en el menu
> (ver Natives.java showReplyMoveComponent()/hideReplyMoveComponent() -- 2
> ImageButton reales, reply_write_id/reply_later_id, no un WebView).

---

## `androidui_scroll_info_text` (line ~42)

**Source File:** `loader/androidui.h`

> ui_status==5000 (UI_STATUS_REPLY_PAGE): popup de "valorar la app" que
> Natives.showReplyMoveComponent() muestra al tocar "Continuar" en el menu
> (ver Natives.java showReplyMoveComponent()/hideReplyMoveComponent() -- 2
> ImageButton reales, reply_write_id/reply_later_id, no un WebView).

---

## `androidui_scroll_info_text` (line ~54)

**Source File:** `loader/androidui.h`

> ui_status==5000 (UI_STATUS_REPLY_PAGE): popup de "valorar la app" que
> Natives.showReplyMoveComponent() muestra al tocar "Continuar" en el menu
> (ver Natives.java showReplyMoveComponent()/hideReplyMoveComponent() -- 2
> ImageButton reales, reply_write_id/reply_later_id, no un WebView).

---
