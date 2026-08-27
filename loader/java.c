/**
 * @file java.c
 * @brief JNI callbacks and FalsoJNI hooks for Zenonia 3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <falso_jni/FalsoJNI.h>
#include "so_util.h"

extern void game_log(const char *fmt, ...);

// Punteros a las funciones del motor libgameDSO.so
void (* NativeInitDeviceInfo)(void *env, void *obj, int w, int h) = NULL;
void (* NativeInitWithBufferSize)(void *env, void *obj, int w, int h) = NULL;
void (* NativeRender)(void *env, void *obj) = NULL;
void (* NativeResize)(void *env, void *obj, int w, int h) = NULL;
void (* NativeResumeClet)(void *env, void *obj) = NULL;
void (* handleCletEvent)(void *env, void *obj, int type, int p1, int p2, int p3) = NULL;

volatile int g_ui_status = 0;

/* --- Arrays dinamicos persistentes --- */

#define FIELD_TYPE_FLOAT 1
#define FIELD_TYPE_BYTE  2
#define FIELD_TYPE_INT   3

typedef struct {
    int length;
    int type;
    void *elements;
} JavaDynArray;

static JavaDynArray *jda_alloc(int length, int type) {
    JavaDynArray *jda = malloc(sizeof(JavaDynArray));
    jda->length = length;
    jda->type = type;
    size_t sz = sizeof(int);
    if (type == FIELD_TYPE_FLOAT) sz = sizeof(float);
    else if (type == FIELD_TYPE_BYTE) sz = sizeof(uint8_t);
    jda->elements = calloc(length, sz);
    return jda;
}

static JavaDynArray *gfa_persistent_floats(JavaDynArray **slot, int len) {
    if (!*slot) {
        *slot = jda_alloc(len, FIELD_TYPE_FLOAT);
    }
    return *slot;
}

/* --- Intercepcion de RegisterNatives durante JNI_OnLoad --- */

static jint JNICALL Zenonia_RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods) {
    (void)env;
    (void)clazz;
    game_log("[JNI] RegisterNatives llamado con %d metodos\n", nMethods);
    for (int i = 0; i < nMethods; i++) {
        game_log("[JNI] Metodo: %s (sig: %s) -> %p\n", methods[i].name, methods[i].signature, methods[i].fnPtr);

        if (strstr(methods[i].name, "InitDeviceInfo") || strstr(methods[i].name, "initDeviceInfo")) {
            NativeInitDeviceInfo = methods[i].fnPtr;
        } else if (strstr(methods[i].name, "InitWithBufferSize") || strstr(methods[i].name, "initWithBufferSize")) {
            NativeInitWithBufferSize = methods[i].fnPtr;
        } else if (strstr(methods[i].name, "Render") || strcmp(methods[i].name, "render") == 0) {
            NativeRender = methods[i].fnPtr;
        } else if (strstr(methods[i].name, "Resize") || strcmp(methods[i].name, "resize") == 0) {
            NativeResize = methods[i].fnPtr;
        } else if (strstr(methods[i].name, "ResumeClet") || strcmp(methods[i].name, "resumeClet") == 0) {
            NativeResumeClet = methods[i].fnPtr;
        } else if (strstr(methods[i].name, "handleCletEvent")) {
            handleCletEvent = methods[i].fnPtr;
        }
    }
    return JNI_OK;
}

/* --- Callbacks llamados por libgameDSO hacia Java --- */

void Java_com_gamevil_nexus2_Natives_showTitleComponent(JNIEnv *env, jobject obj, jint status) {
    (void)env; (void)obj;
    game_log("[JNI] showTitleComponent status=%d\n", status);
    g_ui_status = status;
}

void Java_com_gamevil_nexus2_Natives_showReplyMoveComponent(JNIEnv *env, jobject obj, jint status) {
    (void)env; (void)obj;
    game_log("[JNI] showReplyMoveComponent status=%d\n", status);
    g_ui_status = status;
}

void Java_com_gamevil_nexus2_Natives_vibrate(JNIEnv *env, jobject obj, jint ms) {
    (void)env; (void)obj; (void)ms;
}

