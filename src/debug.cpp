#include <iostream>
#include <iomanip>
#include <string>
#include "debug.hpp"
#include "values.hpp"

void disassembleChunk(const Chunk& chunk, const std::string& name) 
{
    std::cout << "==" << name << "==\n";

    int offset = 0;

    while(offset < static_cast<int>(chunk.code.size()))
    {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(const std::string& name, 
                               const Chunk& chunk, 
                               int offset)
{
    std::uint8_t constant = chunk.code[offset + 1];

    std::cout
        << std::left
        << std::setw(16)
        << name
        << std::right
        << ' '
        << std::setw(4)
        << static_cast<int>(constant)
        << " '";

    printValue(chunk.constants.values[constant]);

    std::cout << "'\n";

    return offset + 2;
}

static int simpleInstruction(const std::string& name, int offset) 
{
    std::cout << name << "\n";
    return offset + 1;
}

int disassembleInstruction(const Chunk& chunk, int offset)
{
    std::cout << std::setw(4)
              << std::setfill('0')
              << offset
              << std::setfill(' ')
              << ' ';

    auto instruction = static_cast<OpCode>(chunk.code[offset]);

    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);

        default:
            std::cout
                << "Unknown opcode "
                << static_cast<int>(
                        chunk.code[offset]
                    )
                << '\n';
            return offset + 1;
    }
}
