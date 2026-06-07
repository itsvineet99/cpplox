#include "common.hpp"
#include "chunk.hpp"
#include "debug.hpp"

int main() 
{
    Chunk chunk;

    int constant = addConstant(chunk, 1.2);
    writeChunk(chunk, OP_CONSTANT);
    writeChunk(chunk, constant);
    writeChunk(chunk, OP_RETURN);

    disassembleChunk(chunk, "test chunk");

    return 0;
}
