#include <iostream>
#include "../include/lexer.h"
#include "../include/token_matcher.h"
#include "../include/number_helper.h"

/**
 * Enum representing the possible states of the tokenizer during lexical analysis.
 */
enum TokenizerState {
    STATE_NONE,            // No specific state; beginning or resetting tokenization.
    STATE_WORD,            // Reading an identifier or keyword.
    STATE_LINE_COMMENT,    // Inside a single-line comment.
    STATE_BLOCK_COMMENT,   // Inside a multi-line block comment.
    STATE_LITERAL_STRING,  // Inside a string literal.
    STATE_LITERAL_CHAR,    // Inside a character literal.
    STATE_OPERATORS,       // Parsing operators.
    STATE_NUMBERS,         // Parsing numbers.
    STATE_HEX,             // Parsing a hexadecimal number.
    STATE_BINARY,          // Parsing a binary number.
};

/**
 * Struct representing additional information when parsing numbers.
 */
struct NumberInfo {
    bool hasUsedDot;  // True if the number contains a decimal point.
    bool hasUsedE;    // True if the number contains an exponent ('e' or 'E').
};

/**
 * Handles the initial classification of a character and transitions to the appropriate tokenizer state.
 *
 * This function is used when the lexer is in the `STATE_NONE` state, meaning no active token is being processed.
 * Based on the input character and its context (e.g., the next character), it determines whether to start a new token,
 * emit an immediate token, or transition to a specific processing state.
 *
 * Key Responsibilities:
 * - Comment Detection:
 *   - Detects single-line comments starting with `//` and transitions to `STATE_LINE_COMMENT`.
 *   - Detects multi-line comments starting with `/ *` and transitions to `STATE_BLOCK_COMMENT`.
 * - Literal Detection:
 *   - Detects string literals starting with `"` and transitions to `STATE_LITERAL_STRING`.
 *   - Detects character literals starting with `'` and transitions to `STATE_LITERAL_CHAR`.
 * - Number Detection:
 *   - Detects hexadecimal numbers starting with `0x` or `0X` and transitions to `STATE_HEX`.
 *   - Detects binary numbers starting with `0b` or `0B` and transitions to `STATE_BINARY`.
 *   - Detects regular or floating-point numbers and transitions to `STATE_NUMBERS`.
 * - Other Token Types:
 *   - Processes operators and symbols like `+`, `->`, `;` by emitting tokens or transitioning to `STATE_OPERATORS`.
 *   - Recognizes whitespace and emits a `WHITESPACE` token.
 *   - Identifies identifiers (e.g., variable names, keywords) and transitions to `STATE_WORD`.
 *
 * @param tokens Reference to the vector where generated tokens are stored.
 * @param position Current position in the source code, used to track the location of the token.
 * @param word Accumulator for the current token being processed.
 * @param state Current tokenizer state, which will be updated based on the character type.
 * @param c Current character being analyzed.
 * @param next_c Next character in the input sequence, used for lookahead operations.
 */
void consume(
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c,
        char next_c
) {
    word = c;
    if (c == '/' && next_c == '/') {
        state = TokenizerState::STATE_LINE_COMMENT;
    } else if (c == '/' && next_c == '*') {
        state = TokenizerState::STATE_BLOCK_COMMENT;
    } else if (c == '"') {
        state = TokenizerState::STATE_LITERAL_STRING;
    } else if (c == '\'') {
        state = TokenizerState::STATE_LITERAL_CHAR;
    } else if (c == '0' && (next_c == 'x' || next_c == 'X')) {
        state = TokenizerState::STATE_HEX;
    } else if (c == '0' && (next_c == 'b' || next_c == 'B')) {
        state = TokenizerState::STATE_BINARY;
    } else if (c == '@' && isIdentifierLetter(std::string{next_c})) { // NOLINT(bugprone-branch-clone)
        state = TokenizerState::STATE_WORD;
    } else if (isNumberStarter(c, next_c)) {
        state = TokenizerState::STATE_NUMBERS;
    } else if (isOperatorStart(word)) {
        state = TokenizerState::STATE_OPERATORS;
    } else if (isSymbol(word)) {
        bool isDoubleColon = c == ':' && next_c == ':';
        if (isDoubleColon) {
            word += next_c;
            position.index++;
        }
        tokens.emplace_back(TokenType::SYMBOL, word, position, 1);
        position.column += (int) word.size();
        word = "";
    } else if (isWhitespace(word)) {
        if (!tokens.empty() && tokens.back().type != WHITESPACE) {
            tokens.emplace_back(TokenType::WHITESPACE, word, position, 0);
        }
        position.column += (int) word.size();
        word = "";
    } else if (isIdentifier(word)) {
        state = TokenizerState::STATE_WORD;
    } else {
        tokens.emplace_back(TokenType::UNKNOWN, word, position, 0);
        position.column += (int) word.size();
        word = "";
    }
}

