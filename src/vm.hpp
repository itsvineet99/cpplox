#pragma once

#include "chunk.hpp"
#include "values.hpp"

enum InterpretResult
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM
{
public: 
    InterpretResult interpret(const std::string& source);

private:
    InterpretResult run();

    template<typename F>
    void binaryOp(F operation);

    void push(Value value);
    Value pop();

    std::uint8_t readByte();
    Value readConstant();
    void resetStack();

    Chunk* chunk = nullptr;
    std::size_t ip = 0;
    std::vector<Value> stack;
};
