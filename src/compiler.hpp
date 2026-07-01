#pragma once

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


class Parser
{
public:
    explicit Parser(Scanner& scanner, Chunk& chunk);

    bool compile();

private:
    Scanner& scanner;

    Token current;
    Token previous;
    Chunk& compilingChunk;

    bool hadError = false;
    bool panicMode = false;

    void advance();
    void errorAtCurrent(std::string_view message);
    void error(std::string_view message);
    void errorAt(const Token& token, std::string_view message);
    void consume(TokenType type, std::string_view message);
    void emitByte(std::uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    Chunk& currentChunk();
    void emitReturn();
    void endCompiler();
    void number();
    void emitConstant(Value value);
    std::uint8_t makeConstant(Value value);
    void grouping();
    void unary();
    void parsePrecedence(Precedence precedence);
    void binary();

    static const std::array<ParseRule, TOKEN_EOF + 1> rules;
    const ParseRule& getRule(TokenType type) const;
};


using ParseFn = void (Parser::*)();

struct ParseRule
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

bool compile(const std::string& source, Chunk& chunk);
