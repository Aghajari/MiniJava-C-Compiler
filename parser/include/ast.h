#ifndef SIMPLEMINIJAVACOMPILERTOC_AST_H
#define SIMPLEMINIJAVACOMPILERTOC_AST_H

#include "field.h"
#include "symbol_table.h"
#include "../../lexer/include/token.h"

/**
 * @enum ASTType
 * @brief An enumeration of the various types of Abstract Syntax Tree (AST) nodes.
 *
 * Each type represents a distinct construct in the Mini-Java language,
 * such as statements, expressions, or other program components.
 */
enum ASTType {
    AST_CodeBlock,
    AST_BinaryExpression,
    AST_NotExpression,
    AST_CastExpression,
    AST_ReturnStatement,
    AST_BreakStatement,
    AST_ContinueStatement,
    AST_NewObject,
    AST_ReferenceASTNode,
    AST_NumberASTNode,
    AST_BooleanASTNode,
    AST_LocalVariableASTNode,
    AST_Assignment,
    AST_MethodCall,
    AST_ArrayCall,
    AST_IfStatement,
    AST_WhileStatement,
    AST_ForStatement,
};

/**
 * @struct ASTNode
 * @brief The base class for all nodes in the Abstract Syntax Tree (AST).
 *
 * All specific ASTNode types inherit from this base class and implement the
 * required virtual methods for printing, resolving types, and retrieving the node's type.
 */
struct ASTNode {
    /// Type of node, resolves after the semantic analysis phase.
    std::string type;

    virtual ~ASTNode() = default;

    /**
     * @brief Prints the AST node's details in a human-readable format.
     * @param strm The output stream to write to.
     * @param depth The depth in the tree (for indentation).
     */
    virtual void print(std::ostream &strm, int depth = 0) const = 0;

    /**
     * @brief Retrieves the type of the AST node.
     * @return An `ASTType` enum value representing this node type.
     */
    virtual ASTType getType() const = 0;

    /**
     * @brief Resolves the type of the AST node within the given symbol table.
     * @param symbolTable The symbol table used for type resolution and validation.
     */
    virtual void analyseSemantics(SymbolTable &symbolTable) = 0;
};

/**
 * @struct CodeBlock
 * @brief Represents a block of code, containing multiple other AST nodes.
 *
 * A code block is typically used as the body of control structures (`if`, `while`, etc.)
 * or as a grouping construct for multiple statements.
 */
struct CodeBlock : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> codes;

    CodeBlock();

    void addCode(std::unique_ptr<ASTNode> &node);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    /**
     * @brief Resolves types for the block but keeps the scope within the same symbol table.
     * @param symbolTable The symbol table used for resolution.
     */
    void analyseSemanticsWithSameScope(SymbolTable &symbolTable);

    ASTType getType() const override {
        return ASTType::AST_CodeBlock;
    }
};

// ------------------ REFERENCE CHAIN -------------------

/**
 * @struct MethodCall
 * @brief Represents a method invocation, including the name, arguments, and caller type.
 *
 * A `MethodCall` AST node is used to model any function calls in Mini-Java code.
 */
struct MethodCall : public ASTNode {
    /// Name of the method being called.
    std::string methodName;
    /// List of arguments passed to the method.
    std::vector<std::unique_ptr<ASTNode>> arguments;
    /// The type of the caller (e.g., the class or object invoking the method).
    /// Sets by ReferenceChain, before calling analyseSemantics() on this node.
    /// Can be used to determine the scope of the field.
    std::string callerType;

    explicit MethodCall(std::string methodName_);

    void addArgument(std::unique_ptr<ASTNode> argument);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_MethodCall;
    }
};

/**
 * @struct ArrayCall
 * @brief Represents access to an array element, including the array name and index.
 *
 * An `ArrayCall` AST node is used for array indexing operations like `arrayName[index]`.
 */
struct ArrayCall : public ASTNode {
    /// Name of the array being accessed.
    std::string arrayName;
    /// The index expression for the array.
    std::unique_ptr<ASTNode> bracket;
    /// The type of the caller (e.g., the class or object accessing the field).
    /// Sets by ReferenceChain, before calling analyseSemantics() on this node.
    /// Can be used to determine the scope of the field.
    std::string callerType;

