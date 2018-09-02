add_library(VULKAN_STATIC INTERFACE IMPORTED GLOBAL)

if ("${BUILD_PLATFORM}" STREQUAL "Windows")
  if(NOT DEFINED VULKAN_1_1_77_0_INCLUDE_DIR)
    set(VULKAN_1_1_77_0_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/Vulkan/1.1.77.0/Include/
        CACHE STRING ""
        FORCE)
  endif()

  #  Vulkan 32-bit
  if(NOT DEFINED VULKAN_1_1_77_0_LIB32_DIR)
    set(VULKAN_1_1_77_0_LIB32_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/Vulkan/1.1.77.0/Lib32/
        CACHE STRING ""
        FORCE)
  endif()

  #  Vulkan 64-bit
  if(NOT DEFINED VULKAN_1_1_77_0_LIB64_DIR)
    set(VULKAN_1_1_77_0_LIB64_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/Vulkan/1.1.77.0/Lib/
        CACHE STRING ""
        FORCE)
  endif()

  if ("${BUILD_ARCH}" STREQUAL "64")
    set(VULKAN_INCLUDE ${VULKAN_1_1_77_0_INCLUDE_DIR} CACHE STRING "" FORCE)
    set(VULKAN_LIB ${VULKAN_1_1_77_0_LIB64_DIR} CACHE STRING "" FORCE)
  elseif("${BUILD_ARCH}" STREQUAL "32")
    set(VULKAN_INCLUDE ${VULKAN_1_1_77_0_INCLUDE_DIR} CACHE STRING "" FORCE)
    set(VULKAN_LIB ${VULKAN_1_1_77_0_LIB32_DIR} CACHE STRING "" FORCE)
  endif()

  target_link_libraries(VULKAN_STATIC INTERFACE ${VULKAN_LIB}/glfw3.lib)
endif()

target_include_directories(VULKAN_STATIC INTERFACE ${VULKAN_INCLUDE})
