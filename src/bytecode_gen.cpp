#include "bytecode_gen.hpp"
#include "common.hpp"

inline auto generate_bytecode(const SelectStmt& statement) -> SqlBytecodeProgram {

}
inline auto generate_bytecode(const CreateTableStmt& statement) -> SqlBytecodeProgram {

}
inline auto generate_bytecode(const InsertStmt& statement) -> SqlBytecodeProgram {
    SqlBytecodeProgram program{};
    /* example:
    sqlite> explain insert into main values(1,2);
            0|Transaction|0|0|
            1|VerifyCookie|234|0|
            2|OpenWrite|0|3|main
            3|NewRecno|0|0|
            4|Integer|1|0|1
            5|Integer|2|0|2
            6|MakeRecord|2|0|
            7|PutIntKey|0|1|
            8|Close|0|0|
            9|Commit|0|0|
    */

    program.push_back(Instruction(Opcode::TRANSACTION, 0, 0, {}));
    program.push_back(Instruction(Opcode::VERIFY_COOKIE, 0, 0, {}));
    program.push_back(Instruction(Opcode::NEWRECNO, 0, 0, {}));
    program.push_back(Instruction(Opcode::OPENWRITE, 0, 0, statement.table.table.table_name));

    program.push_back(Instruction(Opcode::INTEGER, 0, 0, {}));
    program.push_back(Instruction(Opcode::INTEGER, 0, 0, {}));
    program.push_back(Instruction(Opcode::MAKERECORD, 0, 0, {}));

    program.push_back(Instruction(Opcode::PUTINTKEY, 0, 1, {}));
    program.push_back(Instruction(Opcode::CLOSE, 0, 0, {}));
    program.push_back(Instruction(Opcode::COMMIT, 0, 0, {}));

    return program;
}

auto generate_bytecode(const Statement& statement) -> SqlBytecodeProgram {
    return std::visit(overloaded{
        [](const SelectStmt& stmt) {
            return generate_bytecode(stmt);
        },
        [](const CreateTableStmt& stmt) {
            return generate_bytecode(stmt);
        },
        [](const InsertStmt& stmt) {
            return generate_bytecode(stmt);
        }
    }, statement);

}
