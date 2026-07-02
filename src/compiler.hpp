#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "chunk.hpp"
#include "common.hpp"
#include "scanner.hpp"

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY
};

struct Parser {
    Token current;
    Token previous;
    bool hadError = false;
    bool panicMode = false;
};

class Compiler {
public:
    using ParseFn = void (Compiler::*)(bool canAssign);

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    explicit Compiler(Scanner& scanner, Chunk* chunk);

    bool compile();

private:
    Parser currentParser;   // State object for tokens and errors
    Scanner& scanner;       // Reference to tokenizer
    Chunk* compilingChunk;  // Pointer to the chunk being emitted to

    // Core parsing and tokens
    void advance();
    void consume(TokenType type, std::string_view message);
    void parsePrecedence(Precedence precedence);
    
    // Error handling (modifies currentParser state)
    void errorAtCurrent(std::string_view message);
    void error(std::string_view message);
    void errorAt(const Token& token, std::string_view message);

    // Bytecode emission
    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
    void emitReturn();
    void emitConstant(Value value);
    std::uint8_t makeConstant(Value value);
    Chunk* currentChunk();
    void endCompiler();

    // Pratt parser functions matching the ParseFn signature
    void expression();
    void number(bool canAssign);
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);

    static const std::array<ParseRule, TOKEN_EOF + 1> rules;
    
    const ParseRule& getRule(TokenType type) const;
};

