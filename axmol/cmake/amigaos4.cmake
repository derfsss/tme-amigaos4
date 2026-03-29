set(CMAKE_SYSTEM_NAME AmigaOS4)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

set(CMAKE_C_COMPILER   ppc-amigaos-gcc)
set(CMAKE_CXX_COMPILER ppc-amigaos-g++)
set(CMAKE_AR           ppc-amigaos-ar)
set(CMAKE_RANLIB       ppc-amigaos-ranlib)

set(CMAKE_C_FLAGS_INIT   "-mcrt=clib4")
set(CMAKE_CXX_FLAGS_INIT "-mcrt=clib4 -std=c++20")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mcrt=clib4")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(AMIGAOS4 TRUE)
set(CLIB4 TRUE)

# SDK paths inside Docker (clib4-dev:os4-gcc11)
set(AMIGAOS4_SDK_LOCAL "/opt/ppc-amigaos/ppc-amigaos/SDK/local")
set(AMIGAOS4_SDK_COMMON "${AMIGAOS4_SDK_LOCAL}/common")

# GL4ES headers and libs
include_directories(SYSTEM "${AMIGAOS4_SDK_COMMON}/include")
# clib4-specific headers (freetype2, etc)
include_directories(SYSTEM "${AMIGAOS4_SDK_LOCAL}/clib4/include")
include_directories(SYSTEM "${AMIGAOS4_SDK_LOCAL}/clib4/include/freetype2")
link_directories("${AMIGAOS4_SDK_LOCAL}/clib4/lib")

# VulkanOS4 SDK (for future use)
set(VULKAN_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../vulkan-sdk")

# GLFW built from our glfw sub-project
set(GLFW_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../../glfw/build/src")
set(GLFW_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../glfw/src/include")

# OpenAL built from our openal-soft sub-project
set(OPENAL_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../../openal-soft/build")
set(OPENAL_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../openal-soft/src/include")