/**
 * Processes words, including identifiers, keywords, or annotations.
 *
 * This function is used when the lexer is in the `STATE_WORD` state. It accumulates characters
 * that form valid Java identifiers, keywords, or annotations and finalizes the token when
 * encountering a character that is not part of an identifier.
 *
 * Key Responsibilities:
 * - Accumulate valid identifier characters (`[a-zA-Z0-9_$]`).
 * - Finalize and emit tokens for completed words.
 * - Transition back to `STATE_NONE` after emitting the token.
 *
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word The current word being processed.
 * @param state Current tokenizer state.
 * @param c Current character.
 * @return true if the character was consumed as part of the word; false otherwise.
 */
bool consumeWord(
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c
) {
    std::string c_str{c};
    if (isIdentifierLetter(c_str)) {
        word += c;
        return true;
    } else {
        tokens.emplace_back(getTokenType(word), word, position, word.size());
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
        return false;
    }
}

/**
 * Processes single-line comments (e.g., `// comment`).
 *
 * This function is used when the lexer is in the `STATE_LINE_COMMENT` state. It accumulates
 * characters until the end of the line (`\n`) is reached and then emits the comment token.
 *
 * Key Responsibilities:
 * - Accumulate all characters in the comment until a newline is encountered.
 * - Finalize the comment token at the end of the line.
 * - Transition back to `STATE_NONE` after emitting the token.
 *
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word Accumulated characters of the comment.
 * @param state Current tokenizer state.
 * @param c Current character.
 */
void consumeLineComment(
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c
) {
    if (c == '\n') {
        tokens.emplace_back(TokenType::LINE_COMMENT, word, position, word.size());
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
    } else {
        word += c;
    }
}

/**
 * Processes multi-line block comments (e.g., `/ * comment * /`).
 *
 * This function is used when the lexer is in the `STATE_BLOCK_COMMENT` state. It accumulates
 * characters until the closing sequence (`* /`) is detached. The start position of the comment
 * is preserved to ensure accurate token metadata.
 *
 * Key Responsibilities:
 * - Accumulate all characters in the block comment.
 * - Detect the closing sequence (`* /`) to finalize the comment token.
 * - Transition back to `STATE_NONE` after emitting the token.
 *
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param startPosition Position where the block comment started.
 * @param word Accumulated characters of the comment.
 * @param state Current tokenizer state.
 * @param c Current character.
 * @param prev_c Previous character, used to detect the closing `* /`.
 * @param isEOF true if processing final character.
 */
void consumeBlockComment(
        std::vector<Token> &tokens,
        struct Position &position,
        struct Position &startPosition,
        std::string &word,
        TokenizerState &state,
        char c,
        char prev_c,
        bool isEOF
) {
    bool isClosing = prev_c == '*' && c == '/';
    if (!isEOF || isClosing) {
        word += c;
        position.column++;
    }
    if (isClosing) {
        tokens.emplace_back(TokenType::BLOCK_COMMENT, word, startPosition, 0);
        word = "";
        state = TokenizerState::STATE_NONE;
    } else if (isEOF) {
        tokens.emplace_back(TokenType::UNKNOWN, word, startPosition, 0);
        word = "";
        state = TokenizerState::STATE_NONE;
    }
}

/**
 * Processes string literals (e.g., `"hello"`).
 *
 * This function is used when the lexer is in the `STATE_LITERAL_STRING` state. It accumulates
 * characters until the closing quote (`"`) is detected. Escaped quotes (`\"`) are handled correctly.
 * If a newline is encountered before the closing quote, the string is marked as invalid.
 *
 * Key Responsibilities:
 * - Accumulate all characters in the string literal, including escaped quotes.
 * - Finalize the string literal token when the closing quote is encountered.
 * - Emit an `UNKNOWN` token if the string is invalid (e.g., unclosed string).
 *
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word Accumulated characters of the string literal.
 * @param state Current tokenizer state.
 * @param c Current character.
 * @param prev_c Previous character, used to detect escaped quotes.
 */
