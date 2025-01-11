#ifndef SIMPLEJAVALEXER_TOKEN_MATCHER_H
#define SIMPLEJAVALEXER_TOKEN_MATCHER_H

#include <iostream>

/**
 * Checks if the given token is a Java keyword.
 * @param token - The string to check.
 * @return true if the token is a valid Java keyword; false otherwise.
 */
bool isKeyword(const std::string &token);

/**
 * Checks if the given token is a Java operator.
 * @param token - The string to check.
 * @return true if the token is a valid Java operator; false otherwise.
 */
bool isOperator(const std::string &token);

/**
 * Checks if the given token is a Java symbol (e.g., semicolons, braces, etc.).
 * @param token - The string to check.
 * @return true if the token is a valid Java symbol; false otherwise.
 */
bool isSymbol(const std::string &token);

/**
 * Checks if the given token is a valid identifier character in Java (letters, digits, '_', or '$').
 * @param token - The string to check.
 * @return true if the token matches the rules for an identifier character; false otherwise.
 */
bool isIdentifierLetter(const std::string &token);


/**
 * Checks if the given token is a valid Java identifier.
 * A valid identifier starts with a letter, '_' or '$', and can be followed by letters, digits, '_' or '$'.
 * @param token - The string to check.
 * @return true if the token matches the rules for a valid Java identifier; false otherwise.
 */
bool isIdentifier(const std::string &token);

/**
 * Checks if the given token consists of whitespace characters (spaces, tabs, etc.).
 * @param token - The string to check.
 * @return true if the token contains only whitespace characters; false otherwise.
 */
bool isWhitespace(const std::string &token);

/**
 * Checks if the given token starts with a character that could initiate a Java operator.
 * @param token - The string to check.
 * @return true if the first character of the token matches a potential operator starter; false otherwise.
 */
bool isOperatorStart(const std::string &token);

/**
 * Determines the type of the given token based on Java syntax rules.
 * @param token - The string to classify.
 * @return The token's type as a `TokenType` enum value (e.g.,
 *  KEYWORD, OPERATOR, IDENTIFIER, WHITESPACE, SYMBOL, ANNOTATION, or UNKNOWN).
 */
TokenType getTokenType(const std::string &token);

#endif //SIMPLEJAVALEXER_TOKEN_MATCHER_H
