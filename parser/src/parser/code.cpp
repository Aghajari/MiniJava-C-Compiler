#include "../../internal/parser_internal.h"

/**
 * @brief Parses a local variable declaration and its optional initializer.
 *
 * This function handles the declaration of local variables inside a method or block. It validates:
 * - The type of the variable.
 * - The variable's identifier (name).
 * - An optional initializer (e.g., assignment `=` or compound assignment `+=`, `-=`, etc.).
 *
 * Example:
 * ```java
 * int x;         // Local variable without initializer
 * int y = 5;     // Local variable with assignment initializer
 * int z += 2;    // Local variable with compound assignment
 * ```
 *
 * If a valid variable is found, it is added to the current code block as a `LocalVariableASTNode`.
 *
 * @param codeBlock The `CodeBlock` to which the parsed statement will be added.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseLocalVariableCode(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
) {
    ParamSignature sign = {};
    sign.isStatic = false;
    Token *nameToken = parseParam(&sign, project, streamer);

    Field field = Field(sign.type, sign.type_lexeme, sign.name);
    std::unique_ptr<ASTNode> astNode = std::make_unique<LocalVariableASTNode>(field);
    codeBlock->addCode(astNode);

    Token *next = streamer.read();
    if (next == nullptr) {
        error("Failed to parse local variable code, Expected ; or assignment but got null");
    }

    if (next->lexeme == ";") {
        return;
    }

    if (isAssignment(next)) {
        parseAssignmentForLocalVariable(codeBlock, nameToken, next, project, streamer);
    } else {
        error("Failed to parse local variable code, Expected ; or assignment", next);
    }
}

/**
 * @brief Parses a `return` statement.
 *
 * The `return` statement consists of:
 * - Optionally, an expression to return.
 * - A mandatory semicolon (`;`) at the end of the statement.
 *
 * Example:
 * ```java
 * return;              // Return without value
 * return 42;           // Return with an integer expression
 * return x + y * 2;    // Return with a complex expression
 * ```
 *
 * The parsed `ReturnStatement` is added to the current code block.
 *
 * @param codeBlock The `CodeBlock` to which the parsed statement will be added.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseReturn(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
) {
    if (streamer.peek() == nullptr || streamer.peek()->lexeme == ";") {
        streamer.read();
        std::unique_ptr<ASTNode> astNode = std::make_unique<ReturnStatement>(nullptr);
        codeBlock->addCode(astNode);
        return;
    } else {
        auto expr = parseExpression(project, streamer);
        if (streamer.peek() == nullptr || streamer.peek()->lexeme != ";") {
            error("Failed to parse return expression, Expected ';'", streamer.peek());
        }
        std::unique_ptr<ASTNode> astNode = std::make_unique<ReturnStatement>(std::move(expr));
        codeBlock->addCode(astNode);
    }
}

/**
 * @brief Parses a `break` statement.
 *
 * A `break` statement terminates the nearest enclosing loop (`for`, `while`). The statement
 * must terminate with a semicolon (`;`).
 *
 * Example:
 * ```java
 * break;  // Break from current loop
 * ```
 *
 * The parsed `BreakStatement` is added to the current code block.
 *
 * @param codeBlock The `CodeBlock` to which the parsed statement will be added.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseBreak(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
) {
    if (streamer.peek() == nullptr || streamer.peek()->lexeme == ";") {
        streamer.read();
        std::unique_ptr<ASTNode> astNode = std::make_unique<BreakStatement>();
        codeBlock->addCode(astNode);
        return;
    } else {
        error("Failed to parse break, Expected ';'", streamer.peek());
    }
}

/**
 * @brief Parses a `continue` statement.
 *
 * A `continue` statement skips the current iteration of the nearest enclosing loop
 * (`for`, `while`). The statement must terminate with a semicolon (`;`).
 *
 * Example:
 * ```java
 * continue;  // Skip to the next iteration of the loop
 * ```
 *
 * The parsed `ContinueStatement` is added to the current code block.
 *
 * @param codeBlock The `CodeBlock` to which the parsed statement will be added.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseContinue(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
) {
    if (streamer.peek() == nullptr || streamer.peek()->lexeme == ";") {
        streamer.read();
        std::unique_ptr<ASTNode> astNode = std::make_unique<ContinueStatement>();
        codeBlock->addCode(astNode);
        return;
    } else {
        error("Failed to parse continue, Expected ';'", streamer.peek());
    }
}

/**
 * @brief Parses a simple statement (e.g., a local variable declaration or an assignment).
 *
 * A simple statement can be:
 * - A local variable declaration, optionally initialized.
 * - An assignment or compound assignment to a variable or field.
 * - A unary operation (`++`, `--`).
 *
 * Example:
 * ```java
 * int x = 10;        // Local variable with assignment
 * ++x;               // Unary increment
 * x += 5;            // Compound addition assignment
 * ```
 *
 * The parsed statement is added to the current code block.
 * For-loop initializers are simple statements.
 *
 * @param codeBlock The `CodeBlock` to which the parsed statement will be added.
 * @param token The first token of the statement.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseSimpleStatement(
        CodeBlock *codeBlock,
        Token *token,
        Project &project,
        TokenStreamer &streamer
) {
    if (token->lexeme == "--" || token->lexeme == "++") {
        parseUnary(token, nullptr, codeBlock, project, streamer);
        return;
    }

    if (isValidType(token, false)) {
        streamer.save();
        if (streamer.peek() != nullptr) {
            if (streamer.peek()->type == TokenType::IDENTIFIER
                || (token->lexeme == "int" && streamer.peek()->lexeme == "[")) {
                streamer.restore();
                streamer.unread();
                parseLocalVariableCode(codeBlock, project, streamer);
                return;
            }
        }
        streamer.restore();
    }

    parseAssignment(codeBlock, token, project, streamer);
}

/**
 * @brief Parses a generic Java statement, including control flow constructs and expressions.
 *
 * This function handles:
 * - Control flow statements: `if`, `while`, `for`.
 * - Local variable declarations.
 * - Block statements: `{ ... }`.
 * - Return and jump statements: `return`, `break`, `continue`.
 * - Expressions: Assignments, unary operations, etc.
 *
 * Example:
 * ```java
 * if (x > 0) { ... }     // If-statement
 * while (x > 0) { ... }  // While-statement
 * return x;              // Return-statement
 * ```
 *
 * Each parsed statement is added to the provided code block.
 *
 * @param codeBlock The `CodeBlock` to which the parsed statement will be added.
 * @param token The first token of the statement.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseStatement(
        CodeBlock *codeBlock,
        Token *token,
        Project &project,
        TokenStreamer &streamer
) {
    if (token->lexeme == "--" || token->lexeme == "++") {
        parseUnary(token, nullptr, codeBlock, project, streamer);
        goto read_semicolon;
    }

    if (isValidType(token, false)) {
        streamer.save();
        if (streamer.peek() != nullptr) {
            if (streamer.peek()->type == TokenType::IDENTIFIER
                || (token->lexeme == "int" && streamer.peek()->lexeme == "[")) {
                streamer.restore();
                streamer.unread();
                parseLocalVariableCode(codeBlock, project, streamer);
                goto read_semicolon;
            }
        }
        streamer.restore();
    }

    if (token->lexeme == "if") {
        auto node = parseIfStatement(project, streamer);
        codeBlock->addCode(node);
    } else if (token->lexeme == "while") {
        auto node = parseWhileStatement(project, streamer);
        codeBlock->addCode(node);
    } else if (token->lexeme == "do") {
        auto node = parseDoWhileStatement(project, streamer);
        codeBlock->addCode(node);
    } else if (token->lexeme == "for") {
        auto node = parseForStatement(project, streamer);
        codeBlock->addCode(node);
    } else if (token->type == TokenType::IDENTIFIER || token->lexeme == "this" || token->lexeme == "new") {
        parseAssignment(codeBlock, token, project, streamer);
    } else if (token->lexeme == "{") {
        CodeBlock cb = {};
        parseCodeBlock(&cb, project, streamer);
        std::unique_ptr<ASTNode> astNode = std::make_unique<CodeBlock>(std::move(cb));
        codeBlock->addCode(astNode);
    } else if (token->lexeme == "return") {
        parseReturn(codeBlock, project, streamer);
    } else if (token->lexeme == "break") {
        parseBreak(codeBlock, project, streamer);
    } else if (token->lexeme == "continue") {
        parseContinue(codeBlock, project, streamer);
    } else {
        error("Failed to parse statement", token);
    }

    read_semicolon:
    if (streamer.peek()->lexeme == ";") streamer.read();
}

/**
 * @brief Parses a code block containing multiple statements.
 *
 * A code block consists of:
 * - Zero or more statements.
 * - Enclosed by `{` and `}` braces in Java.
 *
 * Example:
 * ```java
 * {
 *     int x = 10;
 *     x++;
 *     if (x > 0) {
 *         return x;
 *     }
 * }
 * ```
 *
 * Each parsed statement is added to the `CodeBlock`.
 *
 * @param codeBlock The `CodeBlock` to populate with parsed statements.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseCodeBlock(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
) {
    while (true) {
        Token *token = streamer.read();
        if (token == nullptr) {
            error("Failed to parse method body, Expected } but got null");
        }
        if (token->lexeme == ";") {
            continue;
        }
        if (token->lexeme == "}") {
            return;
        }

        parseStatement(codeBlock, token, project, streamer);
    }
}

/**
 * @brief Parses either a code block (enclosed in braces) or a single statement.
 *
 * This function handles two cases:
 * 1. A block of code enclosed in curly braces `{ ... }`
 * 2. A single statement without braces
 *
 * This is commonly used in control structures where braces are optional for single statements:
 * ```java
 * // With braces (code block):
 * if (condition) {
 *     statement1;
 *     statement2;
 * }
 *
 * // Without braces (single statement):
 * if (condition)
 *     statement;
 * ```
 *
 * Note: Even single statements are wrapped in a CodeBlock for uniform handling
 * in the AST.
 *
 * @param token The current token being processed
 * @param project The project context
 * @param streamer The token streamer for reading input
 * @return A unique_ptr to a CodeBlock containing either:
 *         - Multiple statements if parsing a braced block
 *         - A single statement if parsing an unbraced statement
 */
std::unique_ptr<CodeBlock> parseCodeBlockOrStatement(
        Token *token,
        Project &project,
        TokenStreamer &streamer
) {
    CodeBlock cb = {};
    if (token->lexeme != ";") {
        if (token->lexeme == "{") {
            parseCodeBlock(&cb, project, streamer);
        } else {
            parseStatement(&cb, token, project, streamer);
        }
    }
    return std::make_unique<CodeBlock>(std::move(cb));
}