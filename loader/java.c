/**
 * @file java.c
 * @brief FalsoJNI bindings and JNI callbacks for Zenonia 3 (Gamevil Nexus2 engine).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <falso_jni/FalsoJNI.h>
#include <falso_jni/FalsoJNI_Logger.h>
#include "so_util.h"

extern void game_log(const char *fmt, ...);

// Punteros a las funciones nativas registradas dinamicamente por libgameDSO.so
void (* NativeInitDeviceInfo)(void *env, void *obj, int w, int h) = NULL;
void (* NativeInitWithBufferSize)(void *env, void *obj, int w, int h) = NULL;
void (* NativeRender)(void *env, void *obj) = NULL;
void (* NativeResize)(void *env, void *obj, int w, int h) = NULL;
void (* NativeResumeClet)(void *env, void *obj) = NULL;
void (* handleCletEvent)(void *env, void *obj, int type, int p1, int p2, int p3) = NULL;

volatile int g_ui_status = 0;

/* --- Definicion del array dinamico auxiliar --- */

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

/**
 * @brief Obtiene o crea un array persistente de floats para evitar allocs en cada frame.
 */
static JavaDynArray *gfa_persistent_floats(JavaDynArray **slot, int len) {
    if (!*slot) {
        *slot = jda_alloc(len, FIELD_TYPE_FLOAT);
    }
    return *slot;
}

/* --- Intercepcion de RegisterNatives --- */

static jint JNICALL Zenonia_RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods) {
    (void)env;
    (void)clazz;
    game_log("[JNI] RegisterNatives llamado con %d metodos\n", nMethods);
    for (int i = 0; i < nMethods; i++) {
        game_log("[JNI] Registrando: %s (sig: %s) -> %p\n", methods[i].name, methods[i].signature, methods[i].fnPtr);

        if (strcmp(methods[i].name, "NativeInitDeviceInfo") == 0 || strcmp(methods[i].name, "initDeviceInfo") == 0) {
            NativeInitDeviceInfo = methods[i].fnPtr;
        } else if (strcmp(methods[i].name, "NativeInitWithBufferSize") == 0 || strcmp(methods[i].name, "initWithBufferSize") == 0) {
            NativeInitWithBufferSize = methods[i].fnPtr;
        } else if (strcmp(methods[i].name, "NativeRender") == 0 || strcmp(methods[i].name, "render") == 0) {
            NativeRender = methods[i].fnPtr;
        } else if (strcmp(methods[i].name, "NativeResize") == 0 || strcmp(methods[i].name, "resize") == 0) {
            NativeResize = methods[i].fnPtr;
        } else if (strcmp(methods[i].name, "NativeResumeClet") == 0 || strcmp(methods[i].name, "resumeClet") == 0) {
            NativeResumeClet = methods[i].fnPtr;
        } else if (strcmp(methods[i].name, "handleCletEvent") == 0) {
            handleCletEvent = methods[i].fnPtr;
        }
    }
    return JNI_OK;
}

/* --- Callbacks para clases Java de Nexus2 / Zenonia 3 --- */

// com/gamevil/nexus2/Natives
static void Zenonia_showTitleComponent(jmethodID id, va_list args) {
    int status = va_arg(args, int);
    game_log("[JNI] showTitleComponent status=%d\n", status);
    g_ui_status = status;
}

static void Zenonia_showReplyMoveComponent(jmethodID id, va_list args) {
    int status = va_arg(args, int);
    game_log("[JNI] showReplyMoveComponent status=%d\n", status);
    g_ui_status = status;
}

static void Zenonia_vibrate(jmethodID id, va_list args) {
    int ms = va_arg(args, int);
    (void)ms;
}

static void Zenonia_playBGM(jmethodID id, va_list args) {
    const char *path = va_arg(args, const char *);
    game_log("[JNI] playBGM: %s\n", path ? path : "(null)");
}

static void Zenonia_stopBGM(jmethodID id, va_list args) {
    (void)args;
    game_log("[JNI] stopBGM\n");
}

static void Zenonia_playSE(jmethodID id, va_list args) {
    int sound_id = va_arg(args, int);
    (void)sound_id;
}

