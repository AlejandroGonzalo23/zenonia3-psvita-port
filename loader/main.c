/**
 * @brief main.c ARMv7 Shared Libraries loader. Zenonia 3 (Gamevil Nexus2).
 */
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "debugScreen.h"
#include "so_util.h"
#include "audio.h"
#include "androidui.h"
#include <taihen.h>
#include <vitaGL.h>
#include <falso_jni/FalsoJNI.h>

int kuKernelCpuUnrestrictedMemcpy(void *dst, const void *src, SceSize size);

#define printf psvDebugScreenPrintf
#define LOG_DIR "ux0:data/zenonia3/logs"

#define GAME_W 800
#define GAME_H 480
#define SCREEN_W 960
#define SCREEN_H 544

FILE *log_file = NULL;
int gl_active = 0;

int _newlib_heap_size_user = 128 * 1024 * 1024;
unsigned int sceLibcHeapSize = 4 * 1024 * 1024;

void init_log() {
    sceIoMkdir(LOG_DIR, 0777);

    char log_path[256];
    time_t t = time(NULL);
    snprintf(log_path, sizeof(log_path), LOG_DIR "/log_%u.txt", (unsigned int) t);

    log_file = fopen(log_path, "w");
    if (log_file) {
        fprintf(log_file, "--- ZENONIA 3 PORT LOG START (%s) ---\n", log_path);
        fflush(log_file);
    }
}

void game_log(const char *fmt, ...) {
    va_list list;
    char string[512];

    va_start(list, fmt);
    vsnprintf(string, sizeof(string), fmt, list);
    va_end(list);

    if (log_file) {
        fprintf(log_file, "%s", string);
        fflush(log_file);
    }
}

void fatal_error(const char *fmt, ...) {
    va_list list;
    char string[512];

    va_start(list, fmt);
    vsnprintf(string, sizeof(string), fmt, list);
    va_end(list);

    game_log("[FATAL] %s\n", string);
    psvDebugScreenInit();
    printf("[FATAL] %s\n", string);
    sceKernelDelayThread(10 * 1000 * 1000);
    sceKernelExitProcess(0);
}

so_module zenonia3_mod;

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    (void)prio;
    va_list list;
    char string[512];

    va_start(list, fmt);
    vsnprintf(string, sizeof(string), fmt, list);
    va_end(list);

    game_log("[ANDROID] %s: %s\n", tag, string);
    return 0;
}

extern so_default_dynlib default_dynlib[];
extern int default_dynlib_size;

int (* Game_JNI_OnLoad)(void *vm, void *reserved);
void (* NativeRender)(void *env, void *obj) = NULL;
void (* NativeResize)(void *env, void *obj, int w, int h) = NULL;
void (* NativeResumeClet)(void *env, void *obj) = NULL;
void (* handleCletEvent)(void *env, void *obj, int type, int p1, int p2, int p3) = NULL;

#define MH_KEY_PRESSEVENT        2
#define MH_KEY_RELEASEEVENT      3
#define MH_POINTER_PRESSEVENT    23
#define MH_POINTER_RELEASEEVENT  24
#define MH_POINTER_MOVEEVENT     25

#define HAL_KEY_UP    (-1)
#define HAL_KEY_DOWN  (-2)
#define HAL_KEY_LEFT  (-3)
#define HAL_KEY_RIGHT (-4)
#define HAL_KEY_OK    (-5)
#define HAL_KEY_MAP   (-6)
#define HAL_KEY_SAVE  (-10)
#define HAL_KEY_BACK  (-16)
#define HAL_KEY_SKIP  (35)

#define NEXUS_HAL_REPLY_YESNO           19450815
#define NEXUS_HAL_FIRST_MOVE_REPLY_PAGE 20010913
#define NEXUS_HAL_YES_MOVE_REPLY_PAGE   20010911
#define NEXUS_HAL_NO_MOVE_REPLY_PAGE    20010912

#define UI_STATUS_EXIT 104

typedef struct { int type, p1, p2, p3; } input_event;
static input_event event_queue[16];
static int eq_head = 0, eq_tail = 0;

