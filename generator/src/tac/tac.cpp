#include "../../internal/generator_tac.h"
#include "../../../lexer/include/token_matcher.h"

/**
 * @brief Generates the negated form of a boolean condition.
 *
 * Converts boolean conditions into their negated counterparts:
 * - `true` → `false`
 * - `false` → `true`
 * - For any other condition, wraps it in `!(...)` for negation.
 *
 * Example:
 * ```c
 * not_condition("x > 5") → "!(x > 5)"
 * not_condition("true") → "false"
 * ```
 *
 * @param condition The boolean condition to negate.
 * @return A string representing the negated condition.
 */
std::string not_condition(const std::string &condition) {
    if (condition == "true") {
        return "false";
    } else if (condition == "false") {
        return "true";
    } else {
        return "!(" + condition + ")";
    }
}

/**
 * @brief Generates TAC for a binary expression (e.g., `x + y`).
 *
 * Handles binary operators including:
 * - Arithmetic: `+`, `-`, `*`, `/`, '%'
 * - Equality: '==', '!='
 * - Comparison: `<`, `>`, `<=`, `>=`
 * - Bitwise: `&`, `|`, `^`
 * - Shifts: `<<`, `>>`, `>>>`
 *
 * Special handling is provided for the unsigned right shift operator (`>>>`),
 * which is translated to C using explicit casting to achieve unsigned behavior.
 *
 * The function:
 * - Recursively generates TAC for the left and right operands.
 * - Allocates a new temporary variable to store the result.
 * - Emits a TAC instruction like `temp1 = left + right`.
 *
 * Examples:
 * ```java
 * x + y
 * a >>> b
 * ```
 *
 * Generated TAC:
 * ```c
 * int temp1 = x + y;
 * int temp2 = (int)((unsigned int)(a) >> b);
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `BinaryExpression` node representing the binary operation.
 * @return The name of the temporary variable storing the result.
 */
std::string generate(ThreeAddressCodeGenerator &gen, BinaryExpression *node) {
    std::string leftTemp = generate(gen, &node->left);
    std::string rightTemp = generate(gen, &node->right);
    std::string result = gen.tempGen.newTemp();
    if (node->op.lexeme == ">>>") {
        gen.emit(get_type(node->type) + result + " = (int) ((unsigned int) (" + leftTemp + ") >> " + rightTemp + ")");
    } else {
        gen.emit(get_type(node->type) + result + " = " + leftTemp + " " + node->op.lexeme + " " + rightTemp);
    }
    return result;
}

/**
 * @brief Generates TAC for a unary NOT expression (e.g., `!x` or `~x`).
 *
 * This function:
 * - Recursively generates TAC for the operand expression.
 * - Allocates a new temporary variable to store the result.
 * - Emits a TAC instruction for the unary operator.
 *
 * Example:
 * ```c
 * bool temp = !x;
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `NotExpression` node representing the unary NOT operation.
 * @return The name of the temporary variable storing the result.
 */
std::string generate(ThreeAddressCodeGenerator &gen, NotExpression *node) {
    std::string temp = generate(gen, &node->expr);
    std::string result = gen.tempGen.newTemp();
    gen.emit(get_type(node->type) + result + " = " + node->op.lexeme + temp);
    return result;
}

/**
 * @brief Generates TAC for a literal number.
 *
 * Simply returns the numeric value (lexeme) of the token as a string,
 * removing formatting (e.g., underscores).
 *
 * Example:
 * ```c
 * 42; // Returned directly as "42"
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `NumberASTNode` representing the number literal.
 * @return The string representation of the number.
 */
std::string generate(ThreeAddressCodeGenerator &gen, NumberASTNode *node) {
    auto t = node->token.lexeme;
    t.erase(std::remove(t.begin(), t.end(), '_'), t.end());
    return t;
}

/**
 * @brief Generates TAC for a boolean literal (`true` or `false`).
 *
 * Returns the boolean literal as a string.
 *
 * Example:
 * ```c
 * true; // Directly returned as "true"
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `BooleanASTNode` representing the boolean literal.
 * @return The string representation of the boolean literal.
 */
std::string generate(ThreeAddressCodeGenerator &gen, BooleanASTNode *node) {
    return node->token.lexeme;
}

/**
 * @brief Generates TAC for an assignment statement.
 *
 * Handles assignments in the form:
 * ```java
 * x = expr;    // Simple assignment
 * x += expr;   // Compound assignment
 * ```
 * Example TAC:
 * ```c
 * x = temp1; // Emits TAC for the assignment.
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `Assignment` node representing the assignment statement.
 * @return The name of the temporary variable storing the assigned value.
 */
