#ifndef SIMPLEJAVALEXER_TOKEN_H
#define SIMPLEJAVALEXER_TOKEN_H

#include <iostream>

/**
 * Enum representing the types of tokens in the lexer.
 */
enum TokenType {
    KEYWORD,         // Java keywords (e.g., "class", "if", "while").
    LINE_COMMENT,    // Single-line comments (e.g., "// comment").
    BLOCK_COMMENT,   // Multi-line comments (e.g., "/* comment */").
    STRING,          // String literals (e.g., "\"hello\"").
    CHAR,            // Character literals (e.g., '\'a\'').
    IDENTIFIER,      // Identifiers (e.g., variable or method names).
    ANNOTATION,      // Java annotations (e.g., "@Override").
    NUMBER,          // Numeric literals (e.g., "123").
    HEX_NUMBER,      // Hexadecimal numbers (e.g., "0x1A3F").
    BINARY_NUMBER,   // Binary numbers (e.g., "0b1010").
    OPERATOR,        // Operators (e.g., "+", "==", "&&").
    SYMBOL,          // Symbols (e.g., ";", "{", "}").
    WHITESPACE,      // Whitespace (spaces, tabs, newlines).
    UNKNOWN          // Unrecognized tokens.
};

/**
 * Struct representing the position of a token in the source code.
 * This includes the index, line number, and column number.
 */
struct Position {
    int index;  // Absolute index in the source code.
    int line;   // Line number in the source code (1-based).
    int column; // Column number in the source code (1-based).
};

/**
 * Class representing a token produced by the lexer.
 */
class Token {
public:
    TokenType type;           // The type of the token.
    std::string lexeme;       // The actual text of the token.
    struct Position position; // The position of the token in the source code.

    Token(
            TokenType type,
            std::string lex,
            struct Position position,
            int indexOffset
    ) : type(type),
        lexeme(std::move(lex)),
        position(position) {
        this->position.index -= (int) indexOffset;
    }

    Token(
            TokenType type,
            std::string lex
    ) : type(type),
        lexeme(std::move(lex)),
        position({-1}) {}

    /**
     * Returns the name of the token type as a string.
     * @return A string representing the token type.
     */
    [[nodiscard]] std::string getTokenTypeName() const {
        switch (type) {
            case TokenType::KEYWORD :
                return "KEYWORD";
            case TokenType::LINE_COMMENT:
                return "LINE_COMMENT";
            case TokenType::BLOCK_COMMENT:
                return "BLOCK_COMMENT";
            case TokenType::STRING:
                return "STRING";
            case TokenType::CHAR:
                return "CHAR";
            case TokenType::IDENTIFIER:
                return "IDENTIFIER";
            case TokenType::ANNOTATION:
                return "ANNOTATION";
            case TokenType::NUMBER:
                return "NUMBER";
            case TokenType::HEX_NUMBER:
                return "HEX_NUMBER";
            case TokenType::BINARY_NUMBER:
                return "BINARY_NUMBER";
            case TokenType::OPERATOR:
                return "OPERATOR";
            case TokenType::SYMBOL:
                return "SYMBOL";
            case TokenType::WHITESPACE:
                return "WHITESPACE";
            case TokenType::UNKNOWN:
                return "UNKNOWN";
        }
    }

    friend std::ostream &operator<<(std::ostream &strm, const Token &token) {
        if (token.position.index == -1) {
            return strm << "Token{Type: " << token.getTokenTypeName()
                        << ", Lexeme: '" << token.lexeme
                        << "'}";
        }

        return strm << "Token{Type: " << token.getTokenTypeName()
                    << ", Position: " << token.position.line
                    << ":" << token.position.column
                    << ", Lexeme: '" << token.lexeme
                    << "'}";
    }
};

#endif //SIMPLEJAVALEXER_TOKEN_H
