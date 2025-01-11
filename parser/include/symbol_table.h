#ifndef SIMPLEMINIJAVACOMPILERTOC_SYMBOL_TABLE_H
#define SIMPLEMINIJAVACOMPILERTOC_SYMBOL_TABLE_H

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../../common/include/error_handler.h"

/**
 * @struct Symbol
 * @brief Represents an entry (symbol) in the symbol table.
 *
 * A `Symbol` is used to represent variables, methods, fields, or classes in the
 * Mini-Java symbol table. It encapsulates:
 * - **Name**: The identifier associated with the symbol.
 * - **Type**: The resolved type of the symbol (e.g., "int", "boolean", or a class name).
 * - **isMethod**: Whether the symbol represents a method.
 * - **Parameters**: For methods, the list of parameter types.
 * - **Return Type**: For methods, the method's return type.
 *
 * Example Symbol Usage:
 * - Variable: `int x;` → `{name: "x", type: "int", isMethod: false}`
 * - Method: `int add(int a, boolean flag);` →
 *   `{name: "add", type: "method", isMethod: true, params: ["int", "boolean"], returnType: "int"}`
 */
struct Symbol {
    std::string name;           ///< The name of the symbol (e.g., variable, method, field, or class name).
    std::string type;           ///< The type of the symbol (e.g., "int", "boolean", or "MyClass").
    bool isMethod = false;      ///< A flag indicating whether this symbol represents a method.
    std::vector<std::string> params; ///< List of parameter types (applicable for methods).
    std::string returnType;     ///< For methods, the return type of the method (e.g., "int", "void").

    /**
     * @brief Constructor for a non-method symbol.
     * @param name_ The name of the symbol.
     * @param type_ The type of the symbol.
     */
    Symbol(std::string name_, std::string type_)
            : name(std::move(name_)), type(std::move(type_)) {}

    /**
     * @brief Constructor for a method symbol.
     * @param name_ The name of the method.
     * @param type_ The type of the symbol (typically "method").
     * @param isMethod_ A boolean indicating that this is a method symbol.
     * @param params_ A list of parameter types for the method.
     * @param returnType_ The return type of the method.
     */
    Symbol(std::string name_, std::string type_, bool isMethod_, std::vector<std::string> params_,
           std::string returnType_)
            : name(std::move(name_)), type(std::move(type_)), isMethod(isMethod_),
              params(std::move(params_)), returnType(std::move(returnType_)) {}
};

/**
 * @class SymbolTable
 * @brief Represents a hierarchical symbol table for Mini-Java.
 *
 * The `SymbolTable` manages the resolution of variables, methods, and classes in the
 * Mini-Java compiler. It supports a hierarchical structure for scoping, linking child
 * scopes to parent scopes, and managing class-level visibility.
 *
 * Features:
 * - **Instance-level symbols**: Tracks symbols (variables, methods, etc.) defined in a specific scope.
 * - **Parent scopes**: Supports nested scopes (e.g., method-level symbols inside a class-level scope).
 * - **Class-level symbol management**: Allows global registration and retrieval of class symbol tables for
 *   inheritance and type checking.
 * - **Type-checking utilities**: Provides utilities for resolving types and checking casting relationships.
 */
class SymbolTable {
private:
    /// Symbol table for the current scope (maps identifiers to their `Symbol` entries).
    std::unordered_map<std::string, Symbol> symbols;

    /// Pointer to the parent scope, if this scope is nested (e.g., method inside a class).
    SymbolTable *parentScope = nullptr;

    /// The name of the class that this scope represents (empty if not a class scope).
    std::string className;

    /// The return type of the current method, if applicable (empty if not inside a method scope).
    std::string returnType;

    /// A global registry of class-level symbol tables for managing inheritance and classes.
    static std::unordered_map<std::string, std::shared_ptr<SymbolTable>> classSymbolTables;

public:
    /**
     * @brief Constructs a new symbol table for a nested scope.
     * @param parent The parent symbol table (e.g., a method's local variables).
     */
    explicit SymbolTable(SymbolTable *parent = nullptr);

    /**
     * @brief Constructs a new symbol table for a method scope.
     * @param parent The parent scope of the method.
     * @param returnType_ The return type of the method.
     */
    explicit SymbolTable(SymbolTable *parent, std::string returnType_);

    /**
     * @brief Constructs a new symbol table for a class scope.
     * @param className_ The name of the class represented by this symbol table.
     * @param parent The parent symbol table (typically global scope).
     */
    explicit SymbolTable(std::string className_, SymbolTable *parent = nullptr);

    /**
     * @brief Adds a new symbol to the current scope.
     * @param name The name of the symbol.
     * @param symbol The `Symbol` object to associate with the name.
     */
    void addSymbol(const std::string &name, const Symbol &symbol);

    /**
     * @brief Searches for a symbol in the current or its parent scope.
     * @param name The name of the symbol to find.
     * @return A pointer to the `Symbol` if it exists in the current scope
     *  or parents, otherwise `nullptr`.
     */
    Symbol *lookup(const std::string &name);

    /**
     * @brief Searches for a symbol only in the current scope, without checking parent scopes.
     * @param name The name of the symbol to search for.
     * @return A pointer to the `Symbol` if found in the current scope,
     *  otherwise `nullptr`.
     */
    Symbol *find(const std::string &name);

    /**
     * @brief Adds a class-level symbol table to the global registry.
     * @param className The name of the class to register.
     * @param table The symbol table representing the class.
     */
    static void addClassSymbolTable(const std::string &className, const SymbolTable &table);

    /**
     * @brief Retrieves the symbol table of a registered class.
     * @param className The name of the class.
     * @return A pointer to the symbol table of the class, or `nullptr` if not found.
     */
    static SymbolTable *getClassSymbolTable(const std::string &className);

    /**
     * @brief Checks if one type can be casted to another (e.g., for class inheritance).
     * @param from The source type.
     * @param to The target type.
     * @return `true` if casting is allowed, otherwise `false`.
     */
    static bool canCast(const std::string &from, const std::string &to);

    /**
     * @brief Checks if this scope represents a class-level scope.
     * @return `true` if it's a class scope, otherwise `false`.
     */
    bool isClassScope() const { return !className.empty(); }

    /**
     * @brief Retrieves the name of the current class represented by this symbol table.
     * @return The class name, or an empty string if not in a class scope.
     */
    std::string getClassName() const { return className; }

    /**
     * @brief Retrieves the parent symbol table of the current scope.
     * @return A pointer to the parent symbol table, or `nullptr` if this is the root scope.
     */
    SymbolTable *getParent() const { return parentScope; }

    /**
     * @brief Retrieves the return type of the current method's scope.
     * @return The return type as a string, or an empty string if no return type is defined.
     */
    std::string getReturnType() const { return returnType; }

    /**
     * @brief Retrieves the symbol table for the current class scope.
     * @return A pointer to the current class's symbol table, or `nullptr` if not in a class scope.
     */
    SymbolTable *getCurrentClassSymbolTable();
};

#endif // SIMPLEMINIJAVACOMPILERTOC_SYMBOL_TABLE_H