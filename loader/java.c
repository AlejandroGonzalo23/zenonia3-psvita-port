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

// Punteros a las funciones nativas registradas dinámicamente por libgameDSO.so
extern void (* NativeInitDeviceInfo)(void *env, void *obj, int w, int h);
extern void (* NativeInitWithBufferSize)(void *env, void *obj, int w, int h);
extern void (* NativeRender)(void *env, void *obj);
extern void (* NativeResize)(void *env, void *obj, int w, int h);
extern void (* NativeResumeClet)(void *env, void *obj);
extern void (* handleCletEvent)(void *env, void *obj, int type, int p1, int p2, int p3);

volatile int g_ui_status = 0;

/* --- Definición del array dinámico auxiliar --- */

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

/* --- Intercepción de RegisterNatives --- */

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

// Interfaz gráfica y fuentes (GFA)
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

/* --- Tabla de Métodos JNI y Clases FalsoJNI --- */

static NamelessMethod nexusMethods[] = {
    { 1, "showTitleComponent", METHOD_TYPE_VOID, (void *)Zenonia_showTitleComponent },
    { 2, "showReplyMoveComponent", METHOD_TYPE_VOID, (void *)Zenonia_showReplyMoveComponent },
    { 3, "vibrate", METHOD_TYPE_VOID, (void *)Zenonia_vibrate },
    { 4, "playBGM", METHOD_TYPE_VOID, (void *)Zenonia_playBGM },
    { 5, "stopBGM", METHOD_TYPE_VOID, (void *)Zenonia_stopBGM },
    { 6, "playSE", METHOD_TYPE_VOID, (void *)Zenonia_playSE },
    { 7, "isNetworkConnected", METHOD_TYPE_BOOLEAN, (void *)Zenonia_isNetworkConnected },
    { 8, "readAssets", METHOD_TYPE_OBJECT, (void *)Zenonia_readAssets },
    { 9, "GFA_DrawFont", METHOD_TYPE_OBJECT, (void *)Zenonia_GFA_DrawFont },
    { 10, "GFA_DrawText", METHOD_TYPE_OBJECT, (void *)Zenonia_GFA_DrawText },
    { 11, "GFA_MeasureText", METHOD_TYPE_OBJECT, (void *)Zenonia_GFA_MeasureText },
    { 0, NULL, 0, NULL }
};

static NamelessField nexusFields[] = {
    { 0, NULL, 0 }
};

static NamelessClass registeredClasses[] = {
    { 1, "com/gamevil/nexus2/Natives", nexusMethods, nexusFields },
    { 2, "com/gamevil/zenonia3/global/Zenonia3", nexusMethods, nexusFields },
    { 0, NULL, NULL, NULL }
};

/* --- Hooks de gestión de Arrays en JNIEnv --- */

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
    jni.GetArrayLength = Zenonia_GetArrayLength;
    jni.GetByteArrayElements = Zenonia_GetByteArrayElements;
    jni.ReleaseByteArrayElements = Zenonia_ReleaseByteArrayElements;
    jni.GetFloatArrayElements = Zenonia_GetFloatArrayElements;
    jni.ReleaseFloatArrayElements = Zenonia_ReleaseFloatArrayElements;
    jni.RegisterNatives = Zenonia_RegisterNatives;
    game_log("[JNI] Hooks de arrays y RegisterNatives instalados en JNIEnv\n");
}

void jni_init(void) {
    FalsoJNI_LoggerSetName("Zenonia3");
    
    // Asignar el hook de RegisterNatives antes de inicializar la JVM
    jni.RegisterNatives = Zenonia_RegisterNatives;
    
    for (int i = 0; registeredClasses[i].name != NULL; i++) {
        falsoJNI_registerClass(registeredClasses[i].name, registeredClasses[i].methods, registeredClasses[i].fields);
    }
    
    game_log("[JNI] FalsoJNI inicializado correctamente.\n");
}
