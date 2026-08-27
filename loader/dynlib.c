#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>

#include <vitaGL.h>
#include "so_util.h"

#if defined(OPTIMIZE_NEON_FIXED) || defined(RGB565_MODE_NEON)
#include <arm_neon.h>
#endif

extern void game_log(const char *fmt, ...);
extern volatile int g_ui_status;

static int zenonia_verbose_ui(void) {
    return g_ui_status >= 0 && g_ui_status <= 2;
}

#ifdef ZENONIA_HIDE_DPAD_UI
static so_hook g_player_controller_ctor_hook;
#define GVUI_CONTROLLER_ACTIVE_COUNT_OFFSET 0x19c

static void GVUIPlayerController_ctor_hook(void *this) {
    so_unhook(&g_player_controller_ctor_hook);
    ((void (*)(void *)) g_player_controller_ctor_hook.thumb_addr)(this);
    *(int *)((char *) this + GVUI_CONTROLLER_ACTIVE_COUNT_OFFSET) = 0;
    game_log("[HideDpad] GVUIPlayerController construido en %p -- controles tactiles deshabilitados\n", this);
}
#endif

void zenonia_install_hide_dpad_hook(so_module *mod) {
#ifdef ZENONIA_HIDE_DPAD_UI
    uintptr_t ctor_addr = so_symbol(mod, "_ZN20GVUIPlayerControllerC2Ev");
    if (!ctor_addr) {
        game_log("[HideDpad] simbolo _ZN20GVUIPlayerControllerC2Ev no encontrado\n");
        return;
    }
    g_player_controller_ctor_hook = hook_addr(ctor_addr, (uintptr_t) GVUIPlayerController_ctor_hook);
    so_flush_caches(mod);
    game_log("[HideDpad] hook instalado en GVUIPlayerController::ctor\n");
#else
    (void) mod;
#endif
}

// Stub para __errno
int* __errno(void) {
    static int dummy_errno = 0;
    return &dummy_errno;
}

void glClearColorx_wrapper(int r, int g, int b, int a) {
    glClearColor(r / 65536.0f, g / 65536.0f, b / 65536.0f, a / 65536.0f);
}

void glTexParameterx_wrapper(GLenum target, GLenum pname, int param) {
    glTexParameteri(target, pname, param);
}

#if defined(RGB565_MODE_NEON)
static void convert_rgb565_to_rgba8888_neon(const uint16_t *src, uint8_t *dst, int npix) {
    int i = 0;
    for (; i + 8 <= npix; i += 8) {
        uint16x8_t p = vld1q_u16(src + i);
        uint16x8_t r5 = vshrq_n_u16(p, 11);
        uint16x8_t g6 = vandq_u16(vshrq_n_u16(p, 5), vdupq_n_u16(0x3F));
        uint16x8_t b5 = vandq_u16(p, vdupq_n_u16(0x1F));
        uint8x8_t r8 = vmovn_u16(vorrq_u16(vshlq_n_u16(r5, 3), vshrq_n_u16(r5, 2)));
        uint8x8_t g8 = vmovn_u16(vorrq_u16(vshlq_n_u16(g6, 2), vshrq_n_u16(g6, 4)));
        uint8x8_t b8 = vmovn_u16(vorrq_u16(vshlq_n_u16(b5, 3), vshrq_n_u16(b5, 2)));
        uint8x8x4_t out = {{ r8, g8, b8, vdup_n_u8(255) }};
        vst4_u8(dst + i * 4, out);
    }
    for (; i < npix; i++) {
        uint16_t p = src[i];
        uint8_t r5 = (p >> 11) & 0x1F, g6 = (p >> 5) & 0x3F, b5 = p & 0x1F;
        dst[i*4 + 0] = (r5 << 3) | (r5 >> 2);
        dst[i*4 + 1] = (g6 << 2) | (g6 >> 4);
        dst[i*4 + 2] = (b5 << 3) | (b5 >> 2);
        dst[i*4 + 3] = 255;
    }
}
#endif

