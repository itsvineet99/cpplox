#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "common.hpp"
#include "vm.hpp"

void repl(VM& vm)
{
    std::string line;
    
    while (true)
    {
        std::cout << "> ";
        
        if (!std::getline(std::cin, line))
        {
            std::cout << '\n';
            break;
        }

        vm.interpret(line);
    }
}

void runFile(VM& vm, const std::string& path)
{
    std::ifstream file(path);

    if(!file)
    {
        std::cerr << "Could not open file \"" << path << "\".\n";
        std::exit(74);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string source = buffer.str();

    InterpretResult result = vm.interpret(source);

    if (result == INTERPRET_COMPILE_ERROR){
        std::exit(65);
    }
    if (result == INTERPRET_RUNTIME_ERROR){
        std::exit(70);
    }
}

int main(int argc, char* argv[]) 
{
    VM vm;
    
    if (argc == 1)
    {
        repl(vm);
    }
    else if (argc == 2)
    {
        runFile(vm, argv[1]);
    }
    else
    {
        std::cerr << "Usage: cpplox [path]\n";
        return 64;
    }

    return 0;
}
