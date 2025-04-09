set (EXEC_NAME "db")

add_executable(${EXEC_NAME} main.cpp)

target_link_libraries(${EXEC_NAME} Expected fmt)

set(MAIN_FLAGS ${COMPILE_FLAGS})

target_compile_options(${EXEC_NAME} PRIVATE ${MAIN_FLAGS})
target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Used for visual studio project creation
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