static void queue_input_event(int type, int p1, int p2, int p3) {
    int next = (eq_tail + 1) % 16;
    if (next != eq_head) {
        event_queue[eq_tail].type = type;
        event_queue[eq_tail].p1 = p1;
        event_queue[eq_tail].p2 = p2;
        event_queue[eq_tail].p3 = p3;
        eq_tail = next;
    }
}

static const struct { unsigned int btn; int hal; } btn_map[] = {
    { SCE_CTRL_UP,       HAL_KEY_UP },
    { SCE_CTRL_DOWN,     HAL_KEY_DOWN },
    { SCE_CTRL_LEFT,     HAL_KEY_LEFT },
    { SCE_CTRL_RIGHT,    HAL_KEY_RIGHT },
    { SCE_CTRL_CROSS,    HAL_KEY_OK },
    { SCE_CTRL_CIRCLE,   HAL_KEY_BACK },
    { SCE_CTRL_TRIANGLE, HAL_KEY_SKIP },
    { SCE_CTRL_SQUARE,   HAL_KEY_MAP },
    { SCE_CTRL_LTRIGGER, HAL_KEY_SAVE },
};
#define BTN_MAP_COUNT (sizeof(btn_map) / sizeof(btn_map[0]))

extern volatile int g_ui_status;
extern void zenonia_install_array_hooks(void);
extern void zenonia_install_hide_dpad_hook(so_module *mod);

static GLuint splash_tex = 0;