    ArrayCall(std::string array_name_, std::unique_ptr<ASTNode> bracket_);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_ArrayCall;
    }
};

/**
 * @struct ReferenceChain
 * @brief Represents a chain of successive member accesses, method calls, or object creation.
 *
 * A ReferenceChain models references to objects, fields, and method calls in Mini-Java.
 * It is essential for resolving member variables, method calls, and chaining operations.
 *
 * A `ReferenceChain` AST node can represent expressions like:
 *
 *   - Simple chains: `variableName.fieldName`
 *   - Array chains: `variableName.arrayName[arrayIndex].length`
 *   - Complex chains: `variable.field.method(params...).field`
 *   - Instantiation chains: `new ClassName().method().field`
 *
 * This enables the compiler to resolve variable types, method signatures, and relationships
 * between objects as it traverses the chain. It ensures proper type checking and is critical
 * for generating valid C code.
 */
struct ReferenceChain {

    /// Stores the elements of the reference chain. Each chain element is a pair:
    /// - `Token`: Represents the name or operator (e.g., a field, method, or special keyword like `new`).
    /// - `std::unique_ptr<ASTNode>`: Represents arguments (e.g., method parameters).
    std::vector<std::pair<Token, std::unique_ptr<ASTNode>>> chain;

    /// The resolved type of the reference chain at the point of compilation.
    /// For example:
    /// - For `variable.field`, `type` would store the type of the `field`.
    /// - For `new ClassName().method()`, `type` would store the return type of the `method()`.
    std::string type;

    /// True if the chain is referencing the special `length` field in an array.
    /// Example: `array.length`.
    bool isArrayLength = false;

    /**
     * @brief Adds a field reference to the chain.
     * @param token Token representing the field name (e.g., `.field` in `variable.field`).
     *
     * This method appends a field-only reference (without arguments) to the chain.
     * It is used when processing identifiers or field accesses.
     */
    void addField(const Token &token);

    /**
     * @brief Adds a node reference to the chain.
     * @param token Token representing the reference (e.g., field name, method name, etc.).
     * @param node A pointer to an AST node associated with the reference.
     *
     * This method allows appending elements that include both a reference and arguments:
     * - Method calls (e.g., `method(params...)`).
     * - Subscript operations (e.g., `array[index]`).
     */
    void addNode(const Token &token, std::unique_ptr<ASTNode> node);

    void print(std::ostream &strm, int depth = 0) const;

    /**
     * @brief Resolves the type of the ReferenceChain from its components.
     * @param symbolTable The symbol table used for type resolution and validation.
     *
     * During this process:
     * - Field types are validated and resolved based on the chain's type history.
     * - Method return types are checked and propagated.
     * - Object creation (`new`) is validated against class declarations.
     * - Special cases, like array `length` references, are resolved appropriately.
     */
    void analyseSemantics(SymbolTable &symbolTable);
};

/**
 * @struct BinaryExpression
 * @brief Represents a binary operation (e.g., `x + y`, `a && b`).
 */
struct BinaryExpression : public ASTNode {
    /// Operator node representing the binary operator (e.g., `+`, `&&`).
    Token op;
    /// Left-hand side of the binary expression.
    std::unique_ptr<ASTNode> left;
    /// Right-hand side of the binary expression.
    std::unique_ptr<ASTNode> right;

    BinaryExpression(Token op_, std::unique_ptr<ASTNode> left_, std::unique_ptr<ASTNode> right_);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_BinaryExpression;
    }
};

/**
 * @struct NotExpression
 * @brief Represents a unary "not" expression (`!` or `~`).
 *
 * In Mini-Java, `NotExpression` can only support two unary operators:
 *  - `!` (logical NOT): Applies to boolean expressions. Converts `true` to `false` and `false` to `true`.
 *  - `~` (bitwise NOT): Applies to integers. Flips all bits of the integer in its binary representation (two's complement).
 *
 * This node ensures type validation:
 * - `!` must only target expressions of type `boolean`.
 * - `~` must only target expressions of type `int`.
 *
 * It is resolved during the semantic analysis phase to enforce these rules and properly propagate the resulting type.
 */
