/**
 * @file java.c
 * @brief Standalone JNI environment and callbacks for Zenonia 3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <falso_jni/FalsoJNI.h>
#include "so_util.h"

extern void game_log(const char *fmt, ...);

// Punteros a las funciones del motor libgameDSO.so (definidos en main.c)
extern void (* NativeInitDeviceInfo)(void *env, void *obj, int w, int h);
extern void (* NativeInitWithBufferSize)(void *env, void *obj, int w, int h);
extern void (* NativeRender)(void *env, void *obj);
extern void (* NativeResize)(void *env, void *obj, int w, int h);
extern void (* NativeResumeClet)(void *env, void *obj);
extern void (* handleCletEvent)(void *env, void *obj, int type, int p1, int p2, int p3);

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

/* --- Stubs de la interfaz JNIEnv para la JVM --- */

static jclass JNICALL Zenonia_FindClass(JNIEnv *env, const char *name) {
    (void)env; (void)name;
    return (jclass)0x1;
}

static jmethodID JNICALL Zenonia_GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    (void)env; (void)clazz; (void)name; (void)sig;
    return (jmethodID)0x1;
}

static jmethodID JNICALL Zenonia_GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    (void)env; (void)clazz; (void)name; (void)sig;
    return (jmethodID)0x1;
}

static jstring JNICALL Zenonia_NewStringUTF(JNIEnv *env, const char *bytes) {
    (void)env;
    return (jstring)bytes;
}

static const char* JNICALL Zenonia_GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy) {
    (void)env;
    if (isCopy) *isCopy = JNI_FALSE;
    return (const char *)string;
}

static void JNICALL Zenonia_ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *utf) {
    (void)env; (void)string; (void)utf;
}

static jint JNICALL Zenonia_GetEnv(JavaVM *vm, void **env, jint version) {
    (void)vm; (void)version;
    if (env) {
        *env = (void *)jni;
    }
    return JNI_OK;
}

static struct JNINativeInterface zenonia_jni_native_interface = {
    .FindClass = Zenonia_FindClass,
    .GetMethodID = Zenonia_GetMethodID,
    .GetStaticMethodID = Zenonia_GetStaticMethodID,
    .NewStringUTF = Zenonia_NewStringUTF,
    .GetStringUTFChars = Zenonia_GetStringUTFChars,
    .ReleaseStringUTFChars = Zenonia_ReleaseStringUTFChars,
    .GetArrayLength = Zenonia_GetArrayLength,
    .GetByteArrayElements = Zenonia_GetByteArrayElements,
    .ReleaseByteArrayElements = Zenonia_ReleaseByteArrayElements,
    .GetFloatArrayElements = Zenonia_GetFloatArrayElements,
    .ReleaseFloatArrayElements = Zenonia_ReleaseFloatArrayElements,
    .RegisterNatives = Zenonia_RegisterNatives,
};

static struct JNIInvokeInterface zenonia_jvm_interface = {
    .GetEnv = Zenonia_GetEnv,
};

JNIEnv jni = &zenonia_jni_native_interface;
JavaVM jvm = &zenonia_jvm_interface;

void zenonia_install_array_hooks(void) {
    game_log("[JNI] Entorno JNI configurado\n");
}

void jni_init(void) {
    game_log("[JNI] Inicializado entorno JNI personalizado.\n");
}
