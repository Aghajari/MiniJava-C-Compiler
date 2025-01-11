#ifndef SIMPLEJAVALEXER_NUMBER_HELPER_H
#define SIMPLEJAVALEXER_NUMBER_HELPER_H

#include <iostream>

/**
 * Checks if a character is a decimal digit (0-9).
 * @param c - The character to check.
 * @return true if the character is a digit; false otherwise.
 */
bool isNumber(char c);

/**
 * Checks if a character is valid in a binary or hexadecimal number.
 * @param c - The character to check.
 * @param binary - true to check for binary digits (0 or 1), false for hexadecimal digits (0-9, a-f, A-F).
 * @return true if the character is valid for the specified number type; false otherwise.
 */
bool isHexOrBinaryNumber(char c, bool binary);

/**
 * Checks if a character is a valid type identifier for a number in Java.
 * Type identifiers include 'f', 'F', 'd', 'D', and optionally 'l', 'L' (for long numbers).
 * @param c - The character to check.
 * @param supportsLong - true if 'l' and 'L' should be considered valid identifiers.
 * @return true if the character is a valid number type identifier; false otherwise.
 */
bool isNumberTypeIdentifier(char c, bool supportsLong);

/**
 * Determines if two characters could start a number.
 * Examples:
 * - '1' or '3' are valid starters.
 * - '.' is valid only if the next character is a digit.
 * @param c - The current character.
 * @param next_c - The next character in the sequence.
 * @return true if the characters can start a number; false otherwise.
 */
bool isNumberStarter(char c, char next_c);

/**
 * Checks if a character is valid within the range of a binary, hexadecimal, or decimal number.
 * @param c - The character to check.
 * @param isBinary - true if checking for a binary number.
 * @param isHex - true if checking for a hexadecimal number.
 * @return true if the character is valid in the specified range; false otherwise.
 */
bool isNumberInRange(char c, bool isBinary, bool isHex);

/**
 * Consumes underscores in a number sequence and validates their placement.
 *
 * This function checks for underscores (`_`) in numeric literals, ensuring they are correctly placed
 * and conform to Java's syntax rules. Unlike the previous implementation, this optimized version
 * avoids quadratic complexity by consuming all forward underscores at once and validating their usage
 * in a single pass.
 *
 * Key Behavior:
 * - The first `_` encountered must follow a valid numeric character.
 * - Subsequent underscores are consumed in a single forward traversal.
 * - The function ensures there is at least one valid numeric character after the underscores.
 *
 * @param source The source code string being tokenized.
 * @param index The current position of the `_` in the string.
 * @param isBinary A flag indicating whether the number is in binary format.
 * @param isHex A flag indicating whether the number is in hexadecimal format.
 * @return A string containing the consumed valid numeric segment with underscores, or an empty
 *         string if the underscores are invalid.
 */
std::string consumeUnderscoreInNumber(const std::string &source, int index, bool isBinary, bool isHex);

#endif //SIMPLEJAVALEXER_NUMBER_HELPER_H