std::string generate(ThreeAddressCodeGenerator &gen, Assignment *node) {
    std::string value = generate(gen, &node->expression);
    auto ref = generate(gen, &node->reference);
    gen.emit(ref + " " + node->assignmentToken.lexeme + " " + value);
    return value;
}

/**
 * @brief Generates TAC for a `return` statement.
 *
 * If the `return` statement has an associated expression, the expression is
 * resolved into TAC before generating the `return` instruction.
 *
 * Example:
 * ```java
 * return;       // Emits `return` TAC instruction
 * return x + y; // Emits `return temp1;`
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `ReturnStatement` node.
 * @return An empty string as no intermediate result is produced.
 */
std::string generate(ThreeAddressCodeGenerator &gen, ReturnStatement *node) {
    if (node->expr) {
        std::string value = generate(gen, &node->expr);
        gen.emit("return " + value);
    } else {
        gen.emit("return");
    }
    return "";
}

/**
 * @brief Generates TAC for a reference chain (e.g., variables, fields, or arrays).
 *
 * Delegates the actual generation to the underlying `ReferenceChain` object for resolution.
 *
 * Example:
 * ```java
 * obj.field; // TAC might resolve as "temp1 = obj.field;"
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `ReferenceASTNode`.
 * @return The reference name as a string.
 */
std::string generate(ThreeAddressCodeGenerator &gen, ReferenceASTNode *node) {
    return generate(gen, &node->reference);
}

/**
 * @brief Generates Three-Address Code (TAC) for a block of statements.
 *
 * This function processes a sequence of statements within a code block (enclosed by `{}`),
 * maintaining proper scope and formatting. It handles:
 * - Opening and closing of scopes
 * - Sequential generation of statements
 * - Proper spacing between statements
 *
 * Example Mini-Java:
 * ```java
 * {
 *     A a = new A();
 *     a.field = 42;
 *     System.out.println(x);
 * }
 * ```
 *
 * Generated TAC Structure:
 * ```c
 * {
 *     A *a;
 *     a = $_new_A();
 *     a->field = 42;
 *
 *     int tmp = a->field;
 *     printf("%d\n", tmp);
 * }
 * ```
 *
 * @param gen The TAC generator context for managing blocks and emission.
 * @param node The CodeBlock node containing a vector of statements to process.
 * @return An empty string as code blocks don't produce a value.
 *
 * Implementation Details:
 * - Opens a new scope block for variable isolation
 * - Processes each statement in sequence
 * - Adds newlines between statements (except for specific cases)
 * - Closes the scope block when done
 */
std::string generate(ThreeAddressCodeGenerator &gen, CodeBlock *node) {
    gen.openBlock();
    unsigned long l = node->codes.size();
    for (int i = 0; i < l; i++) {
        auto n = node->codes[i].get();
        generate(gen, n);

        if (i != l - 1 &&
            n->getType() != ASTType::AST_LocalVariableASTNode &&
            n->getType() != ASTType::AST_Assignment) {
            gen.newLine();
        }
    }
    gen.closeBlock();
    return "";
}

/**
 * @brief Generates Three-Address Code (TAC) for a local variable declaration.
 *
 * This function handles the declaration of local variables within a method or block scope.
 * It:
 * 1. Emits the variable declaration with appropriate type.
 * 2. Registers the variable in the current scope for later reference.
 *
 * Example Mini-Java:
 * ```java
 * int x;
 * MyClass obj;
 * ```
 *
 * Generated TAC:
 * ```c
 * int x;
 * MyClass *obj;
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The LocalVariableASTNode containing the field information.
 * @return An empty string as declarations don't produce a value.
 *
 * Note: Variable initialization, if any, is handled separately by Assignment nodes.
 */
std::string generate(ThreeAddressCodeGenerator &gen, LocalVariableASTNode *node) {
    gen.emit(get_type(&node->field) + node->field.getName());
    gen.addVariable(node->field.getName(), node->field.getTypeLexeme());
    return "";
}

