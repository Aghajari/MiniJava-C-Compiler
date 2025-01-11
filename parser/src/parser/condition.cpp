#include "../../internal/parser_internal.h"

/**
 * @brief Parses an `if` statement and its associated blocks.
 *
 * The `if` statement consists of:
 * - A mandatory condition enclosed in parentheses `()`.
 * - A body (can be a code block `{}` or a single statement).
 * - An optional `else` block, which may contain:
 *   - Another `if` statement for `else if`.
 *   - A code block or single statement for `else`.
 *
 * The function ensures proper nesting of `if-else` statements and constructs
 * an `IfStatement` AST node with the condition, body, and optional `else` part.
 *
 * Example:
 * ```java
 * if (x > 0) {
 *     // Do something
 * } else if (x < 0) {
 *     // Do something else
 * } else {
 *     // Fallback
 * }
 * ```
 *
 * @param project The `Project` containing the source being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 * @return A `std::unique_ptr` to the `IfStatement` AST node.
 */
std::unique_ptr<ASTNode> parseIfStatement(
        Project &project,
        TokenStreamer &streamer
) {
    Token *token = streamer.read();
    if (token == nullptr || token->lexeme != "(") {
        error("Failed to parse if-statement, expected '('", token);
    }
    auto condition = parseExpression(project, streamer);
    token = streamer.read();
    if (token == nullptr || token->lexeme != ")") {
        error("Failed to parse if-statement, expected ')'", token);
    }

    token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse if-statement, expected '{'", token);
    }
    auto codeBlockNode = parseCodeBlockOrStatement(token, project, streamer);

    std::unique_ptr<ASTNode> elseBody = nullptr;
    token = streamer.read();
    if (token != nullptr && token->lexeme == "else") {
        token = streamer.read();
        if (token == nullptr) {
            error("Failed to parse if-statement, Expected else body");
        }

        if (token->lexeme == "if") {
            elseBody = parseIfStatement(project, streamer);
        } else {
            elseBody = parseCodeBlockOrStatement(token, project, streamer);
        }
    } else {
        streamer.unread();
    }

    std::unique_ptr<ASTNode> node = std::make_unique<IfStatement>(
            std::move(condition),
            std::move(codeBlockNode),
            std::move(elseBody)
    );
    return node;
}

/**
 * @brief Parses a `while` loop and its associated body.
 *
 * The `while` loop consists of:
 * - A mandatory condition enclosed in parentheses `()`.
 * - A body (can be a code block `{}` or a single statement).
 *
 * This function constructs a `WhileStatement` AST node with the parsed condition
 * and body. It ensures the syntax of the `while` loop is correct, including the
 * presence of the condition and body structure.
 *
 * Example:
 * ```java
 * while (x > 0) {
 *     x--;
 * }
 * ```
 *
 * @param project The `Project` containing the source being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 * @return A `std::unique_ptr` to the `WhileStatement` AST node.
 */
std::unique_ptr<ASTNode> parseWhileStatement(
        Project &project,
        TokenStreamer &streamer
) {
    Token *token = streamer.read();
    if (token == nullptr || token->lexeme != "(") {
        error("Failed to parse while-statement, expected '('", token);
    }
    auto condition = parseExpression(project, streamer);
    token = streamer.read();
    if (token == nullptr || token->lexeme != ")") {
        error("Failed to parse while-statement, expected ')'", token);
    }

    token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse while-statement, expected '{'", token);
    }

    std::unique_ptr<ASTNode> node = std::make_unique<WhileStatement>(
            std::move(condition),
            parseCodeBlockOrStatement(token, project, streamer),
            false
    );
    return node;
}

/**
 * @brief Parses a `do-while` loop statement in Mini-Java.
 *
 * The `do-while` loop consists of:
 * 1. A 'do' keyword
 * 2. A body (executed at least once)
 * 3. A 'while' keyword
 * 4. A condition in parentheses
 * 5. A semicolon
 *
 * This function constructs a `WhileStatement` AST node with the parsed condition
 * and body. It ensures the syntax of the `do-while` loop is correct, including the
 * presence of the condition and body structure.
 *
 * Example Mini-Java Code:
 * ```java
 * // With braces
 * do {
 *     x--;
 * } while (x > 0);
 *
 * // Without braces (single statement)
 * do x--; while (x > 0);
 * ```
 *
 * @param project The Project context for parsing.
 * @param streamer The TokenStreamer for reading tokens.
 * @return std::unique_ptr<ASTNode> A WhileStatement node with isDoWhile=true.
 */
