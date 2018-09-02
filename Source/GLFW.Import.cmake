add_library(GLFW_STATIC INTERFACE IMPORTED GLOBAL)

if ("${BUILD_PLATFORM}" STREQUAL "Windows")
  #  GLFW 32-bit Windows
  if(NOT DEFINED GLFW_3_2_1_INCLUDE32_DIR)
    set(GLFW_3_2_1_INCLUDE32_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/GLFW/glfw-3.2.1.bin.WIN32/include/
        CACHE STRING ""
        FORCE)
  endif()

  if(NOT DEFINED GLFW_3_2_1_LIB32_DIR)
    set(GLFW_3_2_1_LIB32_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/GLFW/glfw-3.2.1.bin.WIN32/lib-vc2015/
        CACHE STRING ""
        FORCE)
  endif()

  #  GLFW 64-bit Windows
  if(NOT DEFINED GLFW_3_2_1_INCLUDE64_DIR)
    set(GLFW_3_2_1_INCLUDE64_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/GLFW/glfw-3.2.1.bin.WIN64/include/
        CACHE STRING ""
        FORCE)
  endif()

  if(NOT DEFINED GLFW_3_2_1_LIB64_DIR)
    set(GLFW_3_2_1_LIB64_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Imports/Windows/GLFW/glfw-3.2.1.bin.WIN64/lib-vc2015/
        CACHE STRING ""
        FORCE)
  endif()

  if ("${BUILD_ARCH}" STREQUAL "64")
    set(GLFW_INCLUDE ${GLFW_3_2_1_INCLUDE64_DIR} CACHE STRING "" FORCE)
    set(GLFW_LIB ${GLFW_3_2_1_LIB64_DIR} CACHE STRING "" FORCE)
  elseif("${BUILD_ARCH}" STREQUAL "32")
    set(GLFW_INCLUDE ${GLFW_3_2_1_INCLUDE32_DIR} CACHE STRING "" FORCE)
    set(GLFW_LIB ${GLFW_3_2_1_LIB32_DIR} CACHE STRING "" FORCE)
  endif()

  target_link_libraries(GLFW_STATIC INTERFACE ${GLFW_LIB}/glfw3.lib)
endif()

target_include_directories(GLFW_STATIC INTERFACE ${GLFW_INCLUDE})
