include(FetchContent)

# EXPECTED
add_library (Expected INTERFACE)
target_include_directories(Expected INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/expected)

# FMT
find_package(fmt QUIET)
if (NOT fmt_FOUND)
    set(FMT_TEST FF CACHE INTERNAL "disabling fmt tests")
    FetchContent_Declare(
            fmt
            GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
            GIT_TAG         11.1.4
            GIT_PROGRESS    TRUE
            USES_TERMINAL_DOWNLOAD TRUE
    )
    FetchContent_MakeAvailable(fmt)
endif()

# DOCTEST
add_library(Doctest INTERFACE)
target_include_directories(Doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/doctest)