void *convert_rgb565_to_rgba8888(const void *pixels, int width, int height) {
    static uint8_t *conv_buf = NULL;
    static int conv_buf_cap = 0;

    if (!pixels) return NULL;
    uint16_t *src = (uint16_t *)pixels;
    int npix = width * height;
    if (npix * 4 > conv_buf_cap) {
        uint8_t *new_buf = (uint8_t *)realloc(conv_buf, npix * 4);
        if (!new_buf) return NULL;
        conv_buf = new_buf;
        conv_buf_cap = npix * 4;
    }
    uint8_t *dst = conv_buf;

#if defined(RGB565_MODE_LUT)
    static uint8_t r5_to_8[32], g6_to_8[64], b5_to_8[32];
    static int lut_ready = 0;
    if (!lut_ready) {
        for (int i = 0; i < 32; i++) r5_to_8[i] = i * 255 / 31;
        for (int i = 0; i < 64; i++) g6_to_8[i] = i * 255 / 63;
        for (int i = 0; i < 32; i++) b5_to_8[i] = i * 255 / 31;
        lut_ready = 1;
    }
    for (int i = 0; i < npix; i++) {
        uint16_t p = src[i];
        dst[i*4 + 0] = r5_to_8[(p >> 11) & 0x1F];
        dst[i*4 + 1] = g6_to_8[(p >> 5) & 0x3F];
        dst[i*4 + 2] = b5_to_8[p & 0x1F];
        dst[i*4 + 3] = 255;
    }
#elif defined(RGB565_MODE_NEON)
    convert_rgb565_to_rgba8888_neon(src, dst, npix);
#else
    for (int i = 0; i < npix; i++) {
        uint16_t p = src[i];
        dst[i*4 + 0] = ((p >> 11) & 0x1F) * 255 / 31;
        dst[i*4 + 1] = ((p >> 5) & 0x3F) * 255 / 63;
        dst[i*4 + 2] = (p & 0x1F) * 255 / 31;
        dst[i*4 + 3] = 255;
    }
#endif
    return dst;
}

void glTexImage2D_wrapper(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
    if (format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5) {
#if defined(RGB565_MODE_NATIVE)
        glTexImage2D(target, level, GL_RGB, width, height, border, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels);
#else
        void *new_pixels = convert_rgb565_to_rgba8888(pixels, width, height);
        glTexImage2D(target, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, new_pixels);
#endif
    } else {
        glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    }
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void glTexSubImage2D_wrapper(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
    if (format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5) {
#if defined(RGB565_MODE_NATIVE)
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels);
#else
        void *new_pixels = convert_rgb565_to_rgba8888(pixels, width, height);
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, new_pixels);
#endif
    } else {
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    }
}

static const int32_t *pending_fixed_verts = NULL;
static GLint pending_fixed_size = 0;
static GLsizei pending_fixed_stride = 0;
static GLfloat *fixed_vert_buf = NULL;
static int fixed_vert_buf_cap = 0;

static const int32_t *pending_fixed_colors = NULL;
static GLint pending_fixed_color_size = 0;
static GLsizei pending_fixed_color_stride = 0;
static GLfloat *fixed_color_buf = NULL;
static int fixed_color_buf_cap = 0;

static const int32_t *pending_fixed_texcoords = NULL;
static GLint pending_fixed_texcoord_size = 0;
static GLsizei pending_fixed_texcoord_stride = 0;
static GLfloat *fixed_texcoord_buf = NULL;
static int fixed_texcoord_buf_cap = 0;

#ifdef OPTIMIZE_NEON_FIXED
static void fixed_to_float_neon(const int32_t *src, GLfloat *dst, int total_elems) {
    int i = 0;
    float32x4_t scale = vdupq_n_f32(1.0f / 65536.0f);
    for (; i + 4 <= total_elems; i += 4) {
        int32x4_t v = vld1q_s32(src + i);
        float32x4_t f = vcvtq_f32_s32(v);
        vst1q_f32(dst + i, vmulq_f32(f, scale));
    }
    for (; i < total_elems; i++) {
        dst[i] = src[i] / 65536.0f;
    }
}
#endif