void consumeLiteralString(
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c,
        char prev_c
) {
    if (c == '\n') {
        tokens.emplace_back(TokenType::UNKNOWN, word, position, word.size());
        word = "";
        state = TokenizerState::STATE_NONE;
        return;
    }

    word += c;
    if (prev_c != '\\' && c == '"') {
        tokens.emplace_back(TokenType::STRING, word, position, word.size() - 1);
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
    }
}

/**
 * Processes character literals (e.g., `'a'`).
 *
 * This function is used when the lexer is in the `STATE_LITERAL_CHAR` state. It accumulates
 * characters until the closing quote (`'`) is detected. Escaped characters (e.g., `\'`) are
 * handled correctly. If a newline is encountered before the closing quote, the character literal
 * is marked as invalid.
 *
 * Key Responsibilities:
 * - Accumulate all characters in the character literal, including escaped characters.
 * - Finalize the character literal token when the closing quote is encountered.
 * - Emit an `UNKNOWN` token if the character literal is invalid (e.g., unclosed or multiline).
 *
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word Accumulated characters of the character literal.
 * @param state Current tokenizer state.
 * @param c Current character.
 * @param prev_c Previous character, used to detect escaped characters.
 */
void consumeLiteralChar(
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c,
        char prev_c
) {
    if (c == '\n') {
        tokens.emplace_back(TokenType::UNKNOWN, word, position, word.size());
        word = "";
        state = TokenizerState::STATE_NONE;
        return;
    }

    word += c;
    if (prev_c != '\\' && c == '\'') {
        tokens.emplace_back(TokenType::CHAR, word, position, word.size() - 1);
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
    }
}

/**
 * Explanation:
 *
 * 1. `c != '='`
 *    - Ensures operators expecting '=' as the next character (e.g., '==', '!=', '+=', '-=') are handled properly.
 *    - Example:
 *      - If `word` is '!' and `c` is '=', parsing continues to form '!='.
 *      - If `word` is '!' and `c` is '+', parsing stops, and '!' is finalized as a separate token.
 *
 * 2. `c != prev_c`
 *    - Ensures repeated characters like '++' or '--' are valid operators but prevents unexpected repetitions.
 *    - Example:
 *      - If `word` is '+' and `c` is '+', parsing continues to form '++'.
 *      - If `word` is '+' and `c` is '-', parsing stops, and '+' is finalized as a token.
 *
 * 3. `!(prev_c == '-' && c == '>')`
 *    - Special case to allow the lambda operator '->', which consists of '-' followed by '>'.
 *    - Example:
 *      - If `word` is '-' and `c` is '>', parsing continues to form '->'.
 *      - If `word` is '-' and `c` is '+', parsing stops, and '-' is finalized as a token.
 *
 * Purpose:
 * - Handles complex sequences like `a--+-b` by ensuring `--`, `+`, and `-` are treated as separate operators.
 * - Prevents invalid combinations or merging of operators.
 **/
bool isUnmatchedOperator(char prev_c, char c) {
    return c != '=' && c != prev_c && !(prev_c == '-' && c == '>');
}

/**
 * Processes operators (e.g., `+`, `+=`, `&&`).
 *
 * This function is used when the lexer is in the `STATE_OPERATORS` state. It accumulates
 * characters that form valid operators and finalizes the token when encountering a character
 * that does not belong to an operator.
 *
 * Key Responsibilities:
 * - Accumulate valid operator characters.
 * - Finalize the operator token when encountering a non-operator character.
 *
 * Special Case:
 * - The `->` symbol, which is used in Java for lambda expressions, will also
 *   be processed here. However, at the end, when `getTokenType` is called, it
 *   will classify `->` as a `SYMBOL` type rather than an `OPERATOR`.
 *
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word Accumulated characters of the operator.
 * @param state Current tokenizer state.
 * @param c Current character.
  * @param next_c Next character in the source, Used to identify comment openings.
 * @return true if the character was consumed as part of the operator; false otherwise.
 */
bool consumeOperator(
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c,
        char next_c
) {
    // Checks if the current character (`c`) starts a comment,
    // either single-line (`//`) or block (`/*`).
    bool isFollowingByComment = c == '/' && (next_c == '/' || next_c == '*');

    // Checks if the current word represents a lambda symbol (`->`).
    bool isLambdaSymbol = word == "->";

    // Checks if the current word is an increment (`++`) or decrement (`--`) operator.
    bool isIncreaseDecrease = word == "++" || word == "--";

    // Checks if the current character (`c`) doesn't match the expected continuation of the operator
    // (e.g., for `==`, `!=`, `+=`, or `->`), ensuring no unmatched operators.
    bool isUnmatched = isUnmatchedOperator(word.back(), c);

    bool canContinue = !isFollowingByComment &&
                       !isLambdaSymbol &&
                       !isIncreaseDecrease &&
                       !isUnmatched;

    if (canContinue && isOperatorStart(std::string{c})) {
        word += c;
        return true;
    } else {
        tokens.emplace_back(getTokenType(word), word, position, word.size());
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
        return false;
    }
}

