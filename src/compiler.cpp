#include <iostream>
#include <iomanip>
#include <limits>

#include "common.hpp"
#include "chunk.hpp"
#include "compiler.hpp"
#include "scanner.hpp"


Parser::Parser(Scanner& scanner, Chunk& chunk)
    : scanner(scanner),
      compilingChunk(chunk)
{
}

void Parser::errorAt(const Token& token, std::string_view message)
{
    if (panicMode) return;
    panicMode = true;

    std::cerr << "[line " << token.line << "] Error";

    if (token.type == TOKEN_EOF)
    {
        std::cerr << " at end";
    }
    else if (token.type == TOKEN_ERROR)
    {
        // Nothing.
    }
    else
    {
        std::cerr << " at '" << token.lexeme << "'";
    }

    std::cerr << ": " << message << '\n';

    hadError = true;
}

void Parser::errorAtCurrent(std::string_view message) 
{
    errorAt(current, message);
}

void Parser::error(std::string_view message) 
{
    errorAt(previous, message);
}

void Parser::advance ()
{
    previous = current;
    
    while(true)
    {
        current = scanner.scanToken();
        if (current.type != TOKEN_ERROR) break;
        errorAtCurrent(current.lexeme);
    }
}

void Parser::consume(TokenType type, std::string_view message) 
{
    if (current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

Chunk& Parser::currentChunk()
{
    return compilingChunk;
}

void Parser::emitByte(std::uint8_t byte)
{
    writeChunk(currentChunk(), byte, previous.line);
}

void Parser::emitBytes(uint8_t byte1, uint8_t byte2) 
{
    emitByte(byte1);
    emitByte(byte2);
}

void Parser::emitReturn () 
{
    emitByte(OP_RETURN);
}

void Parser::endCompiler() 
{
    emitReturn();
}

// some ambiguous code (i do not understand how to arrange it).
// emiting part

void Parser::number()
{
    double value = std::stod(std::string(previous.lexeme));
    emitConstant(value);
}

void Parser::emitConstant(Value value) 
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

std::uint8_t Parser::makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);

    if (constant > std::numeric_limits<std::uint8_t>::max())
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return static_cast<std::uint8_t>(constant);
}

void Parser::grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Parser::unary() {
    TokenType operatorType = previous.type;

    expression();

    parsePrecedence(PREC_UNARY);

    switch (operatorType) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default:
        return; 
    }
}

void Parser::binary()
{
    // Remember the operator.
    TokenType operatorType = previous.type;

    // Compile the right operand.
    ParseRule* rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(rule->precedence + 1));

    // Emit the operator instruction.
    switch (operatorType)
    {
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;

        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;

        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;

        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;

        default:
            return; // Unreachable.
    }
}

// abe ye kya bkchodi hai?/???????

#include <array>

const std::array<ParseRule, TOKEN_EOF + 1> Parser::rules = {{
    { &Parser::grouping, nullptr,          PREC_NONE   }, // TOKEN_LEFT_PAREN
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_RIGHT_PAREN
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_LEFT_BRACE
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_RIGHT_BRACE
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_COMMA
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_DOT
    { &Parser::unary,    &Parser::binary,  PREC_TERM   }, // TOKEN_MINUS
    { nullptr,           &Parser::binary,  PREC_TERM   }, // TOKEN_PLUS
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_SEMICOLON
    { nullptr,           &Parser::binary,  PREC_FACTOR }, // TOKEN_SLASH
    { nullptr,           &Parser::binary,  PREC_FACTOR }, // TOKEN_STAR
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_BANG
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_BANG_EQUAL
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_EQUAL
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_EQUAL_EQUAL
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_GREATER
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_GREATER_EQUAL
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_LESS
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_LESS_EQUAL
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_IDENTIFIER
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_STRING
    { &Parser::number,   nullptr,          PREC_NONE   }, // TOKEN_NUMBER
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_AND
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_CLASS
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_ELSE
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_FALSE
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_FOR
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_FUN
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_IF
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_NIL
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_OR
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_PRINT
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_RETURN
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_SUPER
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_THIS
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_TRUE
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_VAR
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_WHILE
    { nullptr,           nullptr,          PREC_NONE   }, // TOKEN_ERROR
    { nullptr,           nullptr,          PREC_NONE   }  // TOKEN_EOF
}};


 const ParseRule& Parser::getRule(TokenType type) const
{
    return rules[type];
}

void Parser::parsePrecedence(Precedence precedence)
{
    advance();

    ParseFn prefixRule = getRule(previous.type).prefix;

    if (prefixRule == nullptr)
    {
        error("Expect expression.");
        return;
    }

    (this->*prefixRule)();

    while (precedence <= getRule(current.type).precedence) {
    advance();
    ParseFn infixRule = getRule(previous.type).infix;
    (this->*infixRule) ();
    
}

// the heart / engine of this module

bool Parser::compile()
{
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    endCompiler();
    return !hadError;
}

bool compile(const std::string& source, Chunk& chunk)
{
    Scanner scanner(source);
    Parser parser(scanner, chunk);

    return parser.compile();
}
