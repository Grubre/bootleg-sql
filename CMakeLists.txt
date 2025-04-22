cmake_minimum_required(VERSION 3.20)

# antlr config variables
# they can't be set in ./external/antlr/CMakeLists.txt cause then ./src/CMakeLists.txt doesn't see them
set(ANTLR4_WITH_STATIC_CRT OFF) # using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR_BUILD_SHARED OFF)
set(ANTLR_TAG 4.13.2)
set(DISABLE_WARNINGS ON)
set(ANTLR_BUILD_CPP_TESTS OFF)
set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/4.13.2.zip)

set(ANTLR_EXECUTABLE "${CMAKE_SOURCE_DIR}/build/antlr-${ANTLR_TAG}-complete.jar")

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

if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

add_subdirectory(external)
add_subdirectory(src)