static void splash_load(void) {
    FILE *f = fopen("app0:splash.rgba", "rb");
    if (!f) return;
    void *data = malloc(960 * 544 * 4);
    if (!data) { fclose(f); return; }
    fread(data, 1, 960 * 544 * 4, f);
    fclose(f);

    glGenTextures(1, &splash_tex);
    glBindTexture(GL_TEXTURE_2D, splash_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 960, 544, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    free(data);
}

static void splash_draw(void) {
    if (!splash_tex) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrthof(0, 960, 544, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, splash_tex);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    static const float verts[] = { 0, 0,  960, 0,  0, 544,  960, 544 };
    static const float uvs[]   = { 0, 0,  1, 0,    0, 1,    1, 1 };
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void gl_init() {
    vglUseTripleBuffering(GL_FALSE);
    vglInitExtended(0, 960, 544, 16 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE);
#ifdef LOCK_FPS_30
    vglWaitVblankStart(GL_FALSE);
#endif
    gl_active = 1;
}

void raise_clocks() {
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
}

int main() {
    raise_clocks();
    init_log();
    game_log("Iniciando Zenonia 3 port (SoLoader)\n");

    int res = so_file_load(&zenonia3_mod, "ux0:data/zenonia3/libgameDSO.so", 0x98000000);
    if (res < 0) {
        game_log("Error critico cargando libgameDSO.so: 0x%08X\n", res);
        sceKernelDelayThread(5000000);
        return 0;
    }

    game_log("Libreria cargada con exito.\n");
    so_relocate(&zenonia3_mod);
    so_resolve(&zenonia3_mod, default_dynlib, default_dynlib_size, 0);

    uint32_t text_base = zenonia3_mod.text_base;
    if (*(uint16_t *)(text_base + 0x9a7b4) == 0xdd24) {
        uint16_t patch_beq = 0xd024;
        kuKernelCpuUnrestrictedMemcpy((void *)(text_base + 0x9a7b4), &patch_beq, 2);
        game_log("Parche aplicado: CMvLayerData::PreLoad (ble -> beq)\n");
    }
    so_flush_caches(&zenonia3_mod);
    so_initialize(&zenonia3_mod);

    game_log("SoLoader inicializado. Iniciando vitaGL...\n");
    gl_init();
    game_log("vitaGL inicializado.\n");
    audio_init();
    splash_load();
    androidui_load(SCREEN_W, SCREEN_H);

    jni_init();

    Game_JNI_OnLoad = (void *)so_symbol(&zenonia3_mod, "JNI_OnLoad");
    NativeRender = (void *)so_symbol(&zenonia3_mod, "Java_com_gamevil_nexus2_Natives_NativeRender");
    NativeResize = (void *)so_symbol(&zenonia3_mod, "Java_com_gamevil_nexus2_Natives_NativeResize");
    NativeResumeClet = (void *)so_symbol(&zenonia3_mod, "Java_com_gamevil_nexus2_Natives_NativeResumeClet");
    handleCletEvent = (void *)so_symbol(&zenonia3_mod, "Java_com_gamevil_nexus2_Natives_handleCletEvent");

    zenonia_install_hide_dpad_hook(&zenonia3_mod);

    game_log("Llamando JNI_OnLoad...\n");
    if (Game_JNI_OnLoad) {
        Game_JNI_OnLoad((void *)jvm, NULL);
    }

    game_log("Simbolos: Render=%p Resize=%p Resume=%p CletEvent=%p\n",
        (void*)NativeRender, (void*)NativeResize, (void*)NativeResumeClet, (void*)handleCletEvent);

    game_log("Iniciando Bucle Principal...\n");

    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    SceCtrlData pad;
    SceTouchData touch;
    int last_touch = 0;
    int last_touch_suppressed = 0;
    int last_tx = 0, last_ty = 0;
    unsigned int old_buttons = 0;
    int frame = 0;
    int engine_started = 0;

    int fps_count = 0;
    SceUInt64 fps_window_start = sceKernelGetProcessTimeWide();

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

        if (!engine_started) {
            engine_started = 1;
            // Configurar viewport del motor a la resolucion nativa soportada por Nexus2 (800x480)
            if (NativeResize) {
                game_log("Ejecutando NativeResize(%d,%d)...\n", GAME_W, GAME_H);
                NativeResize((void *)jni, NULL, GAME_W, GAME_H);
            }
            if (NativeResumeClet) {
                game_log("Ejecutando NativeResumeClet...\n");
                NativeResumeClet((void *)jni, NULL);
            }
        }

        if ((frame++ % 120) == 0) {
            game_log("frame %d alive, pad.buttons=0x%08x ui_status=%d\n", frame, (unsigned int) pad.buttons, g_ui_status);
        }

        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT)) break;
        if (g_ui_status == UI_STATUS_EXIT) break;

        unsigned int pressed = pad.buttons & ~old_buttons;
        unsigned int released = old_buttons & ~pad.buttons;

        if (g_ui_status == 4 || g_ui_status == 5) {
            if (pad.buttons & SCE_CTRL_UP)   androidui_scroll_info_text(g_ui_status, -8.0f, SCREEN_W, SCREEN_H);
            if (pad.buttons & SCE_CTRL_DOWN) androidui_scroll_info_text(g_ui_status,  8.0f, SCREEN_W, SCREEN_H);
            pressed  &= ~(SCE_CTRL_UP | SCE_CTRL_DOWN);
            released &= ~(SCE_CTRL_UP | SCE_CTRL_DOWN);
        }

        for (int i = 0; i < BTN_MAP_COUNT; i++) {
            if (pressed & btn_map[i].btn)
                queue_input_event(MH_KEY_PRESSEVENT, btn_map[i].hal, 0, 0);
            if (released & btn_map[i].btn)
                queue_input_event(MH_KEY_RELEASEEVENT, btn_map[i].hal, 0, 0);
        }
        old_buttons = pad.buttons;

        if (touch.reportNum > 0) {
            int x = touch.report[0].x * SCREEN_W / 1920;
            int y = touch.report[0].y * SCREEN_H / 1088;

            if (!last_touch) {
                int sx = x;
                int sy = y;

                if (g_ui_status == 1) {
                    queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_BACK, 0, 0);
                    queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_BACK, 0, 0);
                    last_touch_suppressed = 1;
                } else if (g_ui_status == 2) {
                    androidui_menu_hit hit = androidui_menu_hit_test((float) sx, (float) sy, SCREEN_W, SCREEN_H);
                    switch (hit) {
                        case ANDROIDUI_MENU_HIT_COMMUNITY:
                            queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_LEFT, 0, 0);
                            queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_LEFT, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        case ANDROIDUI_MENU_HIT_OPTIONS:
                            queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_UP, 0, 0);
                            queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_UP, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        case ANDROIDUI_MENU_HIT_NEWGAME:
                            queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_OK, 0, 0);
                            queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_OK, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        case ANDROIDUI_MENU_HIT_CONTINUE:
                            queue_input_event(NEXUS_HAL_REPLY_YESNO, NEXUS_HAL_FIRST_MOVE_REPLY_PAGE, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        case ANDROIDUI_MENU_HIT_HELP:
                            queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_DOWN, 0, 0);
                            queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_DOWN, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        case ANDROIDUI_MENU_HIT_ABOUT:
                            queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_RIGHT, 0, 0);
                            queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_RIGHT, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        default:
                            last_touch_suppressed = 0;
                            break;
                    }
                } else if (g_ui_status == 5 || g_ui_status == 4) {
                    androidui_backbtn_hit hit = androidui_backbtn_hit_test((float) sx, (float) sy, SCREEN_W, SCREEN_H);
                    if (hit == ANDROIDUI_BACKBTN_HIT_BACK) {
                        queue_input_event(MH_KEY_PRESSEVENT, HAL_KEY_BACK, 0, 0);
                        queue_input_event(MH_KEY_RELEASEEVENT, HAL_KEY_BACK, 0, 0);
                        last_touch_suppressed = 1;
                    } else {
                        last_touch_suppressed = 0;
                    }
                } else if (g_ui_status == 5000) {
                    androidui_reply_hit hit = androidui_reply_hit_test((float) sx, (float) sy, SCREEN_W, SCREEN_H);
                    switch (hit) {
                        case ANDROIDUI_REPLY_HIT_WRITE:
                            queue_input_event(NEXUS_HAL_REPLY_YESNO, NEXUS_HAL_YES_MOVE_REPLY_PAGE, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        case ANDROIDUI_REPLY_HIT_LATER:
                            queue_input_event(NEXUS_HAL_REPLY_YESNO, NEXUS_HAL_NO_MOVE_REPLY_PAGE, 0, 0);
                            last_touch_suppressed = 1;
                            break;
                        default:
                            last_touch_suppressed = 0;
                            break;
                    }
                } else {
                    last_touch_suppressed = 0;
                }

                if (!last_touch_suppressed) {
                    queue_input_event(MH_POINTER_PRESSEVENT, x, y, 0);
                }
                last_touch = 1;
            } else if (!last_touch_suppressed && (x != last_tx || y != last_ty)) {
                queue_input_event(MH_POINTER_MOVEEVENT, x, y, 0);
            }
            last_tx = x; last_ty = y;
        } else if (last_touch) {
            if (!last_touch_suppressed) {
                queue_input_event(MH_POINTER_RELEASEEVENT, last_tx, last_ty, 0);
            }
            last_touch = 0;
            last_touch_suppressed = 0;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (handleCletEvent && eq_head != eq_tail) {
            input_event *ev = &event_queue[eq_head];
            eq_head = (eq_head + 1) % 16;
            handleCletEvent((void *)jni, NULL, ev->type, ev->p1, ev->p2, ev->p3);
        }

        if (NativeRender) NativeRender((void *)jni, NULL);

        if (g_ui_status >= 0 && g_ui_status <= 1) splash_draw();

        androidui_draw(g_ui_status, SCREEN_W, SCREEN_H);

#ifdef LOCK_FPS_30
        sceDisplayWaitVblankStartMulti(2);
#endif
        vglSwapBuffers(GL_FALSE);

        fps_count++;
        SceUInt64 now = sceKernelGetProcessTimeWide();
        if (now - fps_window_start >= 2000000) {
            double secs = (now - fps_window_start) / 1000000.0;
            game_log("[PERF] FPS=%.1f (frames=%d en %.2fs)\n", fps_count / secs, fps_count, secs);
            fps_count = 0;
            fps_window_start = now;
        }
    }

    if (log_file) fclose(log_file);
    sceKernelExitProcess(0);
    return 0;
}
