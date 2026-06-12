#include "chunk.hpp"
#include "memory.h"

void writeChunk(Chunk& chunk, std::uint8_t byte, int line) 
{
    chunk.code.push_back(byte);
    chunk.lines.push_back(line);
}

int addConstant(Chunk& chunk, Value value) 
{
    chunk.constants.values.push_back(value);
    return static_cast<int>(
        chunk.constants.values.size() - 1
    );
}