void glDrawArrays_wrapper(GLenum mode, GLint first, GLsizei count) {
    if (pending_fixed_verts) {
        int needed_verts = first + count;
        int needed_floats = needed_verts * pending_fixed_size;
        if (needed_floats > fixed_vert_buf_cap) {
            GLfloat *new_buf = (GLfloat *)realloc(fixed_vert_buf, needed_floats * sizeof(GLfloat));
            if (!new_buf) return;
            fixed_vert_buf = new_buf;
            fixed_vert_buf_cap = needed_floats;
        }
        int stride_elems = pending_fixed_stride > 0 ? pending_fixed_stride / sizeof(int32_t) : pending_fixed_size;
#ifdef OPTIMIZE_NEON_FIXED
        if (stride_elems == pending_fixed_size) {
            fixed_to_float_neon(pending_fixed_verts, fixed_vert_buf, needed_verts * pending_fixed_size);
        } else
#endif
        {
            for (int i = 0; i < needed_verts; i++) {
                const int32_t *src = pending_fixed_verts + i * stride_elems;
                GLfloat *dst = fixed_vert_buf + i * pending_fixed_size;
                for (int c = 0; c < pending_fixed_size; c++) dst[c] = src[c] / 65536.0f;
            }
        }
        glVertexPointer(pending_fixed_size, GL_FLOAT, 0, fixed_vert_buf);
    }

    if (pending_fixed_colors) {
        int needed_verts = first + count;
        int needed_floats = needed_verts * pending_fixed_color_size;
        if (needed_floats > fixed_color_buf_cap) {
            GLfloat *new_buf = (GLfloat *)realloc(fixed_color_buf, needed_floats * sizeof(GLfloat));
            if (!new_buf) return;
            fixed_color_buf = new_buf;
            fixed_color_buf_cap = needed_floats;
        }
        int stride_elems = pending_fixed_color_stride > 0 ? pending_fixed_color_stride / sizeof(int32_t) : pending_fixed_color_size;
#ifdef OPTIMIZE_NEON_FIXED
        if (stride_elems == pending_fixed_color_size) {
            fixed_to_float_neon(pending_fixed_colors, fixed_color_buf, needed_verts * pending_fixed_color_size);
        } else
#endif
        {
            for (int i = 0; i < needed_verts; i++) {
                const int32_t *src = pending_fixed_colors + i * stride_elems;
                GLfloat *dst = fixed_color_buf + i * pending_fixed_color_size;
                for (int c = 0; c < pending_fixed_color_size; c++) dst[c] = src[c] / 65536.0f;
            }
        }
        glColorPointer(pending_fixed_color_size, GL_FLOAT, 0, fixed_color_buf);
    }

    if (pending_fixed_texcoords) {
        int needed_verts = first + count;
        int needed_floats = needed_verts * pending_fixed_texcoord_size;
        if (needed_floats > fixed_texcoord_buf_cap) {
            GLfloat *new_buf = (GLfloat *)realloc(fixed_texcoord_buf, needed_floats * sizeof(GLfloat));
            if (!new_buf) return;
            fixed_texcoord_buf = new_buf;
            fixed_texcoord_buf_cap = needed_floats;
        }
        int stride_elems = pending_fixed_texcoord_stride > 0 ? pending_fixed_texcoord_stride / sizeof(int32_t) : pending_fixed_texcoord_size;
#ifdef OPTIMIZE_NEON_FIXED
        if (stride_elems == pending_fixed_texcoord_size) {
            fixed_to_float_neon(pending_fixed_texcoords, fixed_texcoord_buf, needed_verts * pending_fixed_size);
        } else
#endif
        {
            for (int i = 0; i < needed_verts; i++) {
                const int32_t *src = pending_fixed_texcoords + i * stride_elems;
                GLfloat *dst = fixed_texcoord_buf + i * pending_fixed_texcoord_size;
                for (int c = 0; c < pending_fixed_texcoord_size; c++) dst[c] = src[c] / 65536.0f;
            }
        }
        glTexCoordPointer(pending_fixed_texcoord_size, GL_FLOAT, 0, fixed_texcoord_buf);
    }

    glDrawArrays(mode, first, count);
}

void glTexEnvf_wrapper(GLenum target, GLenum pname, GLfloat param) {
    glTexEnvf(target, pname, param);
}

