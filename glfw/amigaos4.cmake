set(CMAKE_SYSTEM_NAME AmigaOS4)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

set(CMAKE_C_COMPILER   ppc-amigaos-gcc)
set(CMAKE_CXX_COMPILER ppc-amigaos-g++)
set(CMAKE_AR           ppc-amigaos-ar)
set(CMAKE_RANLIB       ppc-amigaos-ranlib)

set(CMAKE_C_FLAGS_INIT   "-mcrt=clib4")
set(CMAKE_CXX_FLAGS_INIT "-mcrt=clib4")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mcrt=clib4")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(AMIGAOS4 TRUE)
set(CLIB4 TRUE)

# Vulkan SDK headers from our vulkan-sdk sub-project
set(VULKAN_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../vulkan-sdk")
include_directories(SYSTEM "${VULKAN_SDK_PATH}/include")