struct NotExpression : public ASTNode {
    /// The operator, either `!` (logical NOT) or `~` (bitwise NOT).
    Token op;

    /// The operand (sub-expression) to which the operator is applied.
    std::unique_ptr<ASTNode> expr;

    NotExpression(
            Token op_,
            std::unique_ptr<ASTNode> expr_
    );

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Performs type resolution for the `NotExpression`.
     * @param symbolTable The symbol table used for looking up types.
     *
     * During resolution:
     * - Ensures the operand (`expr`) has the correct type:
     *   - For `!`, the operand must be of type `boolean`.
     *   - For `~`, the operand must be of type `int`.
     * - Sets the resulting type of the expression to:
     *   - `boolean` for `!`.
     *   - `int` for `~`.
     *
     * Errors are raised during the resolution phase if the operand type is invalid.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_NotExpression;
    }
};

/**
 * @struct CastExpression
 * @brief Represents a type-casting operation in the Mini-Java language.
 *
 * A `CastExpression` is used for explicit type conversions between compatible types.
 * For example:
 *
 *   (Parent) child;
 *   (int) someValue;
 *
 * Here, `(Parent)` or `(int)` is the cast, and `child` or `someValue` is the expression being cast.
 *
 * In Mini-Java, casts are typically used to:
 *   - Convert from a subclass to its superclass or vice versa.
 *   - Ensure compatibility in method calls, variable assignments, and expressions.
 *
 * The `CastExpression` enforces type-checking rules during semantic analysis to ensure that:
 *   - The cast type and the operand's type are compatible.
 *   - The cast does not violate Mini-Java's type system, such as casting between unrelated types.
 */
struct CastExpression : public ASTNode {
    /// The token representing the cast type (e.g., `Parent` in `(Parent) child`).
    Token cast;

    /// The sub-expression being cast (e.g., the `child` in `(Parent) child`).
    std::unique_ptr<ASTNode> expr;

    CastExpression(Token cast_, std::unique_ptr<ASTNode> expr_);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_CastExpression;
    }
};

/**
 * @struct ReturnStatement
 * @brief Represents a `return` statement in a function or method in Mini-Java.
 *
 * A `ReturnStatement` is used to exit a function and optionally return a value to
 * the caller. It can either:
 * - Return a value: `return someValue;`
 * - Return nothing (used in `void` functions in standard Java, though Mini-Java does not have `void` methods):
 *   `return;`
 *
 * In Mini-Java, this statement ensures that:
 * - The type of the returned value matches the return type of the method.
 * - A return statement is present for methods that are expected to return a value.
 *
 * This node will be validated during the semantic analysis phase to enforce these requirements.
 */
struct ReturnStatement : public ASTNode {
    /// The expression to return, or `nullptr` if no value is returned.
    std::unique_ptr<ASTNode> expr;

    explicit ReturnStatement(std::unique_ptr<ASTNode> expr_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Performs type resolution and validation for the `ReturnStatement`.
     * @param symbolTable The symbol table used for type resolution and validation.
     *
     * During resolution:
     * - The type of the `expr` (if present) is checked against the return type of the method.
     * - Raises an error during semantic analysis if the types do not match.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_ReturnStatement;
    }
};

/**
 * @struct BreakStatement
 * @brief Represents a `break` keyword in Mini-Java, used to exit a loop prematurely.
 *
 * The `BreakStatement` node is a part of the Abstract Syntax Tree (AST) and is used
 * to model the `break` keyword that is typically used to:
 * - Exit the nearest enclosing loop (`while` or `for`) immediately.
 *
 * In Mini-Java:
 * - A `break` is valid **only when used inside a loop**.
 * - It is the responsibility of the **code generator** to check and raise an error
 *   if a `break` is encountered outside of a valid loop construct.
 */
struct BreakStatement : public ASTNode {

    BreakStatement() = default;

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Dummy type resolution method for the `BreakStatement`.
     * @param symbolTable The symbol table (not used in resolving `break`).
     *
     * Since the placement of a `break` (valid or not) is handled during code generation,
     * this method does not validate loop context.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_BreakStatement;
    }
};

