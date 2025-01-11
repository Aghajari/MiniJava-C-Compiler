#include "../include/parser.h"
#include "../internal/streamer.h"
#include "../internal/parser_internal.h"

/**
 * @brief Adds built-in Java system classes and their methods/fields to the global symbol table.
 *
 * This function registers two built-in entities in the symbol table:
 * 1. The `System` class, including:
 *    - `out`: Represents the standard output (e.g., `System.out`).
 *    - `println(int)`, `print(int)`, and `printf(int)`: Built-in methods for printing integers.
 * 2. The `int[]` type, which represents arrays, including:
 *    - `length`: A field representing the size of the array.
 *
 * These system classes must be included before semantic analysis to allow references like `System.out.println()` or `array.length`.
 *
 * Example:
 * ```java
 * System.out.println(42); // Must resolve to the built-in `println` function.
 * int[] arr = new int[10];
 * int size = arr.length; // Must resolve to the built-in `length` field.
 * ```
 */
void addJavaSystemToSymbolTable() {
    SymbolTable system = SymbolTable("System");
    system.addSymbol("out", Symbol("out", "System"));
    system.addSymbol("println", Symbol("println", "void", true, {"int"}, "void"));
    system.addSymbol("print", Symbol("print", "void", true, {"int"}, "void"));
    system.addSymbol("printf", Symbol("printf", "void", true, {"int"}, "void"));
    SymbolTable::addClassSymbolTable("System", system);

    SymbolTable intArray = SymbolTable("int[]");
    intArray.addSymbol("length", Symbol("length", "int"));
    SymbolTable::addClassSymbolTable("int[]", intArray);
}

/**
 * @brief Performs semantic analysis on the parsed `Project` to validate and prepare symbol tables.
 *
 * After parsing the `Project` into class, field, and method containers, semantic analysis is performed to:
 * 1. Add all classes and their members (methods, fields) to the global `SymbolTable`.
 * 2. Topologically sort the classes based on inheritance to ensure that superclasses are processed before subclasses.
 * 3. Validate method bodies and ensure that all variables and types are resolved correctly in the respective scopes.
 *
 * **Steps**:
 * - Register all classes into the global symbol table.
 * - For each method, validate its `CodeBlock` by resolving symbols in the appropriate scope (class scope or method scope).
 *
 * @param project The parsed `Project` containing classes, fields, and methods to analyze.
 */
void semanticAnalysis(Project &project) {
    auto sortedClasses = project.getTopologicalSort();
    addJavaSystemToSymbolTable();

    for (auto &className: sortedClasses) {
        auto clazz = project.getClassByName(className);
        SymbolTable classTable = SymbolTable(clazz->getName(), SymbolTable::getClassSymbolTable(clazz->getExtends()));
        for (auto &field: *clazz->getFields()) {
            classTable.addSymbol(field.getName(), Symbol(field.getName(), field.getTypeLexeme()));
        }
        classTable.addSymbol("System", Symbol("System", "System"));

        for (auto &method: *clazz->getMethods()) {
            std::vector<std::string> params;
            for (auto &param: *method.getParams()) {
                params.push_back(param.getTypeLexeme());
            }
            classTable.addSymbol(method.getName(), Symbol(
                    method.getName(),
                    method.getReturnTypeLexeme(),
                    true,
                    params,
                    method.getReturnTypeLexeme()
            ));
        }
        SymbolTable::addClassSymbolTable(clazz->getName(), classTable);
    }

    for (auto &className: sortedClasses) {
        auto clazz = project.getClassByName(className);
        auto classScope = SymbolTable::getClassSymbolTable(clazz->getName());
        for (auto &method: *clazz->getMethods()) {
            if (method.isMain()) {
                SymbolTable globalScope = SymbolTable("System");
                globalScope.addSymbol("System", Symbol("System", "System"));
                method.getCodeBlock()->analyseSemantics(globalScope);
                continue;
            }

            auto methodScope = SymbolTable(classScope, method.getReturnTypeLexeme());
            for (auto &param: *method.getParams()) {
                methodScope.addSymbol(param.getName(), Symbol(param.getName(), param.getTypeLexeme()));
            }
            method.getCodeBlock()->analyseSemantics(methodScope);
        }
    }
}

/**
 * @brief Parses Mini-Java source code and performs semantic analysis.
 *
 * @param source The Mini-Java source code to parse.
 * @return A `Project` object representing the parsed structure of the program.
 *
 * **Steps**:
 * - Tokenize the input source code using a `TokenStreamer`.
 * - Parse the class definitions into a `Project` object.
 * - Perform semantic analysis using `semanticAnalysis` to resolve types and validate symbols.
 */
Project parse(const std::string &source) {
    Project project = Project();

    TokenStreamer streamer = TokenStreamer(source);
    while (streamer.hasToken()) {
        if (!parseClass(project, streamer)) {
            break;
        }
    }
    semanticAnalysis(project);
    return project;
}