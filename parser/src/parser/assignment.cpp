#include "../../internal/parser_internal.h"

std::vector<std::vector<std::string>> operatorPrecedence = {
        {"||"},                   // Logical OR (Lowest precedence)
        {"&&"},                   // Logical AND
        {"|"},                    // Bitwise OR
        {"^"},                    // Bitwise XOR
        {"&"},                    // Bitwise AND
        {"==", "!="},             // Equality Operators
        {"<",  "<=", ">", ">="},   // Relational Operators
        {"+",  "-"},               // Addition, Subtraction
        {"*",  "/",  "%"},          // Multiplication, Division, Modulus
        {"!",  "~"}                // Logical NOT, Bitwise NOT (Highest precedence)
};

/**
 * @brief Parses a **reference chain** consisting of fields, method calls, array accesses, and object creation.
 *
 * A reference chain represents chained object references (e.g., `object.field.methodCall().array[index]`) or
 * new object/array creation (e.g., `new Object()` or `new int[size]`). This function constructs a `ReferenceChain`
 * representing such entities.
 *
 * Examples:
 * ```java
 * array[index];          // Array access
 * object.field.method(); // Chained method call
 * new MyClass();         // New object
 * new int[10];           // New integer array
 * ```
 *
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 * @param reference The first token of the reference chain.
 * @return A `ReferenceChain` object representing the parsed chain.
 */
ReferenceChain parseReferenceChain(
        Project &project,
        TokenStreamer &streamer,
        Token *reference
) {
    Token *tokenToAdd = reference;
    ReferenceChain referenceChain = {};

    if (tokenToAdd->lexeme == "new") {
        Token *type = streamer.read();
        Token *next;
        std::unique_ptr<ASTNode> array_size = nullptr;

        if (type != nullptr && type->lexeme == "int") {
            next = streamer.read();
            if (next == nullptr || next->lexeme != "[") {
                error("Failed to parse new array, Expected '['", next);
            }
            array_size = parseExpression(project, streamer);
            next = streamer.read();
            if (next == nullptr || next->lexeme != "]") {
                error("Failed to parse new array, Expected ']'", next);
            }
        } else {
            if (type == nullptr || type->type != IDENTIFIER) {
                error("Failed to parse new object, Expected identifier", type);
            }
            next = streamer.read();
            if (next == nullptr || next->lexeme != "(") {
                error("Failed to parse new object, Expected '('", next);
            }
            next = streamer.read();
            if (next == nullptr || next->lexeme != ")") {
                error("Failed to parse new object, Expected ')'", next);
            }
        }
        if (streamer.peek() == nullptr) {
            error("Failed to parse new object, Expected ';'", next);
        }
        referenceChain.addNode(*tokenToAdd, std::make_unique<NewObject>(std::move(*type), std::move(array_size)));
        tokenToAdd = nullptr;
        if (streamer.peek()->lexeme == ";") {
            return referenceChain;
        }
    }

    while (true) {
        Token *next = streamer.read();
        if (next->lexeme == ".") {
            if (tokenToAdd) {
                referenceChain.addField(*tokenToAdd);
            }
            next = streamer.read();
            if (next == nullptr || next->type != TokenType::IDENTIFIER) {
                error("Failed to parse reference chain, Expected identifier", next);
            }
            tokenToAdd = next;
        } else if (next->lexeme == "[") {
            auto bracket = parseExpression(project, streamer);
            auto t = streamer.read();
            if (t == nullptr || t->lexeme != "]") {
                error("Failed to parse bracket, expected ]", t);
            }
            referenceChain.addNode(
                    *tokenToAdd,
                    std::make_unique<ArrayCall>(tokenToAdd->lexeme, std::move(bracket))
            );
            tokenToAdd = nullptr;
        } else if (next->lexeme == "(") {
            auto methodCall = std::make_unique<MethodCall>(tokenToAdd->lexeme);

            while (true) {
                if (streamer.peek() == nullptr) {
                    error("Failed to parse method call", next);
                }

                if (streamer.peek()->lexeme != ")") {
                    std::unique_ptr<ASTNode> node = parseExpression(project, streamer);
                    methodCall->addArgument(std::move(node));
                }

                if (streamer.peek()->lexeme == ",") {
                    streamer.read();
                } else if (streamer.peek()->lexeme == ")") {
                    streamer.read();
                    break;
                }
            }
            referenceChain.addNode(*tokenToAdd, std::move(methodCall));
            tokenToAdd = nullptr;
        } else {
            if (tokenToAdd) {
                referenceChain.addField(*tokenToAdd);
            }
            streamer.unread();
            break;
        }
    }
    return referenceChain;
}