/**
 * Processes numbers, including decimal, floating-point, and exponential formats.
 *
 * This function is used when the lexer is in the `STATE_NUMBERS` state. It handles valid numeric
 * characters, including digits (`0-9`), dots (`.`), exponents (`e` or `E`), and valid underscores
 * (e.g., `1_000`, `1______0_0_0`). It finalizes the token when encountering a character that is not
 * part of a number. It also recognizes type identifiers like `f`, `d`, `l` (for float, double, or long).
 *
 * Key Responsibilities:
 * - Accumulate valid numeric characters, including underscores.
 * - Handle decimal points and exponents correctly.
 * - Finalize the number token when encountering a non-numeric character.
 * - Emit an `UNKNOWN` token for invalid numbers.
 *
 * @param source The full source code being tokenized.
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word Accumulated characters of the number.
 * @param state Current tokenizer state.
 * @param numberInfo Additional information about the number (e.g., use of dot or exponent).
 * @param prev_c Previous character in the source.
 * @param c Current character.
 * @param next_c Next character in the source.
 * @return true if the character was consumed as part of the number; false otherwise.
 */
bool consumeNumber(
        const std::string &source,
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        struct NumberInfo &numberInfo,
        char prev_c,
        char c,
        char next_c
) {
    std::string underscore = consumeUnderscoreInNumber(source, position.index, false, false);
    if (!underscore.empty()) {
        word += underscore;
        position.index += (int) underscore.size() - 1;
        return true;
    }

    if (isNumber(c) ||
        (numberInfo.hasUsedE && (prev_c == 'e' || prev_c == 'E') && (c == '-' || c == '+'))) {
        word += c;
        return true;
    } else if (!numberInfo.hasUsedDot && c == '.') {
        word += c;
        numberInfo.hasUsedDot = true;
        return true;
    } else if (!numberInfo.hasUsedE && (c == 'e' || c == 'E') &&
               (isNumber(next_c) || next_c == '+' || next_c == '-')) {
        word += c;
        numberInfo.hasUsedE = true;
        return true;
    } else {
        bool isType = isNumberTypeIdentifier(c, !numberInfo.hasUsedE && !numberInfo.hasUsedDot);
        if (isType) {
            word += c;
        }
        tokens.emplace_back(TokenType::NUMBER, word, position, word.size() - isType);
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
        return isType;
    }
}

/**
 * Processes hexadecimal (e.g., `0x1A`) and binary (e.g., `0b1010`) numbers.
 *
 * This function is used when the lexer is in the `STATE_HEX` or `STATE_BINARY` state. It accumulates
 * characters valid for hexadecimal or binary numbers, including underscores (e.g., `0x1A_2F`). It finalizes
 * the token when encountering a character that is not valid for the respective number type.
 *
 * Key Responsibilities:
 * - Accumulate valid hexadecimal (digits `0-9` and letters `A-F` or `a-f`) or binary (`0` and `1`) characters.
 * - Handle underscores within numbers, ensuring they are correctly placed.
 * - Finalize the number token when encountering an invalid character.
 * - Recognize type suffixes like `l` or `L` (for long).
 *
 * @param source The full source code being tokenized.
 * @param tokens Reference to the vector of tokens.
 * @param position Current position in the source code.
 * @param word Accumulated characters of the number.
 * @param state Current tokenizer state.
 * @param c Current character.
 * @param isBinary true if processing a binary number; false for hexadecimal.
 * @return true if the character was consumed as part of the number; false otherwise.
 */
bool consumeHexAndBinary(
        const std::string &source,
        std::vector<Token> &tokens,
        struct Position &position,
        std::string &word,
        TokenizerState &state,
        char c,
        bool isBinary
) {
    std::string underscore = consumeUnderscoreInNumber(source, position.index, isBinary, !isBinary);
    if (!underscore.empty()) {
        word += underscore;
        position.index += (int) underscore.size() - 1;
        return true;
    }

    if (word.size() == 1 || // allow '0x' or '0b' prefixes
        isHexOrBinaryNumber(c, isBinary)) {
        word += c;
        return true;
    } else {
        bool isType = c == 'l' || c == 'L';
        if (isType) {
            word += c;
        }
        TokenType type = isBinary ? TokenType::BINARY_NUMBER : TokenType::HEX_NUMBER;
        type = word.size() >= (isType ? 4 : 3) ? type : TokenType::UNKNOWN;
        tokens.emplace_back(type, word, position, word.size() - isType);
        position.column += (int) word.size();
        word = "";
        state = TokenizerState::STATE_NONE;
        return isType;
    }
}