/**
 * @brief Generates Three-Address Code (TAC) for a type cast expression.
 *
 * Handles explicit type casting operations in Mini-Java, generating appropriate
 * cast operations in the TAC. This includes:
 * - Primitive type casts (e.g., int to boolean)
 * - Object type casts (e.g., subclass to superclass)
 *
 * Example Mini-Java:
 * ```java
 * (ParentClass)childObj
 * (int)someValue
 * ```
 *
 * Generated TAC:
 * ```c
 * ParentClass *temp1 = (ParentClass *)childObj;
 * int temp2 = (int)someValue;
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The CastExpression node containing the target type and expression.
 * @return The name of the temporary variable holding the cast result.
 *
 * Implementation Details:
 * 1. Generates code for the expression being cast
 * 2. Creates a new temporary variable for the result
 * 3. Emits the cast operation with appropriate type information
 */
std::string generate(ThreeAddressCodeGenerator &gen, CastExpression *node) {
    std::string exprTemp = generate(gen, &node->expr);
    std::string resultTemp = gen.tempGen.newTemp();
    gen.emit(get_type(node->type) + resultTemp + " = (" + get_type(node->type) + ") " + exprTemp);
    return resultTemp;
}

/**
 * @brief Generates TAC for an `if` statement, including optional `else` branches.
 *
 * Creates labels (`then`, `else`, and `end`) for control flow:
 * - If the condition evaluates to `false`, TAC jumps to the `else` or `end` label.
 * - If there is no `else`, execution directly jumps to `end`.
 *
 * Example:
 * ```java
 * if (condition) { ... } else { ... }
 * ```
 * TAC Output:
 * ```c
 * if (!condition) goto if_else;
 * if_then:
 *     ... // Then block
 * goto if_end;
 * if_else:
 *     ... // Else block
 * if_end:
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `IfStatement` node representing the conditional statement.
 * @return An empty string as no intermediate result is produced.
 */
std::string generate(ThreeAddressCodeGenerator &gen, IfStatement *node) {
    std::string conditionTemp = generate(gen, &node->condition);

    std::string thenLabel = gen.labelGen.newLabel("if_then");
    std::string endLabel = gen.labelGen.newLabel("if_end");
    std::string elseLabel;

    if (node->elseBody) {
        elseLabel = gen.labelGen.newLabel("if_else");
    }

    if (node->elseBody) {
        gen.emit("if (" + not_condition(conditionTemp) + ") goto " + elseLabel);
    } else {
        gen.emit("if (" + not_condition(conditionTemp) + ") goto " + endLabel);
    }

    gen.emitLabel(thenLabel);

    generate(gen, node->body.get());
    gen.emit("goto " + endLabel);

    if (node->elseBody) {
        gen.emitLabel(elseLabel);
        generate(gen, node->elseBody.get());
    }

    gen.emitLabel(endLabel);
    return "";
}

/**
 * @brief Generates TAC for a `while` or 'do-while' loop.
 *
 * Creates labels for the loop:
 * - `start`: For evaluating the condition or start of 'do-while' body.
 * - `end`: For breaking out of the loop.
 * The condition is negated, and execution jumps to `end` if it is false.
 *
 * Example:
 * ```java
 * while (x > 0) { ... }
 * ```
 * TAC Output:
 * ```c
 * while_start:
 * if (!condition) goto while_end;
 *     {...} // Loop body
 * goto while_start;
 * while_end:
 * ```
 *
 * @param gen The TAC generator context.
 * @param node The `WhileStatement` node.
 * @return An empty string as no intermediate result is produced.
 */
std::string generate(ThreeAddressCodeGenerator &gen, WhileStatement *node) {
    std::string startLabel = gen.labelGen.newLabel("while_start");
    std::string endLabel = gen.labelGen.newLabel("while_end");
    gen.pushLabel(startLabel, endLabel);
    gen.emitLabel(startLabel);
    if (node->isDoWhile) {
        generate(gen, node->body.get());
        std::string conditionTemp = generate(gen, &node->condition);
        gen.emit("if (" + not_condition(conditionTemp) + ") goto " + endLabel);
    } else {
        std::string conditionTemp = generate(gen, &node->condition);
        gen.emit("if (" + not_condition(conditionTemp) + ") goto " + endLabel);
        generate(gen, node->body.get());
    }
    gen.emit("goto " + startLabel);
    gen.emitLabel(endLabel);
    gen.popLabel();
    return "";
}

