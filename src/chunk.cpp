#include "chunk.hpp"
#include "memory.h"

void writeChunk(Chunk& chunk, std::uint8_t byte) 
{
    chunk.code.push_back(byte);
}

int addConstant(Chunk& chunk, Value value) 
{
    chunk.constants.values.push_back(value);
    return static_cast<int>(
        chunk.constants.values.size() - 1
    );
}
