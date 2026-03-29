# Standalone CMakeLists.txt for The Lords of Midnight — AmigaOS4 build
# Links against ported axmol engine and all sub-project libraries

cmake_minimum_required(VERSION 3.22)
project(LordsOfMidnight C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Game selection: -DTME=LOM (default) or -DTME=DDR
if(NOT DEFINED TME OR TME STREQUAL "LOM")
    set(TME_GAME "LOM")
    set(TME_BUILD "_LOM_")
    set(TME_EXE_NAME "LordsOfMidnight")
elseif(TME STREQUAL "DDR")
    set(TME_GAME "DDR")
    set(TME_BUILD "_DDR_")
    set(TME_EXE_NAME "DoomdarksRevenge")
endif()

set(GAME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/main/midnight)
set(AX_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../axmol/src)

# Platform detection
include(${AX_ROOT}/1k/platforms.cmake)

##############################################################################
# Compile definitions
##############################################################################
add_compile_definitions(
    AX_USE_GL=1
    AX_USE_ALSOFT=1
    AX_ENABLE_CACHE_TEXTURE_DATA=0
    STBI_NO_THREAD_LOCALS
    AX_ENABLE_PREMULTIPLIED_ALPHA=1
    _OS_DESKTOP_=1
    _OS_AMIGAOS4_=1
    _UNIX_=1
    TME=${TME_GAME}
    ${TME_BUILD}
)

##############################################################################
# Gather game sources
##############################################################################
file(GLOB_RECURSE GAME_SRC
    ${GAME_ROOT}/Source/*.cpp
    ${GAME_ROOT}/proj.amigaos4/main.cpp
)

# Exclude other platform files
list(FILTER GAME_SRC EXCLUDE REGEX ".*/win32/.*")
list(FILTER GAME_SRC EXCLUDE REGEX ".*/android/.*")
list(FILTER GAME_SRC EXCLUDE REGEX ".*/ios/.*")
list(FILTER GAME_SRC EXCLUDE REGEX ".*/mac/.*")
list(FILTER GAME_SRC EXCLUDE REGEX ".*/apple/.*")
list(FILTER GAME_SRC EXCLUDE REGEX ".*cfile_windows\\.cpp")

# tinyxml2 (bundled in game)
set(TINYXML2_SRC ${GAME_ROOT}/Source/tinyxml2/tinyxml2.cpp)

##############################################################################
# Game executable
##############################################################################
add_executable(${TME_EXE_NAME} ${GAME_SRC} ${TINYXML2_SRC})
target_compile_options(${TME_EXE_NAME} PRIVATE -fpermissive)

target_include_directories(${TME_EXE_NAME} PRIVATE
    ${GAME_ROOT}/Source
    ${GAME_ROOT}/Source/tinyxml2
    ${GAME_ROOT}/Source/library/inc
    ${GAME_ROOT}/Source/tme
    ${GAME_ROOT}/Source/tme/scenarios
    ${GAME_ROOT}/Source/tme/scenarios/lom
    ${GAME_ROOT}/Source/tme/scenarios/ddr
    ${GAME_ROOT}/Source/tme/scenarios/default
    ${GAME_ROOT}/Source/system
    ${GAME_ROOT}/Source/platform
    ${GAME_ROOT}/Source/platform/amigaos4
    ${GAME_ROOT}/Source/landscaping
    ${GAME_ROOT}/Source/panels
    ${GAME_ROOT}/Source/ui
    ${GAME_ROOT}/Source/extensions
    # axmol engine
    ${AX_ROOT}
    ${AX_ROOT}/3rdparty
    ${AX_ROOT}/3rdparty/robin-map/include
    ${AX_ROOT}/3rdparty/rapidjson/include
    ${AX_ROOT}/3rdparty/fmt/include
    ${AX_ROOT}/3rdparty/concurrentqueue
    ${AX_ROOT}/3rdparty/ghc
    ${AX_ROOT}/3rdparty/ntcvt
    ${AX_ROOT}/3rdparty/pugixml
    ${AX_ROOT}/3rdparty/glad/include
    ${AX_ROOT}/3rdparty/mio/include
    ${AX_ROOT}/3rdparty/yasio
    ${AX_ROOT}/core
    ${AX_ROOT}/core/base
    ${AX_ROOT}/core/platform
    ${AX_ROOT}/core/platform/amigaos4
    ${AX_ROOT}/core/audio
)

# SDK paths (from toolchain)
set(AXMOL_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/../axmol/build)
set(GLFW_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/../glfw/build/src)
set(GLFW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../glfw/src/include)
set(OPENAL_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/../openal-soft/build)
set(OPENAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../openal-soft/src/include)

target_include_directories(${TME_EXE_NAME} PRIVATE
    ${GLFW_INCLUDE}
    ${OPENAL_INCLUDE}
)

# Link against our built libraries
target_link_directories(${TME_EXE_NAME} PRIVATE
    ${AXMOL_BUILD}
    ${GLFW_BUILD}
    ${OPENAL_BUILD}
)

target_link_libraries(${TME_EXE_NAME} PRIVATE
    # axmol engine + bundled 3rdparty
    axmol
    fmtlib
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
    # Our sub-project libraries
    glfw3
    openal
    # SDK pre-built libraries
    GL
    freetype
    png16
    jpeg
    webp
    z
    fmt
    curl
    ssl
    crypto
    pthread
    # AmigaOS4 system
    auto
)

target_link_options(${TME_EXE_NAME} PRIVATE -athread=native)
