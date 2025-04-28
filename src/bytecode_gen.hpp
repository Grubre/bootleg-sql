#pragma once

#include "IR.hpp"
#include <cstdint>
#include <vector>

enum class Opcode {
    NOOP,
    HALT,
    VERIFY_COOKIE,
    TRANSACTION,
    OPENWRITE,
    NEWRECNO,
    INTEGER,
    MAKERECORD,
    PUTINTKEY,
    CLOSE,
    COMMIT
};

struct Instruction {
    Opcode opcode;
    std::int64_t P1;
    std::uint64_t P2;
    std::string P3;
};

using SqlBytecodeProgram = std::vector<Instruction>;

inline auto generate_bytecode(const Statement& statement) -> SqlBytecodeProgram;
inline auto generate_bytecode(const SelectStmt& statement) -> SqlBytecodeProgram;
inline auto generate_bytecode(const CreateTableStmt& statement) -> SqlBytecodeProgram;
inline auto generate_bytecode(const InsertStmt& statement) -> SqlBytecodeProgram;