/**
 * @struct ContinueStatement
 * @brief Represents a `continue` keyword in Mini-Java, used to skip the current iteration of a loop.
 *
 * The `ContinueStatement` node is a part of the Abstract Syntax Tree (AST) and is used
 * to model the `continue` keyword. The `continue` statement:
 * - Skips the rest of the code in the current iteration of the nearest enclosing loop
 *   (`while` or `for`).
 * - Transfers control to the loop's next iteration immediately.
 *
 * In Mini-Java:
 * - A `continue` is valid **only when used inside a loop**.
 * - It is the responsibility of the **code generator** to raise an error
 *   if a `continue` is encountered outside of a valid loop construct.
 */
struct ContinueStatement : public ASTNode {

    ContinueStatement() = default;

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Dummy type resolution method for the `ContinueStatement`.
     * @param symbolTable The symbol table (not used in resolving `continue`).
     *
     * Since the placement of a `continue` (valid or not) is handled during code generation,
     * this method does not validate loop context.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_ContinueStatement;
    }
};

/**
 * @struct NewObject
 * @brief Represents an object or array initialization (`new`) in Mini-Java.
 *
 * A `NewObject` node is used to model expressions like:
 * - `new ClassName()` - Instantiation of a class.
 * - `new int[arraySize]` - Instantiation of an array of int with a size `arraySize`.
 *
 * This AST node is commonly used as part of a `ReferenceChain` to model nested calls following the initialization
 * of an object. For example:
 * - `new ClassName().method()` or `new ClassName().field`.
 *
 * The `NewObject` node distinguishes between two forms:
 * - **Object creation**: `new ClassName()` where `arraySize` is `nullptr`.
 * - **Array creation**: `new int[arraySize]` where `arraySize` is not `nullptr`.
 *
 * Semantic analysis ensures that:
 * - The `classType` exists in the symbol table.
 * - If `arraySize` is specified, its type is `int` (only integers are valid for specifying array size).
 */
struct NewObject : public ASTNode {
    /// The type (class name) of the object or array being created.
    Token classType;

    /// The expression defining the size of the array, or `nullptr` if not creating an array.
    std::unique_ptr<ASTNode> arraySize;

    NewObject(Token classType_, std::unique_ptr<ASTNode> array_size_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Performs semantic analysis for the `NewObject` node.
     * @param symbolTable The symbol table to check for the class validity and validate array size (if applicable).
     *
     * During type resolution:
     * - Ensures the `classType` exists in the symbol table as either a class or array type.
     * - If `arraySize` is specified, ensures it resolves to an integer type.
     * - Propagates the resulting type of the node:
     *   - For `new ClassName()`: The resulting type is `ClassName`.
     *   - For `new int[arraySize]`: The resulting type is `int[]`.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_NewObject;
    }
};

/**
 * @struct ReferenceASTNode
 * @brief Represents a reference-based node in the Abstract Syntax Tree (AST).
 *
 * A `ReferenceASTNode` encapsulates a `ReferenceChain` and is used to model expressions that involve:
 * - Simple references, such as a variable or field (`variableName`, `variable.field`).
 * - Method calls (`variable.method(params...)`, `method(params...)`).
 * - Object creation (`new ClassName()` or `new int[size]`).
 * - Complex chains of the above elements (`object.field.method(params...).anotherField` or `new ClassName().method()`).
 *
 * The `ReferenceChain` differentiates between:
 * - A **method call** (e.g., `method(params...)`).
 * - A **new object creation** (e.g., `new ClassName()` or `new ClassName[size]`).
 * - A **chained reference** (e.g., `variable.field.method().anotherField`).
 *
 * During semantic analysis, the node ensures:
 * - Validity of the `ReferenceChain` (e.g., method exists and arguments match the signature, class exists for new objects, correct types for fields).
 * - Proper type propagation (e.g., the type of the last reference in the chain determines this node's type).
 */
struct ReferenceASTNode : public ASTNode {
    ReferenceChain reference;

