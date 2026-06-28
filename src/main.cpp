#include "common.hpp"
#include "chunk.hpp"
#include "debug.hpp"
#include "vm.hpp"

int main() 
{
    VM vm;
    Chunk chunk;

    int constant = addConstant(chunk, 1.2);
    writeChunk(chunk, OP_CONSTANT, 123);
    writeChunk(chunk, constant, 123);

    constant = addConstant(chunk, 3.4);

    writeChunk(chunk, OP_CONSTANT, 123);
    writeChunk(chunk, static_cast<std::uint8_t>(constant), 123);

    writeChunk(chunk, OP_ADD, 123);

    constant = addConstant(chunk, 5.6);
    writeChunk(chunk, OP_CONSTANT, 123);
    writeChunk(chunk, static_cast<std::uint8_t>(constant), 123);

    writeChunk(chunk, OP_DIVIDE, 123);
    writeChunk(chunk, OP_NEGATE, 123);

    writeChunk(chunk, OP_RETURN, 123);

    vm.interpret(chunk);

    disassembleChunk(chunk, "test chunk");

    return 0;
}
