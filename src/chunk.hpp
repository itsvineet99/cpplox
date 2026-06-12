#pragma once

#include "common.hpp"
#include "values.hpp"
#include <vector>

enum OpCode {
    OP_CONSTANT,
    OP_RETURN
};

struct Chunk
{
    std::vector<std::uint8_t> code;
    std::vector<int> lines;
    ValueArray constants;
};

void writeChunk(Chunk& chunk, std::uint8_t byte, int line);
int addConstant(Chunk& chunk, Value value);