std::unique_ptr<ASTNode> parseDoWhileStatement(
        Project &project,
        TokenStreamer &streamer
) {
    Token *token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse do-while-statement, expected '{'", token);
    }
    auto body = parseCodeBlockOrStatement(token, project, streamer);

    token = streamer.read();
    if (token == nullptr || token->lexeme != "while") {
        error("Failed to parse do-while-statement, expected 'while'", token);
    }
    token = streamer.read();
    if (token == nullptr || token->lexeme != "(") {
        error("Failed to parse do-while-statement, expected '('", token);
    }
    auto condition = parseExpression(project, streamer);
    token = streamer.read();
    if (token == nullptr || token->lexeme != ")") {
        error("Failed to parse do-while-statement, expected ')'", token);
    }
    token = streamer.read();
    if (token == nullptr || token->lexeme != ";") {
        error("Failed to parse do-while-statement, expected ';'", token);
    }

    std::unique_ptr<ASTNode> node = std::make_unique<WhileStatement>(
            std::move(condition),
            std::move(body),
            true
    );
    return node;
}

/**
 * @brief Parses a `for` loop and its associated components.
 *
 * The `for` loop consists of:
 * - Optional **initialization** block (e.g., variable declaration or assignment).
 * - Optional **condition**: A boolean expression that determines when the loop stops.
 * - Optional **update** block: Expressions executed after each iteration.
 * - A body: Either a code block `{}` or a single statement.
 *
 * The function uses the parsed components to construct a `ForStatement` AST node,
 * enforcing correct syntax for the `for` loop, including proper separation by
 * semicolons (`;`) and parentheses (`()`).
 *
 * Example:
 * ```java
 * for (int i = 0; i < 10; i++) {
 *     System.out.println(i);
 * }
 * ```
 *
 * @param project The `Project` containing the source being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 * @return A `std::unique_ptr` to the `ForStatement` AST node.
 */
std::unique_ptr<ASTNode> parseForStatement(
        Project &project,
        TokenStreamer &streamer
) {
    std::unique_ptr<CodeBlock> initialization = nullptr;
    std::unique_ptr<ASTNode> condition = nullptr;
    std::unique_ptr<CodeBlock> update = nullptr;
    std::unique_ptr<CodeBlock> body = nullptr;

    Token *token = streamer.read();
    if (token == nullptr || token->lexeme != "(") {
        error("Failed to parse for-statement, expected '('", token);
    }
    token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse for-statement, expected initialization", token);
    } else if (token->lexeme != ";") {
        CodeBlock cb = {};
        parseSimpleStatement(&cb, token, project, streamer);
        initialization = std::make_unique<CodeBlock>(std::move(cb));
    }

    if (initialization) {
        token = streamer.read();
        if (token == nullptr || token->lexeme != ";") {
            error("Failed to parse for-statement, expected ';' and condition", token);
        }
    }

    token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse for-statement, expected condition", token);
    } else if (token->lexeme != ";") {
        streamer.unread();
        condition = parseExpression(project, streamer);
    }

    if (condition) {
        token = streamer.read();
        if (token == nullptr || token->lexeme != ";") {
            error("Failed to parse for-statement, expected ';' and condition", token);
        }
    }

    token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse for-statement, expected update", token);
    } else if (token->lexeme != ")") {
        CodeBlock cb = {};
        parseAssignment(&cb, token, project, streamer);
        update = std::make_unique<CodeBlock>(std::move(cb));
    }

    if (update) {
        token = streamer.read();
        if (token == nullptr || token->lexeme != ")") {
            error("Failed to parse for-statement, expected ')'", token);
        }
    }

    token = streamer.read();
    if (token == nullptr) {
        error("Failed to parse for-statement, expected '{'", token);
    } else if (token->lexeme != ";") {
        body = parseCodeBlockOrStatement(token, project, streamer);
    }

    std::unique_ptr<ASTNode> node = std::make_unique<ForStatement>(
            std::move(initialization),
            std::move(condition),
            std::move(update),
            std::move(body)
    );
    return node;
}