    explicit ReferenceASTNode(ReferenceChain reference_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Resolves and validates the type of the `ReferenceASTNode`.
     * @param symbolTable The symbol table used for type resolution and validation.
     *
     * During resolution:
     * - Each step in the `ReferenceChain` is validated in sequence:
     *   - Ensures that fields exist in the specified class or object type.
     *   - Ensures that methods exist and their arguments match the method signature.
     *   - Ensures that object or array creation (`new`) uses valid class names and, where applicable, correct array sizes.
     * - Sets the type of the `ReferenceASTNode` to the type of the last element in the chain.
     *
     * This ensures that any use of the `ReferenceASTNode` in the parent node (e.g., in a binary expression)
     * knows the correct type for further validation or code generation.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_ReferenceASTNode;
    }
};

/**
 * @struct NumberASTNode
 * @brief Represents a literal number in Mini-Java, supporting various number formats.
 *
 * `NumberASTNode` models numeric literals in Mini-Java source code. It supports the following formats:
 * - **Decimal (base 10)**: Standard integer literals, e.g., `42`, `1234`.
 * - **Hexadecimal (base 16)**: Prefixed with `0x` or `0X`, e.g., `0xAF12`, `0X1A`.
 * - **Binary (base 2)**: Prefixed with `0b` or `0B`, e.g., `0b1010`, `0B1101`.
 *
 * This node ensures:
 * - Proper parsing and representation of numeric literals in various formats.
 * - Type propagation (`int` is the default type for literals in Mini-Java).
 */
struct NumberASTNode : public ASTNode {
    Token token;

    explicit NumberASTNode(Token token_);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_NumberASTNode;
    }
};

/**
 * @struct BooleanASTNode
 * @brief Represents a boolean literal (`true` or `false`) in the Abstract Syntax Tree (AST).
 *
 * `BooleanASTNode` models Mini-Java boolean literals, which are either `true` or `false`.
 * These literals are used in conditions, assignments, or expressions that require
 * logical operations or decisions.
 *
 * Characteristics:
 * - Its value is always one of two literals: `true` or `false`.
 * - The type of this node is always `boolean`.
 *
 * Semantic analysis ensures the propagation of the correct type (`boolean`) and supports
 * parent constructs like conditional expressions (`if`, `while`).
 */
struct BooleanASTNode : public ASTNode {
    Token token;

    explicit BooleanASTNode(Token token_);

    void print(std::ostream &strm, int depth = 0) const override;

    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_BooleanASTNode;
    }
};

/**
 * @struct LocalVariableASTNode
 * @brief Represents the declaration of a local variable inside a method body in Mini-Java.
 *
 * The `LocalVariableASTNode` strictly handles the **declaration** of a local variable, without any initialization.
 * If an initialization is present (e.g., `int x = 24;`), the declaration (handled by `LocalVariableASTNode`)
 * and the initialization (handled by a separate `Assignment` node) are represented as **two independent nodes**.
 *
 * This split allows a clean and modular **Abstract Syntax Tree (AST)** representation of the program.
 * Declarations and statements (e.g., assignments) are handled separately, making it easier to track scope,
 * validate types, and perform code generation.
 *
 * Example Code and Corresponding Nodes:
 * ```java
 * int x = 24;
 * ```
 * This is split into:
 * - A `LocalVariableASTNode` for the **declaration**:
 *   ```java
 *   int x;
 *   ```
 * - An `Assignment` node for the **initializer**:
 *   ```java
 *   x = 24;
 *   ```
 */
struct LocalVariableASTNode : public ASTNode {
    Field field;

    explicit LocalVariableASTNode(Field field_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Resolves the type of the local variable and performs semantic analysis.
     * @param symbolTable The symbol table used for managing the local scope and checking for conflicts.
     *
     * During resolution:
     * - Ensures the declared type exists and is valid (e.g., `int`, `boolean`, or a custom class).
     * - Inserts the variable into the symbol table to track its declaration within the local scope.
     * - Confirms that no duplicate variable exists in the same scope.
     *
     * Errors during semantic analysis:
     * - If a variable is declared with an undefined type.
     * - If a variable with the same name has already been declared in the current scope.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_LocalVariableASTNode;
    }
};

