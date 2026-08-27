cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(zenonia_3)
include("${VITASDK}/share/vita.cmake" REQUIRED)

## Configuracion de la app
set(VITA_APP_NAME "Zenonia 3")
set(VITA_TITLEID  "PSVZ00003")
set(VITA_VERSION  "01.00")

option(ENABLE_VERBOSE_JNI_LOG "Log every FalsoJNI call (debug only, slow)" OFF)
if (ENABLE_VERBOSE_JNI_LOG)
  add_definitions(-DFALSOJNI_DEBUGLEVEL=0)
else()
  add_definitions(-DFALSOJNI_DEBUGLEVEL=2)
endif()

option(HIDE_VIRTUAL_GAMEPAD "Hide on-screen controls" ON)
if (HIDE_VIRTUAL_GAMEPAD)
  add_definitions(-DZENONIA_HIDE_DPAD_UI=1)
endif()

set(RGB565_CONVERT_MODE "NATIVE" CACHE STRING "RGB565->RGBA8888 conversion strategy: SCALAR, LUT, NEON o NATIVE")
set_property(CACHE RGB565_CONVERT_MODE PROPERTY STRINGS SCALAR LUT NEON NATIVE)
if (RGB565_CONVERT_MODE STREQUAL "SCALAR")
  add_definitions(-DRGB565_MODE_SCALAR=1)
elseif (RGB565_CONVERT_MODE STREQUAL "LUT")
  add_definitions(-DRGB565_MODE_LUT=1)
elseif (RGB565_CONVERT_MODE STREQUAL "NEON")
  add_definitions(-DRGB565_MODE_NEON=1)
elseif (RGB565_CONVERT_MODE STREQUAL "NATIVE")
  add_definitions(-DRGB565_MODE_NATIVE=1)
endif()

option(OPTIMIZE_NEON_FIXED "Use NEON SIMD for GL_FIXED" OFF)
if (OPTIMIZE_NEON_FIXED)
  add_definitions(-DOPTIMIZE_NEON_FIXED=1)
endif()

option(LOCK_FPS_30 "Cap framerate at 30fps" ON)
if (LOCK_FPS_30)
  add_definitions(-DLOCK_FPS_30=1)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_XOPEN_SOURCE=700")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1 -d ATTRIBUTE2=12")

include_directories(
  ./common
  ./lib
  ./lib/falso_jni
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
  ./loader/main.c
  ./loader/dynlib.c
  ./loader/so_util.c
  ./loader/java.c
  ./loader/audio.c
  ./loader/font.c
  ./loader/htmltext.c
  ./loader/htmlview.c
  ./loader/androidui.c
  ./common/debugScreen.c
  ./lib/falso_jni/FalsoJNI.c
  ./lib/falso_jni/FalsoJNI_Logger.c
  ./lib/falso_jni/ConvertUTF.c
)

set_source_files_properties(./lib/falso_jni/ConvertUTF.c PROPERTIES COMPILE_FLAGS "-fgnu89-inline")

target_link_libraries(${PROJECT_NAME}
  vitaGL
  vitashark
  mathneon
  m
  vorbisidec
  ogg
  -Wl,--whole-archive pthread -Wl,--no-whole-archive
  SceDisplay_stub
  SceCtrl_stub
  SceTouch_stub
  SceAudio_stub
  SceAudioIn_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceSysmodule_stub
  ScePower_stub
  SceGxm_stub
  SceShaccCg_stub
  SceShaccCgExt
  SceCommonDialog_stub
  SceKernelDmacMgr_stub
  taihen_stub
  kubridge_stub
  stdc++
)

## Crear archivos de Vita
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE NOASLR)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/pic0.png sce_sys/pic0.png
  FILE sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
