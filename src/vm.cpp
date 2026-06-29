#include "common.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "vm.hpp"

InterpretResult VM::interpret(const std::string& source)
{
    compile(source);
    return INTERPRET_OK;
}

std::uint8_t VM::readByte()
{
    return chunk->code[ip++];
}

Value VM::readConstant()
{
    return chunk->constants.values[readByte()];
}

void VM::resetStack()
{
    stack.clear();
}

void VM::push (Value value)
{
    stack.push_back(value);
}

Value VM::pop ()
{
    Value value = stack.back();
    stack.pop_back();
    return value;
}

template<typename F>
void VM::binaryOp(F operation)
{
    Value b = pop();
    Value a = pop();

    push(operation(a,b));
}

InterpretResult VM::run()
{
    while (true)
    {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << ' ';

        for (const Value& value : stack)
        {
            std::cout << "[ ";
            printValue(value);
            std::cout << " ]";
        }

        std::cout << '\n';

        disassembleInstruction(*chunk, static_cast<int>(ip));
#endif
        auto instruction = static_cast<OpCode>(chunk->code[ip++]);

        switch (instruction)
        {
            case OP_CONSTANT:
            {
                Value constant = readConstant();
                push(constant);
                break;
            }
            case OP_ADD:
            { 
                binaryOp([](Value a, Value b) { return a + b; });
                break;
            }
            case OP_SUBTRACT: 
            { 
                binaryOp([](Value a, Value b) { return a - b; });
                break;
            }
            case OP_MULTIPLY: 
            { 
                binaryOp([](Value a, Value b) { return a * b; });
                break;
            }
            case OP_DIVIDE: 
            { 
                binaryOp([](Value a, Value b) { return a / b; });
                break;
            }
            case OP_NEGATE:
            {
                push(-pop());
                break;
            }
            case OP_RETURN:
            {
                printValue(pop());
                std::cout << "\n";
                return INTERPRET_OK;
            }
            default:
            {
                return INTERPRET_RUNTIME_ERROR;
            }
        }
    }
}