/**
 * @brief Generates Three-Address Code (TAC) for a `for` loop statement.
 *
 * This function translates a Mini-Java `for` loop into TAC by creating appropriate labels
 * and control flow instructions. The structure follows:
 * 1. Initialization
 * 2. Condition check
 * 3. Loop body
 * 4. Update statement
 * 5. Jump back to condition
 *
 * Example Mini-Java:
 * ```java
 * for (int i = 0; i < 10; i++) {
 *     // body
 * }
 * ```
 *
 * Generated TAC Structure:
 * ```
 * {
 *     // Initialization
 *     int i = 0;
 *
 * for_start:
 *     // Condition
 *     bool tmp = i < 10;
 *     if (!tmp) goto for_end;
 * for_body:
 *     // Loop body
 * for_update:
 *     i++;
 *     goto for_start;
 * for_end:
 * }
 * ```
 *
 * @param gen The TAC generator context containing label generation and emission functions.
 * @param node The ForStatement AST node containing initialization, condition, update, and body.
 * @return An empty string as for-loops don't produce a value.
 *
 * Implementation Details:
 * - Opens a new block scope for the loop.
 * - Freezes the scope during initialization to prevent variable shadowing.
 * - Creates labels for:
 *   - start: Loop condition check
 *   - body: Loop body execution
 *   - update: Increment/update statement
 *   - end: Loop exit point
 * - Pushes update/end labels for break/continue statement resolution.
 * - Generates code for each component (init, condition, body, update).
 * - Closes the block scope when done.
 */
std::string generate(ThreeAddressCodeGenerator &gen, ForStatement *node) {
    gen.openBlock();
    gen.freeze(true);
    if (node->initialization) {
        generate(gen, node->initialization.get());
    }
    gen.freeze(false);
    std::string startLabel = gen.labelGen.newLabel("for_start");
    std::string bodyLabel = gen.labelGen.newLabel("for_body");
    std::string updateLabel = gen.labelGen.newLabel("for_update");
    std::string endLabel = gen.labelGen.newLabel("for_end");
    gen.pushLabel(updateLabel, endLabel);

    gen.emitLabel(startLabel);
    if (node->condition) {
        std::string conditionTemp = generate(gen, node->condition.get());;
        gen.emit("if (" + not_condition(conditionTemp) + ") goto " + endLabel);
    }
    gen.emitLabel(bodyLabel);
    if (node->body) {
        generate(gen, node->body.get());
    }
    gen.emitLabel(updateLabel);
    if (node->update) {
        generate(gen, node->update.get());
    }
    gen.emit("goto " + startLabel);
    gen.emitLabel(endLabel);
    gen.popLabel();
    gen.closeBlock();
    return "";
}

/**
 * @brief Handles generic dispatch for generating TAC from various AST nodes.
 *
 * This acts as the main entry point for TAC generation and dispatches based on the
 * type of the AST node. For unsupported nodes, it produces no TAC.
 *
 * @param gen The TAC generator context.
 * @param node The `ASTNode` to process.
 * @return The resulting C code or a temporary variable name, depending on the node.
 */
std::string generate(ThreeAddressCodeGenerator &gen, ASTNode *node) {
    if (!node) {
        return "";
    }

    switch (node->getType()) {
        case ASTType::AST_BinaryExpression:
            return generate(gen, (BinaryExpression *) node);
        case ASTType::AST_NumberASTNode:
            return generate(gen, (NumberASTNode *) node);
        case ASTType::AST_Assignment:
            return generate(gen, (Assignment *) node);
        case ASTType::AST_CodeBlock:
            return generate(gen, (CodeBlock *) node);
        case ASTType::AST_NotExpression:
            return generate(gen, (NotExpression *) node);
        case ASTType::AST_CastExpression:
            return generate(gen, (CastExpression *) node);
        case ASTType::AST_ReferenceASTNode:
            return generate(gen, (ReferenceASTNode *) node);
        case ASTType::AST_BooleanASTNode:
            return generate(gen, (BooleanASTNode *) node);
        case ASTType::AST_LocalVariableASTNode:
            return generate(gen, (LocalVariableASTNode *) node);
        case ASTType::AST_IfStatement:
            return generate(gen, (IfStatement *) node);
        case ASTType::AST_WhileStatement:
            return generate(gen, (WhileStatement *) node);
        case ASTType::AST_ForStatement:
            return generate(gen, (ForStatement *) node);
        case ASTType::AST_ReturnStatement:
            return generate(gen, (ReturnStatement *) node);
        case ASTType::AST_BreakStatement:
            gen.breakNow();
            return "";
        case ASTType::AST_ContinueStatement:
            gen.continueNow();
            return "";
        case ASTType::AST_MethodCall:
        case ASTType::AST_ArrayCall:
        case ASTType::AST_NewObject:
            break;
    }
    return "";
}

std::string generate(ThreeAddressCodeGenerator &gen, std::unique_ptr<ASTNode> *node) {
    return generate(gen, node->get());
}