/**
 * @brief Parses a **primary expression** (e.g., literals, identifiers, reference chains).
 *
 * This function handles basic expressions, including:
 * - Literals (`NumberASTNode`, `BooleanASTNode`)
 * - Identifiers, `this`, and `new` for object/array creation (via `ReferenceChain`).
 * - Parenthesized expressions `(expr)` for grouping.
 *
 * Example:
 * ```java
 * 42;         // Number literal
 * true;       // Boolean literal
 * this;       // Reference to the current object
 * (x + y);    // Parenthesized expression
 * new MyClass(); // Object creation
 * ```
 *
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 * @return A `std::unique_ptr` to the corresponding `ASTNode`.
 */
std::unique_ptr<ASTNode> parsePrimary(
        Project &project,
        TokenStreamer &streamer
) {
    auto token = streamer.read();
    if (token == nullptr) {
        error("Expected a primary expression but got null");
    }

    if (token->type == TokenType::NUMBER ||
        token->type == TokenType::HEX_NUMBER ||
        token->type == TokenType::BINARY_NUMBER) {
        return std::make_unique<NumberASTNode>(*token);
    } else if (token->lexeme == "true" || token->lexeme == "false") {
        return std::make_unique<BooleanASTNode>(*token);
    } else if (token->type == TokenType::IDENTIFIER || token->lexeme == "this"
               || token->lexeme == "new") {
        auto n = parseReferenceChain(project, streamer, token);
        return std::make_unique<ReferenceASTNode>(std::move(n));
    } else if (token->lexeme == "(") {
        auto expression = parseExpression(project, streamer);

        auto next = streamer.read();
        if (next == nullptr) {
            error("Failed to parse expression, Expected ')' at end", token);
        } else if (next->lexeme != ")") {
            error("Failed to parse expression, Expected ')'", next);
        }
        return expression;
    }

    error("Expected a primary expression", token);
    throw std::runtime_error("Expected a primary expression");
}

/**
 * @brief Parses an expression while respecting operator precedence.
 *
 * This function parses complex expressions with mixed operators by recursively parsing
 * sub-expressions based on their precedence.
 *
 * Examples:
 * ```java
 * x + y * z;       // Resolves based on precedence (+ is lower than *)
 * a > b && b < c;  // Logical AND has lower precedence than comparisons
 * !(x == 42);      // Logical NOT has the highest precedence
 * ```
 *
 * Operator precedence is defined in the `operatorPrecedence` vector.
 *
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 * @param precedenceLevel The current precedence level to process (default: 0).
 * @return A `std::unique_ptr` to the resulting `ASTNode` (e.g., `BinaryExpression`, `NotExpression`).
 */
std::unique_ptr<ASTNode> parseExpressionWithPrecedence(
        Project &project,
        TokenStreamer &streamer,
        size_t precedenceLevel = 0
) {
    if (precedenceLevel == operatorPrecedence.size() - 1) {
        if (streamer.peek() != nullptr &&
            (streamer.peek()->lexeme == "!" || streamer.peek()->lexeme == "~")) {
            Token *op = streamer.read();
            return std::make_unique<NotExpression>(*op,
                                                   parseExpressionWithPrecedence(project, streamer, precedenceLevel));
        }

        return parsePrimary(project, streamer);
    }

    auto left = parseExpressionWithPrecedence(project, streamer, precedenceLevel + 1);

    while (streamer.peek() != nullptr &&
           std::find(operatorPrecedence[precedenceLevel].begin(),
                     operatorPrecedence[precedenceLevel].end(),
                     streamer.peek()->lexeme) != operatorPrecedence[precedenceLevel].end()) {
        auto op = *streamer.read();
        auto right = parseExpressionWithPrecedence(project, streamer, precedenceLevel + 1);
        left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right));
    }
    return left;
}

/**
 * @brief Parses an expression and resolves casting, unary operators, and precedence.
 *
 * This function works as the entry point for parsing expressions. It handles:
 * - Unary operators (`!`, `~`)
 * - Casting expressions (e.g., `(Type) value`)
 * - Delegates sub-expression parsing to `parseExpressionWithPrecedence`.
 *
 * Examples:
 * ```java
 * (int) x;   // Cast expression
 * !flag;     // Logical NOT
 * x + y * z; // Delegates precedence parsing to `parseExpressionWithPrecedence`
 * ```
 *
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 * @return A `std::unique_ptr` to the corresponding `ASTNode` representation.
 */
std::unique_ptr<ASTNode> parseExpression(
        Project &project,
        TokenStreamer &streamer
) {
    if (streamer.peek() == nullptr) {
        error("Failed to parse, Expected expression but got null");
    }

    if (streamer.peek()->lexeme == "!" || streamer.peek()->lexeme == "~") {
        Token *op = streamer.read();
        return std::make_unique<NotExpression>(*op, parseExpression(project, streamer));
    }

    if (streamer.peek()->lexeme == "(") {
        streamer.save();
        streamer.read();
        if (streamer.peek() != nullptr && streamer.peek()->type == TokenType::IDENTIFIER) {
            Token *castTo = streamer.read();
            if (streamer.peek() != nullptr && streamer.peek()->lexeme == ")") {
                streamer.read();
                if (streamer.peek() != nullptr && streamer.peek()->type != TokenType::OPERATOR &&
                    streamer.peek()->lexeme != ";") {
                    return std::make_unique<CastExpression>(std::move(*castTo), parseExpression(project, streamer));
                }
            }
        }
        streamer.restore();
    }

    return parseExpressionWithPrecedence(project, streamer);
}

