# Standalone CMakeLists.txt for axmol v2.3.1 core library — AmigaOS4 cross-build
# Bypasses axmol's internal dependency management (fetch.cmake, PowerShell)
# Links against pre-built SDK libraries for zlib, png, jpeg, freetype, fmt, etc.

cmake_minimum_required(VERSION 3.22)
project(axmol_amigaos4 C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AX_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Platform detection
include(${AX_ROOT}/1k/platforms.cmake)

# SDK paths (set by toolchain file)
# AMIGAOS4_SDK_LOCAL, GLFW_LIB_PATH, GLFW_INCLUDE_PATH, OPENAL_LIB_PATH, OPENAL_INCLUDE_PATH

##############################################################################
# Compile definitions
##############################################################################
add_compile_definitions(
    AX_USE_GL=1
    AX_USE_ALSOFT=1
    AX_ENABLE_CACHE_TEXTURE_DATA=0
    STBI_NO_THREAD_LOCALS
    AX_ENABLE_PREMULTIPLIED_ALPHA=1
)

##############################################################################
# Third-party: build from bundled source
##############################################################################

# GLAD (OpenGL loader)
add_library(glad STATIC ${AX_ROOT}/3rdparty/glad/src/gl.c)
target_include_directories(glad PUBLIC ${AX_ROOT}/3rdparty/glad/include)
# Don't define GLAD_GLES2 — the #ifdef guard skips the GLES2/EGL section entirely

# ConvertUTF
file(GLOB CONVERTUTF_SRC ${AX_ROOT}/3rdparty/ConvertUTF/*.c ${AX_ROOT}/3rdparty/ConvertUTF/*.cpp)
add_library(ConvertUTF STATIC ${CONVERTUTF_SRC})
target_include_directories(ConvertUTF PUBLIC ${AX_ROOT}/3rdparty/ConvertUTF)

# clipper2
file(GLOB CLIPPER2_SRC ${AX_ROOT}/3rdparty/clipper2/src/*.cpp)
add_library(clipper2 STATIC ${CLIPPER2_SRC})
target_include_directories(clipper2 PUBLIC ${AX_ROOT}/3rdparty/clipper2/include)

# poly2tri
file(GLOB_RECURSE POLY2TRI_SRC ${AX_ROOT}/3rdparty/poly2tri/*.cc)
add_library(poly2tri STATIC ${POLY2TRI_SRC})
target_include_directories(poly2tri PUBLIC ${AX_ROOT}/3rdparty/poly2tri)

# fastlz
add_library(fastlz STATIC ${AX_ROOT}/3rdparty/fastlz/fastlz.c)
target_include_directories(fastlz PUBLIC ${AX_ROOT}/3rdparty/fastlz)

# pugixml
add_library(pugixml STATIC ${AX_ROOT}/3rdparty/pugixml/pugixml.cpp)
target_include_directories(pugixml PUBLIC ${AX_ROOT}/3rdparty/pugixml)

# xxhash
add_library(xxhash STATIC ${AX_ROOT}/3rdparty/xxhash/xxhash.c)
target_include_directories(xxhash PUBLIC ${AX_ROOT}/3rdparty/xxhash)
target_compile_definitions(xxhash PRIVATE XXH_EXPORT)

# unzip (minizip)
file(GLOB UNZIP_SRC ${AX_ROOT}/3rdparty/unzip/*.c ${AX_ROOT}/3rdparty/unzip/*.cpp)
add_library(unzip STATIC ${UNZIP_SRC})
target_include_directories(unzip PUBLIC ${AX_ROOT}/3rdparty/unzip)
target_compile_definitions(unzip PRIVATE NOUNCRYPT)
target_link_libraries(unzip PRIVATE z)

# xsxml
add_library(xsxml INTERFACE)
target_include_directories(xsxml INTERFACE ${AX_ROOT}/3rdparty/xsxml)

# simdjson (scalar fallback on PPC)
add_library(simdjson STATIC ${AX_ROOT}/3rdparty/simdjson/simdjson.cpp)
target_include_directories(simdjson PUBLIC ${AX_ROOT}/3rdparty/simdjson)

# astcenc
file(GLOB ASTCENC_SRC ${AX_ROOT}/3rdparty/astcenc/astcenc_*.cpp)
add_library(astcenc STATIC ${ASTCENC_SRC})
target_include_directories(astcenc PUBLIC ${AX_ROOT}/3rdparty/astcenc)
target_compile_definitions(astcenc PRIVATE ASTCENC_SSE=0 ASTCENC_AVX=0 ASTCENC_POPCNT=0 ASTCENC_DECOMPRESS_ONLY)

# llhttp
file(GLOB LLHTTP_SRC ${AX_ROOT}/3rdparty/llhttp/src/*.c)
add_library(llhttp STATIC ${LLHTTP_SRC})
target_include_directories(llhttp PUBLIC ${AX_ROOT}/3rdparty/llhttp/include)

# websocket-parser
add_library(websocket-parser STATIC ${AX_ROOT}/3rdparty/websocket-parser/websocket_parser.c)
target_include_directories(websocket-parser PUBLIC ${AX_ROOT}/3rdparty/websocket-parser)

# ogg (for audio)
file(GLOB_RECURSE OGG_SRC ${AX_ROOT}/3rdparty/ogg/src/*.c)
# Exclude tool/test files that don't compile standalone
list(FILTER OGG_SRC EXCLUDE REGEX "psytune|tone|barkmel")
add_library(ogg STATIC ${OGG_SRC})
target_include_directories(ogg PUBLIC ${AX_ROOT}/3rdparty/ogg/include ${AX_ROOT}/3rdparty/ogg/src)
target_include_directories(ogg PRIVATE ${AX_ROOT}/3rdparty/ogg/src/vorbis)

# minimp3
add_library(minimp3 INTERFACE)
target_include_directories(minimp3 INTERFACE ${AX_ROOT}/3rdparty/minimp3)

# fmt (compiled, not just header-only — axmol uses fmt 11.1.1)
add_library(fmtlib STATIC
    ${AX_ROOT}/3rdparty/fmt/src/format.cc
    ${AX_ROOT}/3rdparty/fmt/src/os.cc
)
target_include_directories(fmtlib PUBLIC ${AX_ROOT}/3rdparty/fmt/include)

##############################################################################
# Third-party: header-only
##############################################################################
add_library(3rdparty_headers INTERFACE)
target_include_directories(3rdparty_headers INTERFACE
    ${AX_ROOT}/3rdparty
    ${AX_ROOT}/3rdparty/robin-map/include
    ${AX_ROOT}/3rdparty/rapidjson/include
    ${AX_ROOT}/3rdparty/concurrentqueue
    ${AX_ROOT}/3rdparty/stb
    ${AX_ROOT}/3rdparty/ghc
    ${AX_ROOT}/3rdparty/ntcvt
    ${AX_ROOT}/3rdparty/flatbuffers/include
    ${AX_ROOT}/3rdparty/fmt/include
    ${AX_ROOT}/3rdparty/mio/include
)

##############################################################################
# Third-party: yasio (networking)
##############################################################################
file(GLOB YASIO_SRC
    ${AX_ROOT}/3rdparty/yasio/yasio/*.cpp
)
add_library(yasio STATIC ${YASIO_SRC})
target_include_directories(yasio PUBLIC ${AX_ROOT}/3rdparty/yasio)
target_compile_definitions(yasio PUBLIC YASIO_HAVE_KCP=0)

##############################################################################
# Gather axmol core sources
##############################################################################
file(GLOB_RECURSE AX_CORE_SRC
    ${AX_ROOT}/core/2d/*.cpp
    ${AX_ROOT}/core/audio/*.cpp
    ${AX_ROOT}/core/base/*.cpp
    ${AX_ROOT}/core/math/*.cpp
    ${AX_ROOT}/core/network/*.cpp
    ${AX_ROOT}/core/renderer/*.cpp
    ${AX_ROOT}/core/ui/*.cpp
    ${AX_ROOT}/core/platform/*.cpp
    ${AX_ROOT}/core/axmol.cpp
)

# Exclude platform-specific files we don't want
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/win32/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/winrt/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/mac/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/ios/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/android/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/wasm/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/linux/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/metal/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*/UIWebView/.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*-android.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*-wasm.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*-win32.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*-winrt.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*-linux.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*AndroidMediaEngine.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*MfMediaEngine.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*WmfMediaEngine.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*MFUtils.*")
list(FILTER AX_CORE_SRC EXCLUDE REGEX ".*VlcMediaEngine.*")

##############################################################################
# axmol core library
##############################################################################
add_library(axmol STATIC ${AX_CORE_SRC})

target_include_directories(axmol PUBLIC
    ${AX_ROOT}
    ${AX_ROOT}/3rdparty
    ${AX_ROOT}/3rdparty/webp/src/webp
    ${AX_ROOT}/3rdparty/webp/src
    ${AX_ROOT}/core
    ${AX_ROOT}/core/base
    ${AX_ROOT}/core/platform
    ${AX_ROOT}/core/platform/amigaos4
    ${AX_ROOT}/core/audio
)

# GLFW (our patched build)
target_include_directories(axmol PUBLIC ${GLFW_INCLUDE_PATH})
target_link_directories(axmol PUBLIC ${GLFW_LIB_PATH})

# OpenAL (our AHI build)
target_include_directories(axmol PUBLIC ${OPENAL_INCLUDE_PATH})
# Internal OpenAL headers needed for AX_USE_ALSOFT (alc/inprogext.h, etc)
target_include_directories(axmol PRIVATE ${OPENAL_INCLUDE_PATH}/..)
target_link_directories(axmol PUBLIC ${OPENAL_LIB_PATH})

# Link bundled third-party libs
target_link_libraries(axmol PUBLIC
    glad
    ConvertUTF
    clipper2
    poly2tri
    fastlz
    pugixml
    xxhash
    unzip
    simdjson
    astcenc
    llhttp
    websocket-parser
    ogg
    yasio
    fmtlib
    3rdparty_headers
    minimp3
    xsxml
)

# Link SDK pre-built libraries
target_link_libraries(axmol PUBLIC
    glfw3
    openal
    GL
    freetype
    png16
    jpeg
    webp
    z
    curl
    ssl
    crypto
    pthread
)

# Precompiled header — axmol.h includes all engine headers
target_precompile_headers(axmol PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${AX_ROOT}/core/axmol.h>")

message(STATUS "axmol AmigaOS4 standalone build configured")
message(STATUS "  Core sources: ${AX_ROOT}/core/")
message(STATUS "  GLFW: ${GLFW_LIB_PATH}")
message(STATUS "  OpenAL: ${OPENAL_LIB_PATH}")
