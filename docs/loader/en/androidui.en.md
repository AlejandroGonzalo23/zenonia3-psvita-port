# `loader/androidui.h` — Design Architecture & Notes

Explanatory and architectural design notes extracted from source code and replaced with concise technical Doxygen blocks. This document preserves the reasoning ('why') separated from technical API documentation.

## `androidui_load` (line ~4)

**Source File:** `loader/androidui.h`

> Which menu button (if any) falls under (sx,sy) -- SCREEN coordinates
> (same space 0..screen_w/0..screen_h as androidui_draw(), NOT the space
> 400x240 game that uses the rest of the touch in main.c). It only makes sense
> call it with ui_status==2 (MAINMENU). See the note in main.c: in the APK
>original these are Android ImageButton that consume the touch BEFORE
> that reaches the GLSurfaceView -- that is why it is necessary to intercept them here in
> instead of letting main.c forward the raw touch to the engine.

---

## `androidui.h` (line ~12) (line ~12)

**Source File:** `loader/androidui.h`

> Which menu button (if any) falls under (sx,sy) -- SCREEN coordinates
> (same space 0..screen_w/0..screen_h as androidui_draw(), NOT the space
> 400x240 game that uses the rest of the touch in main.c). It only makes sense
> call it with ui_status==2 (MAINMENU). See the note in main.c: in the APK
>original these are Android ImageButton that consume the touch BEFORE
> that reaches the GLSurfaceView -- that is why it is necessary to intercept them here in
> instead of letting main.c forward the raw touch to the engine.

---

## `androidui.h` (line ~29) (line ~29)

**Source File:** `loader/androidui.h`

> ui_status==5000 (UI_STATUS_REPLY_PAGE): "rate the app" popup that
> Natives.showReplyMoveComponent() shows when you tap "Continue" in the menu
> (see Natives.java showReplyMoveComponent()/hideReplyMoveComponent() -- 2
> Actual ImageButton, reply_write_id/reply_later_id, not a WebView).

---

## `androidui_scroll_info_text` (line ~42)

**Source File:** `loader/androidui.h`

> ui_status==5000 (UI_STATUS_REPLY_PAGE): "rate the app" popup that
> Natives.showReplyMoveComponent() shows when you tap "Continue" in the menu
> (see Natives.java showReplyMoveComponent()/hideReplyMoveComponent() -- 2
> Actual ImageButton, reply_write_id/reply_later_id, not a WebView).

---

## `androidui_scroll_info_text` (line ~54)

**Source File:** `loader/androidui.h`

> ui_status==5000 (UI_STATUS_REPLY_PAGE): "rate the app" popup that
> Natives.showReplyMoveComponent() shows when you tap "Continue" in the menu
> (see Natives.java showReplyMoveComponent()/hideReplyMoveComponent() -- 2
> Actual ImageButton, reply_write_id/reply_later_id, not a WebView).

---