void glBlendFunc_wrapper(GLenum sfactor, GLenum dfactor) {
    glBlendFunc(sfactor, dfactor);
}

void glEnable_wrapper(GLenum cap) {
    glEnable(cap);
}

void glDisable_wrapper(GLenum cap) {
    glDisable(cap);
}

void glTexCoordPointer_wrapper(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if (type == GL_FIXED) {
        pending_fixed_texcoords = (const int32_t *)pointer;
        pending_fixed_texcoord_size = size;
        pending_fixed_texcoord_stride = stride;
        return;
    }
    pending_fixed_texcoords = NULL;
    glTexCoordPointer(size, type, stride, pointer);
}

void glColorPointer_wrapper(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    glEnableClientState(GL_COLOR_ARRAY);
    if (type == GL_FIXED) {
        pending_fixed_colors = (const int32_t *)pointer;
        pending_fixed_color_size = size;
        pending_fixed_color_stride = stride;
        return;
    }
    pending_fixed_colors = NULL;
    glColorPointer(size, type, stride, pointer);
}

void glVertexPointer_wrapper(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    glEnableClientState(GL_VERTEX_ARRAY);
    if (type == GL_FIXED) {
        pending_fixed_verts = (const int32_t *)pointer;
        pending_fixed_size = size;
        pending_fixed_stride = stride;
        return;
    }
    pending_fixed_verts = NULL;
    glVertexPointer(size, type, stride, pointer);
}

void glEnableClientState_wrapper(GLenum array) {
    glEnableClientState(array);
}

void glDisableClientState_wrapper(GLenum array) {
    glDisableClientState(array);
}

void glMatrixMode_wrapper(GLenum mode) {
    glMatrixMode(mode);
}

void glLoadIdentity_wrapper() {
    glLoadIdentity();
}

void glViewport_wrapper(GLint x, GLint y, GLsizei width, GLsizei height) {
    (void)x; (void)y; (void)width; (void)height;
    glViewport(0, 0, 960, 544);
}

void glOrthof_wrapper(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
    glOrthof(left, right, bottom, top, zNear, zFar);
}

void glOrthox_wrapper(GLint left, GLint right, GLint bottom, GLint top, GLint zNear, GLint zFar) {
    glOrthof_wrapper(left / 65536.0f, right / 65536.0f, bottom / 65536.0f, top / 65536.0f, zNear / 65536.0f, zFar / 65536.0f);
}

// Memory & ABI Wrappers
void *__aeabi_memset_impl(void *dest, size_t n, int c) {
    return memset(dest, c, n);
}
void *__aeabi_memclr_impl(void *dest, size_t n) {
    return memset(dest, 0, n);
}
void *__aeabi_memcpy_impl(void *dest, const void *src, size_t n) {
    return memcpy(dest, src, n);
}
void *__aeabi_memmove_impl(void *dest, const void *src, size_t n) {
    return memmove(dest, src, n);
}
void __cxa_pure_virtual(void) {}
int __android_log_print_dummy(int prio, const char *tag, const char *fmt, ...) { 
    (void)prio; (void)tag; (void)fmt; 
    return 0; 
}
static int fake_sF[3];
int dladdr_fake(const void *addr, void *info) { 
    (void)addr; (void)info; 
    return 0; 
}

// Declaraciones externas procedentes de libstdc++
extern void __cxa_begin_cleanup(void);
extern void __cxa_call_unexpected(void);
extern void __cxa_type_match(void);

// Stubs C++/GCC
int __cxa_guard_acquire(int* g) { return !*(char*)(g); }
void __cxa_guard_release(int* g) { *(char*)g = 1; }
void __gnu_Unwind_Find_exidx() {}
void __stack_chk_fail() {}

int __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle) {
    (void) func; (void) arg; (void) dso_handle;
    return 0;
}
void __cxa_finalize(void *dso_handle) {
    (void) dso_handle;
}
int __aeabi_atexit(void *arg, void (*func)(void *), void *dso_handle) {
    return __cxa_atexit(func, arg, dso_handle);
}
int __stack_chk_guard = 0;

