cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    BootlegSQL
    VERSION 0.1
    LANGUAGES CXX
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set(CMAKE_POSITION_INDEPENDENT_CODE ON)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(COMPILE_FLAGS -Wall -Wextra -Wpedantic -Wconversion)
endif()

if(MSVC)
  add_compile_options(/W4)
  add_compile_options(/permissive-)
endif()

add_subdirectory(external)
add_subdirectory(src)