/**
 * @brief Parses unary operators (`++` and `--`) applied to references.
 *
 * This function handles unary operators that modify references (e.g., variables or fields)
 * by constructing an `Assignment` node. It converts the unary operation into an equivalent
 * compound assignment (`+= 1` or `-= 1`).
 *
 * Examples:
 * ```java
 * ++i;  // Converted to `i += 1`
 * --array[index]; // Converted to `array[index] -= 1`
 * ```
 *
 * @param reference The token representing the unary operator (`++` or `--`).
 * @param referenceChain The reference being modified (may be `nullptr`).
 * @param codeBlock The `CodeBlock` to which the resulting assignment is added.
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 */
void parseUnary(
        Token *reference,
        ReferenceChain *referenceChain,
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
) {
    Token convertedToken = Token(reference->type, "", reference->position, 0);
    Token numberToken = Token(TokenType::NUMBER, "1", reference->position, 0);
    if (reference->lexeme == "++") {
        convertedToken.lexeme = "+=";
    } else {
        convertedToken.lexeme = "-=";
    }

    auto number = std::make_unique<NumberASTNode>(numberToken);
    if (referenceChain == nullptr) {
        ReferenceChain referenceChain2 = parseReferenceChain(project, streamer, streamer.read());
        std::unique_ptr<ASTNode> node = std::make_unique<Assignment>(
                std::move(referenceChain2), convertedToken, std::move(number));
        codeBlock->addCode(node);
    } else {
        std::unique_ptr<ASTNode> node = std::make_unique<Assignment>(
                std::move(*referenceChain), convertedToken, std::move(number));
        codeBlock->addCode(node);
    }
}

/**
 * @brief Parses an assignment or method call statement.
 *
 * This function handles:
 * - Assignments (`=`, `+=`, `-=`, etc.) to variables, fields, or array elements.
 * - Method calls and their arguments.
 *
 * Examples:
 * ```java
 * x = 42;             // Simple assignment
 * array[index] += 5;  // Compound assignment to an array element
 * object.method(1, 2); // Method call
 * ```
 *
 * @param codeBlock The `CodeBlock` to which the resulting AST node is added.
 * @param reference The first token in the assignment (e.g., target variable).
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 */
void parseAssignment(
        CodeBlock *codeBlock,
        Token *reference,
        Project &project,
        TokenStreamer &streamer
) {
    ReferenceChain referenceChain = parseReferenceChain(project, streamer, reference);
    Token *next = streamer.read();

    if (next == nullptr) {
        error("Failed to parse assignment code, Expected assignment or method call but got null");
    }

    if (next->lexeme == "=" ||
        next->lexeme == "+=" ||
        next->lexeme == "-=" ||
        next->lexeme == "*=" ||
        next->lexeme == "&=" ||
        next->lexeme == "|=" ||
        next->lexeme == "^=" ||
        next->lexeme == "/=") {

        auto expression = parseExpression(project, streamer);
        std::unique_ptr<ASTNode> node = std::make_unique<Assignment>(
                std::move(referenceChain), *next, std::move(expression));
        codeBlock->addCode(node);
    } else if (next->lexeme == "++" || next->lexeme == "--") {
        parseUnary(next, &referenceChain, codeBlock, project, streamer);
    } else if (next->lexeme == ";") {
        std::unique_ptr<ASTNode> node = std::make_unique<ReferenceASTNode>(std::move(referenceChain));
        codeBlock->addCode(node);
    } else {
        error("Failed to parse assignment code, Expected assignment", next);
    }
}

/**
 * @brief Parses an assignment specific to a local variable.
 *
 * This function handles expressions like:
 * ```java
 * int x = 42; // Local variable declaration with assignment
 * ```
 * It ensures that the local variable is assigned a compatible type and constructs
 * an `Assignment` node for the statement.
 *
 * @param codeBlock The `CodeBlock` to which the parsed assignment is added.
 * @param fieldNameToken The token representing the local variable's name.
 * @param assignmentToken The token representing the assignment operator (`=` or compound).
 * @param project The `Project` context containing parsed data.
 * @param streamer The `TokenStreamer` used for sequential tokenization.
 */
void parseAssignmentForLocalVariable(
        CodeBlock *codeBlock,
        Token *fieldNameToken,
        Token *assignmentToken,
        Project &project,
        TokenStreamer &streamer
) {
    ReferenceChain referenceChain = {};
    referenceChain.addField(*fieldNameToken);

    auto expression = parseExpression(project, streamer);
    std::unique_ptr<ASTNode> node = std::make_unique<Assignment>(
            std::move(referenceChain), *assignmentToken, std::move(expression));
    codeBlock->addCode(node);
}