int pthread_mutex_lock_wrapper(pthread_mutex_t *mutex) {
    if (mutex && (*mutex == NULL || (intptr_t)*mutex == 0x4000)) {
        *mutex = NULL;
        pthread_mutex_init(mutex, NULL);
    }
    return pthread_mutex_lock(mutex);
}
int pthread_mutex_unlock_wrapper(pthread_mutex_t *mutex) {
    if (mutex && (*mutex == NULL || (intptr_t)*mutex == 0x4000)) {
        *mutex = NULL;
        pthread_mutex_init(mutex, NULL);
    }
    return pthread_mutex_unlock(mutex);
}
int pthread_mutex_destroy_wrapper(pthread_mutex_t *mutex) {
    if (!mutex || *mutex == NULL || (intptr_t)*mutex == 0x4000) return 0;
    return pthread_mutex_destroy(mutex);
}
int pthread_cond_wait_wrapper(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    if (cond && (*cond == NULL || (intptr_t)*cond == 0x4000)) {
        *cond = NULL;
        pthread_cond_init(cond, NULL);
    }
    if (mutex && (*mutex == NULL || (intptr_t)*mutex == 0x4000)) {
        *mutex = NULL;
        pthread_mutex_init(mutex, NULL);
    }
    return pthread_cond_wait(cond, mutex);
}
int pthread_cond_broadcast_wrapper(pthread_cond_t *cond) {
    if (cond && (*cond == NULL || (intptr_t)*cond == 0x4000)) {
        *cond = NULL;
        pthread_cond_init(cond, NULL);
    }
    return pthread_cond_broadcast(cond);
}

void* malloc_wrapper(size_t size) {
    return malloc(size);
}
void free_wrapper(void* ptr) {
    free(ptr);
}
void* calloc_wrapper(size_t n, size_t size) {
    return calloc(n, size);
}
void* realloc_wrapper(void* ptr, size_t size) {
    return realloc(ptr, size);
}

void translate_path(const char* in_path, char* out_path, size_t out_size) {
    if (strncmp(in_path, "ux0:", 4) == 0) {
        snprintf(out_path, out_size, "%s", in_path);
        return;
    }
    const char* relative = in_path;
    if (strncmp(in_path, "app0:/", 6) == 0) {
        relative += 6;
    }
    while (*relative == '/') {
        relative++;
    }
    snprintf(out_path, out_size, "ux0:data/zenonia3/assets/%s", relative);
}

FILE* fopen_hook(const char* path, const char* mode) {
    char new_path[256];
    translate_path(path, new_path, sizeof(new_path));
    return fopen(new_path, mode);
}

typedef struct {
    uint64_t st_dev;
    uint8_t  __pad0[4];
    uint32_t __st_ino;
    uint32_t st_mode;
    uint32_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint64_t st_rdev;
    uint8_t  __pad3[4];
    int64_t  st_size;
    uint32_t st_blksize;
    uint64_t st_blocks;
    uint32_t st_atime;
    uint32_t st_atime_nsec;
    uint32_t st_mtime;
    uint32_t st_mtime_nsec;
    uint32_t st_ctime;
    uint32_t st_ctime_nsec;
    uint64_t st_ino;
} bionic_stat_t;

int stat_hook(const char* path, void* statbuf) {
    char new_path[256];
    translate_path(path, new_path, sizeof(new_path));
    struct stat st;
    int res = stat(new_path, &st);
    if (res == 0 && statbuf) {
        bionic_stat_t *bst = (bionic_stat_t *) statbuf;
        memset(bst, 0, sizeof(*bst));
        bst->st_mode = st.st_mode;
        bst->st_nlink = st.st_nlink;
        bst->st_uid = st.st_uid;
        bst->st_gid = st.st_gid;
        bst->st_size = st.st_size;
        bst->st_blksize = st.st_blksize;
        bst->st_blocks = st.st_blocks;
        bst->st_atime = st.st_atime;
        bst->st_mtime = st.st_mtime;
        bst->st_ctime = st.st_ctime;
        bst->__st_ino = st.st_ino;
        bst->st_ino = st.st_ino;
    }
    return res;
}

