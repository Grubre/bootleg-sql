BUILD_DIR := "build"
SRC_DIR := "src"
TEST_DIR := "tests"
EXEC_NAME := "db"

default: run

build BUILD_TYPE="Debug":
    mkdir -p {{BUILD_DIR}} && cd {{BUILD_DIR}} && \
    cmake -DCMAKE_BUILD_TYPE={{BUILD_TYPE}} .. && \
    cmake --build . --config {{BUILD_TYPE}}

run *ARGS: (build)
    ./build/src/{{EXEC_NAME}} {{ARGS}}

test *ARGS: (build)
    cd build && ctest {{ARGS}}

debug *ARGS: (build "Debug")
    gdb --args ./build/src/{{EXEC_NAME}} {{ARGS}}

clean:
    rm -rf {{BUILD_DIR}}

