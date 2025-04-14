set (EXEC_NAME "db")

# bison
find_package(BISON REQUIRED)
BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)

# reflex
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp ${CMAKE_CURRENT_BINARY_DIR}/lexer.hpp
  COMMAND reflex --bison -o ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
          --header-file=${CMAKE_CURRENT_BINARY_DIR}/lexer.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
  COMMENT "Generating lexer.cpp and lexer.hpp with RE/flex"
  VERBATIM
)

add_custom_target(LexerGen DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

add_executable(${EXEC_NAME}
  main.cpp
  ${BISON_Parser_OUTPUTS}
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

add_dependencies(${EXEC_NAME} LexerGen)

target_include_directories(${EXEC_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${EXEC_NAME} PRIVATE Expected fmt reflex)

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES})