int access_hook(const char* path, int amode) {
    char new_path[256];
    translate_path(path, new_path, sizeof(new_path));
    return access(new_path, amode);
}

so_default_dynlib default_dynlib[] = {
    // ARM EABI helpers
    { "__aeabi_memset", (uintptr_t)&__aeabi_memset_impl },
    { "__aeabi_memclr", (uintptr_t)&__aeabi_memclr_impl },
    { "__aeabi_memclr4", (uintptr_t)&__aeabi_memclr_impl },
    { "__aeabi_memcpy", (uintptr_t)&__aeabi_memcpy_impl },
    { "__aeabi_memcpy4", (uintptr_t)&__aeabi_memcpy_impl },
    { "__aeabi_memmove", (uintptr_t)&__aeabi_memmove_impl },
    { "__cxa_pure_virtual", (uintptr_t)&__cxa_pure_virtual },

    // Bionic & Android
    { "__android_log_print", (uintptr_t)&__android_log_print_dummy },
    { "__sF", (uintptr_t)&fake_sF },
    { "dladdr", (uintptr_t)&dladdr_fake },

    // C++ ABI / GCC
    { "__cxa_atexit", (uintptr_t)&__cxa_atexit },
    { "__cxa_finalize", (uintptr_t)&__cxa_finalize },
    { "__aeabi_atexit", (uintptr_t)&__aeabi_atexit },
    { "__cxa_begin_cleanup", (uintptr_t)&__cxa_begin_cleanup },
    { "__cxa_call_unexpected", (uintptr_t)&__cxa_call_unexpected },
    { "__cxa_guard_acquire", (uintptr_t)&__cxa_guard_acquire },
    { "__cxa_guard_release", (uintptr_t)&__cxa_guard_release },
    { "__cxa_type_match", (uintptr_t)&__cxa_type_match },
    { "__gnu_Unwind_Find_exidx", (uintptr_t)&__gnu_Unwind_Find_exidx },
    { "__stack_chk_fail", (uintptr_t)&__stack_chk_fail },
    { "__stack_chk_guard", (uintptr_t)&__stack_chk_guard },

    // Libc - Memoria
    { "malloc", (uintptr_t)&malloc_wrapper },
    { "free", (uintptr_t)&free_wrapper },
    { "calloc", (uintptr_t)&calloc_wrapper },
    { "realloc", (uintptr_t)&realloc_wrapper },
    { "memcpy", (uintptr_t)&memcpy },
    { "memset", (uintptr_t)&memset },
    { "memmove", (uintptr_t)&memmove },
    { "memcmp", (uintptr_t)&memcmp },

    // Libc - String
    { "strcpy", (uintptr_t)&strcpy },
    { "strncpy", (uintptr_t)&strncpy },
    { "strcmp", (uintptr_t)&strcmp },
    { "strncmp", (uintptr_t)&strncmp },
    { "strcat", (uintptr_t)&strcat },
    { "strncat", (uintptr_t)&strncat },
    { "strlen", (uintptr_t)&strlen },
    { "strstr", (uintptr_t)&strstr },
    { "strchr", (uintptr_t)&strchr },

    // Libc - I/O
    { "fopen", (uintptr_t)&fopen_hook },
    { "fread", (uintptr_t)&fread },
    { "fwrite", (uintptr_t)&fwrite },
    { "fclose", (uintptr_t)&fclose },
    { "fseek", (uintptr_t)&fseek },
    { "ftell", (uintptr_t)&ftell },
    { "feof", (uintptr_t)&feof },
    { "fprintf", (uintptr_t)&fprintf },
    { "fflush", (uintptr_t)&fflush },
    { "snprintf", (uintptr_t)&snprintf },
    { "printf", (uintptr_t)&printf },
    { "vprintf", (uintptr_t)&vprintf },
    { "vsprintf", (uintptr_t)&vsprintf },
    { "sprintf", (uintptr_t)&sprintf },
    { "putchar", (uintptr_t)&putchar },
    { "puts", (uintptr_t)&puts },

    // Libc - Filesystem
    { "access", (uintptr_t)&access_hook },
    { "stat", (uintptr_t)&stat_hook },
    { "unlink", (uintptr_t)&unlink },
    { "rename", (uintptr_t)&rename },
    { "close", (uintptr_t)&close },
    { "fcntl", (uintptr_t)&fcntl },
    { "select", (uintptr_t)&select },

    // Libc - Tiempo y matematica
    { "time", (uintptr_t)&time },
    { "gmtime", (uintptr_t)&gmtime },
    { "gettimeofday", (uintptr_t)&gettimeofday },
    { "localtime", (uintptr_t)&localtime },
    { "ceil", (uintptr_t)&ceil },
    { "atoi", (uintptr_t)&atoi },
    { "abort", (uintptr_t)&abort },
    { "exit", (uintptr_t)&exit },
    { "raise", (uintptr_t)&raise },

    // Pthreads
    { "pthread_mutex_init", (uintptr_t)&pthread_mutex_init },
    { "pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_wrapper },
    { "pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_wrapper },
    { "pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_wrapper },
    { "pthread_cond_wait", (uintptr_t)&pthread_cond_wait_wrapper },
    { "pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast_wrapper },
    { "pthread_key_create", (uintptr_t)&pthread_key_create },
    { "pthread_getspecific", (uintptr_t)&pthread_getspecific },
    { "pthread_setspecific", (uintptr_t)&pthread_setspecific },

    // OpenGL / vitaGL
    { "glActiveTexture", (uintptr_t)&glActiveTexture },
    { "glBindTexture", (uintptr_t)&glBindTexture },
    { "glClear", (uintptr_t)&glClear },
    { "glClearColorx", (uintptr_t)&glClearColorx_wrapper },
    { "glColorPointer", (uintptr_t)&glColorPointer_wrapper },
    { "glColor4f", (uintptr_t)&glColor4f },
    { "glColor4x", (uintptr_t)&glClearColorx_wrapper },
    { "glDisable", (uintptr_t)&glDisable_wrapper },
    { "glDisableClientState", (uintptr_t)&glDisableClientState_wrapper },
    { "glDrawArrays", (uintptr_t)&glDrawArrays_wrapper },
    { "glEnable", (uintptr_t)&glEnable_wrapper },
    { "glEnableClientState", (uintptr_t)&glEnableClientState_wrapper },
    { "glBlendFunc", (uintptr_t)&glBlendFunc_wrapper },
    { "glAlphaFunc", (uintptr_t)&glAlphaFunc },
    { "glGenTextures", (uintptr_t)&glGenTextures },
    { "glHint", (uintptr_t)&glHint },
    { "glLoadIdentity", (uintptr_t)&glLoadIdentity_wrapper },
    { "glMatrixMode", (uintptr_t)&glMatrixMode_wrapper },
    { "glNormalPointer", (uintptr_t)&glNormalPointer },
    { "glOrthof", (uintptr_t)&glOrthof_wrapper },
    { "glOrthox", (uintptr_t)&glOrthox_wrapper },
    { "glTexCoordPointer", (uintptr_t)&glTexCoordPointer_wrapper },
    { "glTexEnvf", (uintptr_t)&glTexEnvf_wrapper },
    { "glTexImage2D", (uintptr_t)&glTexImage2D_wrapper },
    { "glTexSubImage2D", (uintptr_t)&glTexSubImage2D_wrapper },
    { "glTexParameterx", (uintptr_t)&glTexParameterx_wrapper },
    { "glVertexPointer", (uintptr_t)&glVertexPointer_wrapper },
    { "glViewport", (uintptr_t)&glViewport_wrapper },

    // Red / Sockets
    { "socket", (uintptr_t)&socket },
    { "connect", (uintptr_t)&connect },
    { "send", (uintptr_t)&send },
    { "recv", (uintptr_t)&recv },
    { "sendto", (uintptr_t)&sendto },
    { "recvfrom", (uintptr_t)&recvfrom },
    { "shutdown", (uintptr_t)&shutdown },
    { "inet_addr", (uintptr_t)&inet_addr },

    // Libc core
    { "__errno", (uintptr_t)&__errno },
};

int default_dynlib_size = sizeof(default_dynlib);
