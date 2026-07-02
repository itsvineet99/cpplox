#include <array>

#include "common.hpp"
#include "compiler.hpp"
#include "debug.hpp"


Compiler::Compiler(Scanner& scanner, Chunk* chunk)
    : currentParser{},
      scanner(scanner),
      compilingChunk(chunk)
{
}

// parsing related

void Compiler::consume (TokenType type, std::string_view message)
{
    if (currentParser.current.type == type)
    {
        advance();
        return;
    }
    errorAtCurrent(message);
}

void Compiler::errorAt(const Token& token, std::string_view message)
{
    if (currentParser.panicMode) return;
    currentParser.panicMode = true;

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

    currentParser.hadError = true;
}

void Compiler::errorAtCurrent(std::string_view message) 
{
    errorAt(currentParser.current, message);
}

void Compiler::error(std::string_view message) 
{
    errorAt(currentParser.previous, message);
}

void Compiler::advance ()
{
    currentParser.previous = currentParser.current;
    
    while(true)
    {
        currentParser.current = scanner.scanToken();
        if (currentParser.current.type != TOKEN_ERROR) break;
        errorAtCurrent(currentParser.current.lexeme);
    }
}

const std::array<Compiler::ParseRule, TOKEN_EOF + 1> Compiler::rules = {{
    { &Compiler::grouping, nullptr,            PREC_NONE   }, // TOKEN_LEFT_PAREN
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_RIGHT_PAREN
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_LEFT_BRACE
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_RIGHT_BRACE
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_COMMA
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_DOT
    { &Compiler::unary,    &Compiler::binary,  PREC_TERM   }, // TOKEN_MINUS
    { nullptr,             &Compiler::binary,  PREC_TERM   }, // TOKEN_PLUS
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_SEMICOLON
    { nullptr,             &Compiler::binary,  PREC_FACTOR }, // TOKEN_SLASH
    { nullptr,             &Compiler::binary,  PREC_FACTOR }, // TOKEN_STAR
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_BANG
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_BANG_EQUAL
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_EQUAL
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_EQUAL_EQUAL
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_GREATER
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_GREATER_EQUAL
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_LESS
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_LESS_EQUAL
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_IDENTIFIER
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_STRING
    { &Compiler::number,   nullptr,            PREC_NONE   }, // TOKEN_NUMBER
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_AND
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_CLASS
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_ELSE
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_FALSE
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_FOR
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_FUN
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_IF
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_NIL
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_OR
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_PRINT
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_RETURN
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_SUPER
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_THIS
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_TRUE
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_VAR
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_WHILE
    { nullptr,             nullptr,            PREC_NONE   }, // TOKEN_ERROR
    { nullptr,             nullptr,            PREC_NONE   }  // TOKEN_EOF
}};

const Compiler::ParseRule& Compiler::getRule(TokenType type) const
{
    return rules[type];
}

void Compiler::parsePrecedence(Precedence precedence)
{
    advance();

    ParseFn prefixRule = getRule(currentParser.previous.type).prefix;

    if (prefixRule == nullptr)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;

    (this->*prefixRule)(canAssign);

    while (precedence <= getRule(currentParser.current.type).precedence)
    {
        advance();
        ParseFn infixRule = getRule(currentParser.previous.type).infix;
        
        (this->*infixRule)(canAssign);
    }

    if (canAssign && currentParser.current.type == TOKEN_EQUAL)
    {
        advance(); 
        error("Invalid assignment target.");
    }
}

void Compiler::expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

// functions to call when tokentype is matched

void Compiler::grouping (bool canAssign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary (bool canAssign) 
{
    TokenType operatorType = currentParser.previous.type;

    parsePrecedence(PREC_UNARY);

    switch (operatorType) 
    {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return; 
    }
}

void Compiler::binary(bool canAssign)
{
    TokenType operatorType = currentParser.previous.type;

    ParseFn rule = getRule(operatorType).infix;
    Precedence prec = getRule(operatorType).precedence;

    parsePrecedence(static_cast<Precedence>(prec + 1));

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

void Compiler::number (bool canAssign)
{
    double value = std::stod(std::string(currentParser.previous.lexeme));
    emitConstant(value);
}

// emiting shii / bytecode generation

Chunk* Compiler::currentChunk() 
{
    return compilingChunk;
}

std::uint8_t Compiler::makeConstant(Value value)
{
    int constant = addConstant(*currentChunk(), value);

    if (constant > std::numeric_limits<std::uint8_t>::max())
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return static_cast<std::uint8_t>(constant);
}

void Compiler::emitConstant (Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

void Compiler::emitByte(std::uint8_t byte) 
{
    writeChunk(*currentChunk(), byte, currentParser.previous.line);
}
void Compiler::emitBytes(std::uint8_t byte1, std::uint8_t byte2) 
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitReturn () 
{
    emitByte(OP_RETURN);
}

void Compiler::endCompiler() 
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!currentParser.hadError) {
        disassembleChunk(*currentChunk(), "code");
    }
#endif
}

bool Compiler::compile()
{
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    endCompiler();
    return !currentParser.hadError;
}