void Java_com_gamevil_nexus2_Natives_playBGM(JNIEnv *env, jobject obj, jstring path) {
    (void)env; (void)obj; (void)path;
}

void Java_com_gamevil_nexus2_Natives_stopBGM(JNIEnv *env, jobject obj) {
    (void)env; (void)obj;
}

void Java_com_gamevil_nexus2_Natives_playSE(JNIEnv *env, jobject obj, jint sound_id) {
    (void)env; (void)obj; (void)sound_id;
}

jboolean Java_com_gamevil_nexus2_Natives_isNetworkConnected(JNIEnv *env, jobject obj) {
    (void)env; (void)obj;
    return JNI_FALSE;
}

static JavaDynArray *g_font_measure_slot = NULL;
static JavaDynArray *g_font_draw_slot = NULL;

jobject Java_com_gamevil_nexus2_Natives_GFA_1DrawFont(JNIEnv *env, jobject obj) {
    (void)env; (void)obj;
    return (jobject)gfa_persistent_floats(&g_font_draw_slot, 4);
}

jobject Java_com_gamevil_nexus2_Natives_GFA_1DrawText(JNIEnv *env, jobject obj) {
    (void)env; (void)obj;
    return (jobject)gfa_persistent_floats(&g_font_draw_slot, 4);
}

jobject Java_com_gamevil_nexus2_Natives_GFA_1MeasureText(JNIEnv *env, jobject obj) {
    (void)env; (void)obj;
    return (jobject)gfa_persistent_floats(&g_font_measure_slot, 2);
}

jbyteArray Java_com_gamevil_nexus2_Natives_readAssets(JNIEnv *env, jobject obj, jstring path_str) {
    (void)env; (void)obj;
    if (!path_str) return NULL;

    const char *path = (const char *)path_str;
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "ux0:data/zenonia3/assets/%s", path);
    FILE *f = fopen(full_path, "rb");
    if (!f) {
        game_log("[JNI] readAssets no encontrado: %s\n", full_path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    JavaDynArray *arr = jda_alloc(sz, FIELD_TYPE_BYTE);
    fread(arr->elements, 1, sz, f);
    fclose(f);

    return (jbyteArray)arr;
}

/* --- Hooks de gestión de Arrays en la tabla JNIEnv --- */

static jsize JNICALL Zenonia_GetArrayLength(JNIEnv *env, jarray array) {
    (void)env;
    if (!array) return 0;
    JavaDynArray *arr = (JavaDynArray *)array;
    return arr->length;
}

static jbyte* JNICALL Zenonia_GetByteArrayElements(JNIEnv *env, jbyteArray array, jboolean *isCopy) {
    (void)env;
    if (isCopy) *isCopy = JNI_FALSE;
    if (!array) return NULL;
    JavaDynArray *arr = (JavaDynArray *)array;
    return (jbyte *)arr->elements;
}

static void JNICALL Zenonia_ReleaseByteArrayElements(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode) {
    (void)env; (void)array; (void)elems; (void)mode;
}

static jfloat* JNICALL Zenonia_GetFloatArrayElements(JNIEnv *env, jfloatArray array, jboolean *isCopy) {
    (void)env;
    if (isCopy) *isCopy = JNI_FALSE;
    if (!array) return NULL;
    JavaDynArray *arr = (JavaDynArray *)array;
    return (jfloat *)arr->elements;
}

static void JNICALL Zenonia_ReleaseFloatArrayElements(JNIEnv *env, jfloatArray array, jfloat *elems, jint mode) {
    (void)env; (void)array; (void)elems; (void)mode;
}

void zenonia_install_array_hooks(void) {
    struct JNINativeInterface_ *table = (struct JNINativeInterface_ *)jni;
    table->GetArrayLength = Zenonia_GetArrayLength;
    table->GetByteArrayElements = Zenonia_GetByteArrayElements;
    table->ReleaseByteArrayElements = Zenonia_ReleaseByteArrayElements;
    table->GetFloatArrayElements = Zenonia_GetFloatArrayElements;
    table->ReleaseFloatArrayElements = Zenonia_ReleaseFloatArrayElements;
    table->RegisterNatives = Zenonia_RegisterNatives;
    game_log("[JNI] Hooks instalados en la tabla JNIEnv\n");
}