/**
 * Tokenizes the given Java source code into a sequence of tokens.
 *
 * This function orchestrates the entire lexical analysis process. It processes
 * the source code character by character, maintaining the current state and invoking
 * appropriate `consume` functions for handling specific token types (e.g., identifiers,
 * comments, literals, and numbers).
 *
 * Key Responsibilities:
 * - Iterates through each character in the source code.
 * - Maintains line and column positions to track token locations.
 * - Transitions between states for specific token types.
 * - Emits tokens into the `tokens` vector as they are finalized.
 * - Handles special cases like EOF and ensures any remaining tokens are processed.
 *
 * @param source The Java source code as a string.
 * @return A vector of tokens representing the lexical elements of the source code.
 */
std::vector<Token> tokenize(const std::string &source) {
    struct Position blockCommentPositionSaver{};
    struct NumberInfo numberInfo{};
    struct Position position = {
            .index = 0,
            .line = 1,
            .column = 1
    };

    std::vector<Token> tokens;
    std::string word;

    TokenizerState currentState = STATE_NONE;
    char prev_c = '\0';
    bool isEOF = false;

    while (source.length() > position.index) {
        char c = source[position.index];
        char next_c = source.length() > position.index + 1 ? source[position.index + 1] : '\0';

        // Label for state transitions that require re-processing
        CONSUMER:
        bool hasConsumed = true;
        switch (currentState) {
            case TokenizerState::STATE_NONE:
                consume(tokens, position, word, currentState, c, next_c);
                if (currentState == TokenizerState::STATE_BLOCK_COMMENT) {
                    // Save the starting position of the block comment.
                    // Block comments can have multiple lines and include '\n',
                    // which resets the column counter. By saving the start position,
                    // we ensure accurate token location data for block comments.
                    blockCommentPositionSaver = position;
                } else if (currentState == TokenizerState::STATE_NUMBERS) {
                    numberInfo.hasUsedDot = c == '.';
                    numberInfo.hasUsedE = false;
                }
                break;
            case TokenizerState::STATE_WORD:
                hasConsumed = consumeWord(tokens, position, word, currentState, c);
                break;
            case TokenizerState::STATE_LINE_COMMENT:
                consumeLineComment(tokens, position, word, currentState, c);
                break;
            case TokenizerState::STATE_BLOCK_COMMENT:
                consumeBlockComment(tokens, position, blockCommentPositionSaver, word,
                                    currentState, c, prev_c, isEOF);
                break;
            case TokenizerState::STATE_LITERAL_STRING:
                consumeLiteralString(tokens, position, word, currentState, c, prev_c);
                break;
            case TokenizerState::STATE_LITERAL_CHAR:
                consumeLiteralChar(tokens, position, word, currentState, c, prev_c);
                break;
            case TokenizerState::STATE_OPERATORS:
                hasConsumed = consumeOperator(tokens, position, word, currentState, c, next_c);
                break;
            case TokenizerState::STATE_NUMBERS:
                hasConsumed = consumeNumber(source, tokens, position, word,
                                            currentState, numberInfo, prev_c, c, next_c);
                break;
            case TokenizerState::STATE_HEX:
                hasConsumed = consumeHexAndBinary(source, tokens, position, word, currentState, c, false);
                break;
            case TokenizerState::STATE_BINARY:
                hasConsumed = consumeHexAndBinary(source, tokens, position, word, currentState, c, true);
                break;
        }
        if (!isEOF && !hasConsumed) {
            // Re-process the current character if the state changed
            // and previous consumer didn't consume it!
            goto CONSUMER;
        }

        position.index++;
        if (c == '\n') {
            position.line++;
            position.column = 1;
        }
        prev_c = c;

        // Handle end of source: finalize any unprocessed token
        if (position.index >= source.length() &&
            currentState != TokenizerState::STATE_NONE &&
            !word.empty() &&
            prev_c != '\n') {
            isEOF = true;
            c = '\n';
            // Trigger final processing for the remaining token
            goto CONSUMER;
        }
    }

    return tokens;
}