static jboolean Zenonia_isNetworkConnected(jmethodID id, va_list args) {
    (void)args;
    return JNI_FALSE;
}

// Interfaz grafica y fuentes (GFA)
static JavaDynArray *g_font_measure_slot = NULL;
static JavaDynArray *g_font_draw_slot = NULL;

static jobject Zenonia_GFA_DrawFont(jmethodID id, va_list args) {
    (void)id;
    (void)args;
    return (jobject)gfa_persistent_floats(&g_font_draw_slot, 4);
}

static jobject Zenonia_GFA_DrawText(jmethodID id, va_list args) {
    (void)id;
    (void)args;
    return (jobject)gfa_persistent_floats(&g_font_draw_slot, 4);
}

static jobject Zenonia_GFA_MeasureText(jmethodID id, va_list args) {
    (void)id;
    (void)args;
    return (jobject)gfa_persistent_floats(&g_font_measure_slot, 2);
}

static jbyteArray Zenonia_readAssets(jmethodID id, va_list args) {
    const char *path = va_arg(args, const char *);
    if (!path) return NULL;

    char full_path[256];
    snprintf(full_path, sizeof(full_path), "ux0:data/zenonia3/assets/%s", path);
    FILE *f = fopen(full_path, "rb");
    if (!f) {
        game_log("[JNI] Assets no encontrado: %s\n", full_path);
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

/* --- Hooks de gestion de Arrays en JNIEnv --- */

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
    (void)env;
    (void)array;
    (void)elems;
    (void)mode;
}

static jfloat* JNICALL Zenonia_GetFloatArrayElements(JNIEnv *env, jfloatArray array, jboolean *isCopy) {
    (void)env;
    if (isCopy) *isCopy = JNI_FALSE;
    if (!array) return NULL;
    JavaDynArray *arr = (JavaDynArray *)array;
    return (jfloat *)arr->elements;
}

static void JNICALL Zenonia_ReleaseFloatArrayElements(JNIEnv *env, jfloatArray array, jfloat *elems, jint mode) {
    (void)env;
    (void)array;
    (void)elems;
    (void)mode;
}

void zenonia_install_array_hooks(void) {
    jni->GetArrayLength = Zenonia_GetArrayLength;
    jni->GetByteArrayElements = Zenonia_GetByteArrayElements;
    jni->ReleaseByteArrayElements = Zenonia_ReleaseByteArrayElements;
    jni->GetFloatArrayElements = Zenonia_GetFloatArrayElements;
    jni->ReleaseFloatArrayElements = Zenonia_ReleaseFloatArrayElements;
    jni->RegisterNatives = Zenonia_RegisterNatives;
    game_log("[JNI] Hooks instalados en JNIEnv\n");
}

void jni_init(void) {
    // Registrar metodos puente con la API estandar de FalsoJNI
    nameToMethod_t nexusMethods[] = {
        { "showTitleComponent", (void *)Zenonia_showTitleComponent },
        { "showReplyMoveComponent", (void *)Zenonia_showReplyMoveComponent },
        { "vibrate", (void *)Zenonia_vibrate },
        { "playBGM", (void *)Zenonia_playBGM },
        { "stopBGM", (void *)Zenonia_stopBGM },
        { "playSE", (void *)Zenonia_playSE },
        { "isNetworkConnected", (void *)Zenonia_isNetworkConnected },
        { "readAssets", (void *)Zenonia_readAssets },
        { "GFA_DrawFont", (void *)Zenonia_GFA_DrawFont },
        { "GFA_DrawText", (void *)Zenonia_GFA_DrawText },
        { "GFA_MeasureText", (void *)Zenonia_GFA_MeasureText },
        { NULL, NULL }
    };

    nameToField_t nexusFields[] = {
        { NULL, 0, NULL }
    };

    // Reemplaza puntero RegisterNatives
    jni->RegisterNatives = Zenonia_RegisterNatives;

    // Registra clases para resolucion JNI
    falsoJNI_registerClass("com/gamevil/nexus2/Natives", nexusMethods, nexusFields);
    falsoJNI_registerClass("com/gamevil/zenonia3/global/Zenonia3", nexusMethods, nexusFields);

    game_log("[JNI] FalsoJNI inicializado correctamente.\n");
}