/**
 * @struct Assignment
 * @brief Represents an assignment operation in Mini-Java.
 *
 * The `Assignment` node models assignments in the Mini-Java source code. Assignments involve:
 * - A **left-hand side (LHS)**: What the value is being assigned to.
 * - An **assignment operator**: `=`, `+=`, `-=`, `*=`, `/=`, '&=', '|=', '^='.
 * - A **right-hand side (RHS)**: The value or expression being assigned.
 *
 * The LHS can be any of the following:
 * - A simple variable: `x = value`.
 * - Array element: `arr[index] -= value`.
 * - A property or field: `object.field *= value`.
 * - A result from a method call, e.g., `this.test().field /= value`.
 * - An object creation and property: `new Object().field ^= value`.
 *
 * The RHS is any expression or value that results in a compatible type for the LHS.
 *
 * During semantic analysis:
 * - Validates that the LHS can legally be assigned to.
 * - Ensures the type of the RHS matches the type of the LHS.
 */
struct Assignment : public ASTNode {
    /// The left-hand side of the assignment (e.g., variable, array access, object field).
    ReferenceChain reference;

    /// The assignment operator (`=`, `+=`, `-=`, `*=`, `/=`, '^=', '|=', '&=').
    Token assignmentToken;

    /// The right-hand side of the assignment, which evaluates to the value being assigned.
    std::unique_ptr<ASTNode> expression;


    Assignment(ReferenceChain reference_, Token token_, std::unique_ptr<ASTNode> expression_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Resolves and validates the assignment during semantic analysis.
     * @param symbolTable The symbol table for resolving variable and type definitions.
     *
     * During resolution:
     * - Resolves the type of the LHS (`reference`) and validates its existence and mutability.
     *   - Ensures the LHS points to a valid variable, array element, or object field.
     * - Resolves the type of the RHS (`expression`) and ensures it is compatible with the LHS type.
     *
     * Example Validations:
     * - Check if `x` exists and can be assigned.
     * - For `arr[index]`, check that `arr` is an array and `index` is an integer.
     * - For `object.field`, ensure the object type has the specified `field`.
     * - For `new Object().field = value`, ensure the `field` exists in the object's type.
     *
     * Errors Raised:
     * - If the LHS is not assignable (e.g., assigning to a non-variable or undefined field).
     * - If the types of LHS and RHS are incompatible.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_Assignment;
    }
};

/**
 * @struct IfStatement
 * @brief Represents an `if` statement in Mini-Java, including optional `else` or `else if` branches.
 *
 * The `IfStatement` node models conditional branching in Mini-Java. A typical `if` statement consists of:
 * - A **condition**: A boolean expression that is evaluated for truth.
 * - A **body**: A block of code or a single statement executed when the condition is `true`.
 * - An optional **`else` body**: A block of code (or another `IfStatement` for `else if`) executed if the condition is `false`.
 *
 * This AST node supports single statements or full blocks for the body and `else` parts without requiring braces `{}`.
 */
struct IfStatement : public ASTNode {
    /// The `if` condition, which must evaluate to a boolean type.
    std::unique_ptr<ASTNode> condition;

    /// The main body of the `if` block, either a `CodeBlock` or a single statement.
    std::unique_ptr<CodeBlock> body;

    /// The optional `else` body, which can be:
    /// - Another `CodeBlock` or `IfStatement`.
    /// - `nullptr` if there is no `else`.
    std::unique_ptr<ASTNode> elseBody;

    IfStatement(std::unique_ptr<ASTNode> condition_,
                std::unique_ptr<CodeBlock> body_,
                std::unique_ptr<ASTNode> else_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Resolves the types and validates the `IfStatement` during semantic analysis.
     * @param symbolTable The symbol table used for resolving variable declarations and types.
     *
     * During resolution:
     * - Ensures the condition is of type `boolean`.
     *
     * Example Validations:
     * - The condition should evaluate to `boolean`. For example:
     *   ```java
     *   if (42) { } // Error: Condition must be boolean.
     *   ```
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_IfStatement;
    }
};

