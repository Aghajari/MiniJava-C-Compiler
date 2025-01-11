//
// Created by Aghajari on 12/8/24.
//

#ifndef SIMPLEJAVALEXER_LEXER_H
#define SIMPLEJAVALEXER_LEXER_H

#include <iostream>
#include "token.h"

/**
 * Tokenizes the given Java source code into a sequence of tokens.
 * @param source - The Java source code as a string.
 * @return A vector of tokens representing the lexical elements of the source code.
 */
std::vector<Token> tokenize(const std::string &source);

#endif //SIMPLEJAVALEXER_LEXER_H
