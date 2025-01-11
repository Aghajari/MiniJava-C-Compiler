#include "../include/token.h"
#include "../include/token_matcher.h"
#include <regex>

// https://en.wikipedia.org/wiki/List_of_Java_keywords
const std::vector<std::string> java_keywords = {
        "abstract", "assert", "boolean", "break",
        "byte", "case", "catch", "char", "class",
        "continue", "default", "do", "double",
        "else", "enum", "extends", "final", "if",
        "finally", "float", "for", "implements",
        "import", "instanceof", "int", "interface",
        "long", "native", "new", "package", "private",
        "protected", "public", "return", "short",
        "static", "super", "switch", "synchronized",
        "this", "throw", "throws", "transient", "try",
        "void", "volatile", "while", "goto", "@interface",
        "true", "false", "null", "const", "strictfp", "_"
};

const std::vector<char> java_operators_starter = {
        '=', '!', '<', '>', '+', '-', '*', '/',
        '&', '~', '|', '%', '^',
};

const std::vector<std::string> java_operators = {
        "<", ">", ">=", "<=",
        "<<", ">>", "<<=", ">>=",
        "+", "+=", "++",
        "-", "-=", "--",
        "&", "&=", "&&",
        "|", "|=", "||",
        "=", "==",
        "*", "*=",
        "/", "/=",
        "%", "%=",
        "~", "~=",
        "!", "!=",
        "^", "^=",
};

const std::vector<std::string> java_symbols = {
        ";", "->", "{", "}", "[", "]", "(", ")", ",", "@", ".", "?", ":"
};

const std::regex identifier_letter_regex = std::regex(R"([a-zA-Z0-9_$])");
const std::regex identifier_regex = std::regex(R"([a-zA-Z_$][a-zA-Z0-9_$]*)");
const std::regex whitespace_regex = std::regex(R"(\s+)");

template<class InputIterator, class Type>
bool find(std::vector<InputIterator> vector, const Type &value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

bool isKeyword(const std::string &token) {
    return find(java_keywords, token);
}

bool isOperator(const std::string &token) {
    return find(java_operators, token);
}

bool isSymbol(const std::string &token) {
    return find(java_symbols, token);
}

bool isIdentifierLetter(const std::string &token) {
    return std::regex_match(token, identifier_letter_regex);
}

bool isIdentifier(const std::string &token) {
    return std::regex_match(token, identifier_regex);
}

bool isWhitespace(const std::string &token) {
    return std::regex_match(token, whitespace_regex);
}

bool isOperatorStart(const std::string &token) {
    return find(java_operators_starter, token[0]);
}

TokenType getTokenType(const std::string &token) {
    if (isKeyword(token)) {
        return KEYWORD;
    } else if (isOperator(token)) {
        return OPERATOR;
    } else if (isIdentifier(token)) {
        return IDENTIFIER;
    } else if (isWhitespace(token)) {
        return WHITESPACE;
    } else if (isSymbol(token)) {
        return SYMBOL;
    } else if (token[0] == '@') {
        std::string sub = token.substr(1);
        if (isKeyword(sub) || !isIdentifier(sub)) {
            return UNKNOWN;
        }
        return ANNOTATION;
    } else {
        return UNKNOWN;
    }
}