#include "../include/error_handler.h"

void error(const std::string &message) {
    throw std::runtime_error(message);
}

void error(const std::string &message, Token *token) {
    if (token == nullptr) {
        throw std::runtime_error(message);
    }

    std::string token_str = "Token{Type: " + token->getTokenTypeName()
                            + ", Position: " + std::to_string(token->position.line)
                            + ":" + std::to_string(token->position.column)
                            + ", Lexeme: '" + token->lexeme
                            + "'}";
    throw std::runtime_error(message + " at " + token_str);
}