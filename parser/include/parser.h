#ifndef SIMPLEMINIJAVACOMPILERTOC_PARSER_H
#define SIMPLEMINIJAVACOMPILERTOC_PARSER_H

#include "project.h"

/**
 * @brief Parses the Mini-Java source code and produces a `Project` object.
 *
 * The `parse` function is the main entry point to the parsing process. It converts the source code
 * into an in-memory representation of the program as a `Project`, which encapsulates all the classes,
 * fields, and methods in the source code.
 *
 * Parsing is typically split into several stages:
 * 1. **Lexical Analysis (Lexing)**:
 *    - The raw source code is broken into a sequence of tokens (keywords, identifiers, symbols, etc.).
 * 2. **Syntactic Analysis (Parsing)**:
 *    - The tokens are analyzed according to Mini-Java's grammar to construct higher-level structures
 *      like classes, fields, methods, statements, and expressions.
 * 3. **AST Construction**:
 *    - The parsed structure is represented as an **Abstract Syntax Tree (AST)** or similar hierarchical format.
 *
 * **Example Usage**:
 * ```cpp
 * try {
 *     Project project = parse(sourceCode);
 *     // The 'project' object now contains the full program representation for further analysis.
 * } catch (const std::runtime_error &e) {
 *     std::cerr << "Parsing Error: " << e.what() << std::endl;
 * }
 * ```
 *
 * **Example Mini-Java Code**:
 * ```java
 * class A {
 *     int x;
 *     void foo() {
 *         x = 42;
 *     }
 * }
 * ```
 *
 * Parsing this code will create:
 * - A `Project` object containing the class `A`.
 * - A `Class` object with:
 *   - A field `x` of type `int`.
 *   - A method `foo` with no parameters and a `void` return type.
 *
 * **Error Handling**:
 * If the source code contains syntax errors, the `parse` function throws an exception (e.g., `std::runtime_error`),
 * with a detailed message about the location and nature of the error.
 *
 * @param source The raw Mini-Java source code as a string.
 * @return A `Project` object representing the parsed program structure.
 */
Project parse(const std::string &source);

#endif //SIMPLEMINIJAVACOMPILERTOC_PARSER_H