/**
 * @struct WhileStatement
 * @brief Represents a `while` loop in Mini-Java.
 *
 * The `WhileStatement` node models iterative behavior in Mini-Java, allowing a block of code
 * to repeatedly execute as long as a given condition evaluates to `true`.
 *
 * A `WhileStatement` consists of:
 * - A **condition**: A boolean expression that determines if the loop should execute.
 * - A **body**: The block of code or a single statement to execute while the condition is `true`.
 *
 * Similar to an `IfStatement`, the body may be:
 * - A single statement (if no braces `{}` are used in the source code).
 * - A `CodeBlock` if braces `{...}` exist.
 *
 * Example Mini-Java Code:
 * - Simple while loop:
 *   ```java
 *   while (x > 0) {
 *       x--;
 *   }
 *   ```
 * - While loop without braces:
 *   ```java
 *   while (x > 0)
 *       x--; // Single statement body
 *   ```
 */
struct WhileStatement : public ASTNode {
    /// The condition controlling the loop, which must evaluate to a boolean type.
    std::unique_ptr<ASTNode> condition;

    /// The loop body, either a `CodeBlock` or a single statement.
    std::unique_ptr<CodeBlock> body;

    WhileStatement(std::unique_ptr<ASTNode> condition_,
                   std::unique_ptr<CodeBlock> body_);

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Resolves the types and validates the `WhileStatement` during semantic analysis.
     * @param symbolTable The symbol table used for resolving variable declarations and types.
     *
     * During resolution:
     * - Ensures the condition is of type `boolean`.
     *
     * Example Validations:
     * - The condition must evaluate to a `boolean`. For example:
     *   ```java
     *   while (42) { } // Error: Condition must be boolean.
     *   ```
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_WhileStatement;
    }
};

/**
 * @struct ForStatement
 * @brief Represents a `for` loop in Mini-Java.
 *
 * The `ForStatement` node models a `for` loop. It consists of:
 * - An **initialization** block: Executed once before the loop starts (typically variable declaration or assignment).
 * - A **condition**: A boolean expression checked before each iteration.
 * - An **update** block: Executed at the end of each iteration.
 * - A **body**: The block of code or statement executed during each iteration.
 *
 * Example Mini-Java Code:
 * - Simple for loop:
 *   ```java
 *   for (int i = 0; i < 10; i++) {
 *       System.out.println(i);
 *   }
 *   ```
 * - For loop with only condition:
 *   ```java
 *   for (; x > 0; x--) {
 *       System.out.println(x);
 *   }
 *   ```
 * - Infinite for loop:
 *   ```java
 *   for (;;) {
 *       // Do something
 *   }
 *   ```
 *
 * During semantic analysis:
 * - The initialization, condition, and update parts are validated.
 * - The condition must evaluate to a `boolean`.
 * - Validates the body for correct expressions and/or statements.
 */
struct ForStatement : public ASTNode {
    /// The initialization block, executed once before the loop starts.
    /// Example: `int i = 0`.
    std::unique_ptr<CodeBlock> initialization;

    /// The loop condition that determines whether the loop should continue.
    /// Must evaluate to a `boolean`.
    std::unique_ptr<ASTNode> condition;

    /// The update block, executed at the end of each iteration.
    /// Example: `i++`.
    std::unique_ptr<CodeBlock> update;

    /// The main body of the `for` loop, executed during each iteration.
    /// Can be a `CodeBlock` or a single statement.
    std::unique_ptr<CodeBlock> body;

    ForStatement(
            std::unique_ptr<CodeBlock> initialization_,
            std::unique_ptr<ASTNode> condition_,
            std::unique_ptr<CodeBlock> update_,
            std::unique_ptr<CodeBlock> body_
    );

    void print(std::ostream &strm, int depth = 0) const override;

    /**
     * @brief Resolves the types and validates the `ForStatement` during semantic analysis.
     * @param symbolTable The symbol table used for resolving variable declarations and types.
     *
     * During resolution:
     * - Validates each block (initialization, condition, update, body).
     * - Ensures the condition evaluates to a `boolean`.
     * - Ensures all variables used in the condition and/or update are defined and correctly resolved.
     *
     * Example Validations:
     * - Ensure `int i` is correctly initialized in the loop.
     * - The condition `i < 10` must resolve to `boolean`.
     * - The update `i++` must be valid.
     */
    void analyseSemantics(SymbolTable &symbolTable) override;

    ASTType getType() const override {
        return ASTType::AST_ForStatement;
    }
};

#endif // SIMPLEMINIJAVACOMPILERTOC_AST_H