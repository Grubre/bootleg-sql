set (EXEC_NAME "db")
set (ANTLR_TARGET_NAME "Parser")

antlr_target(${ANTLR_TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/grammar/Grammar.g4 BOTH VISITOR)

add_executable(${EXEC_NAME}
  main.cpp
  printers.cpp
  ${ANTLR_${ANTLR_TARGET_NAME}_CXX_OUTPUTS}
)

target_include_directories(${EXEC_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(${EXEC_NAME} PUBLIC
    "${antlr_SOURCE_DIR}/runtime/Cpp/runtime/src"
    ${ANTLR_${ANTLR_TARGET_NAME}_OUTPUT_DIR}
)

target_link_libraries(${EXEC_NAME} PRIVATE Expected fmt reflex antlr4_static)

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES})
