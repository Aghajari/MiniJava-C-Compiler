#include "../../internal/parser_internal.h"

/**
 * @brief Parses the body (scope) of a class.
 *
 * This function processes the fields and methods declared within a class. It ensures that:
 * - Fields and methods are unique within the class.
 * - Fields are validated and added to the class.
 * - Methods are parsed, including their parameters and body, and then added to the class.
 *
 * The class body is terminated by a closing brace `}`.
 *
 * Example Input:
 * ```java
 * class MyClass {
 *     int x;
 *     void foo() {
 *         x = 42;
 *     }
 * }
 * ```
 * Example Behavior:
 * - Parses the field `int x;` and adds it to the class.
 * - Parses the method `void foo()` including its body and adds it to the class.
 *
 * Throws:
 * - Errors if duplicate fields or methods are found.
 * - Errors if unrecognized tokens are encountered.
 *
 * @param project The `Project` object containing the parsed program.
 * @param clazz The `Class` object to populate with fields and methods.
 * @param streamer The `TokenStreamer` used to read tokens sequentially.
 */
void parseClassScope(Project &project, Class &clazz, TokenStreamer &streamer) {
    while (true) {
        Token *nextToken = streamer.read();
        if (nextToken == nullptr) {
            error("Failed to parse class body of " + clazz.getName() + ", Expected class body but got null");
        } else if (nextToken->lexeme == "}") {
            return; // end of class
        }
        streamer.unread();

        ParamSignature sign = {};
        parseFieldOrMethod(&sign, project, streamer);
        if (sign.isField) {
            if (clazz.containsField(sign.name)) {
                error("Field " + sign.name + " already exists in " + clazz.getName(), nextToken);
            }
            Field field = Field(sign.type, sign.type_lexeme, sign.name);
            clazz.addField(field);
        } else {
            if (clazz.containsMethod(sign.name)) {
                error("Method " + sign.name + " already exists in " + clazz.getName(), nextToken);
            }
            Method method = Method(sign.type, sign.type_lexeme, sign.name, sign.isStatic);
            parseMethodParams(&method, project, streamer);
            parseMethodBody(&method, project, streamer);
            clazz.addMethod(method);
        }
    }
}

/**
 * @brief Parses a class declaration, including its name, optional superclass, and body.
 *
 * This function identifies and processes a class definition. It:
 * - Validates the `class` keyword.
 * - Parses the class name.
 * - Handles the optional `extends` keyword and validates the superclass name.
 * - Ensures the class does not extend itself.
 * - Parses the class body (fields and methods) using `parseClassScope`.
 *
 * Example Input:
 * ```java
 * class MyClass extends ParentClass {
 *     int x;
 *     void foo() {
 *         x = 42;
 *     }
 * }
 * ```
 *
 * Example Behavior:
 * - Parses the class `MyClass` with superclass `ParentClass`.
 * - Processes its fields and methods through `parseClassScope`.
 *
 * Validations:
 * - Ensures the class name is unique.
 * - Ensures the class body opens with `{`.
 *
 * Returns:
 * - `true` if the class is successfully parsed.
 * - `false` if the `TokenStreamer` reaches the end of the source code without a `class` definition.
 *
 * Throws:
 * - Errors for invalid syntax or semantics (e.g., duplicate classes, missing braces, invalid class names).
 *
 * @param project The `Project` object containing the parsed program.
 * @param streamer The `TokenStreamer` used to read tokens sequentially.
 * @return `true` if a class is successfully parsed, otherwise `false`.
 */
bool parseClass(Project &project, TokenStreamer &streamer) {
    if (streamer.readUntil("class") == nullptr) {
        return false;
    }

    Token *className = streamer.read();
    if (className == nullptr || className->type != TokenType::IDENTIFIER) {
        error("Failed to parse class name, Expected identifier", className);
    }
    if (project.containsClass(className->lexeme)) {
        error("Class " + className->lexeme + " already exists!", className);
    }

    Token *extends = streamer.read();
    bool scopeStarted = false;

    if (extends != nullptr && extends->lexeme == "extends") {
        extends = streamer.read();
        if (extends == nullptr || extends->type != TokenType::IDENTIFIER) {
            error("Failed to parse class " + className->lexeme + " extends, Expected identifier", extends);
        }
        if (extends->lexeme == className->lexeme) {
            error("Failed to parse class, class can not extend itself", extends);
        }

        Token *scopeStart = streamer.read();
        if (scopeStart != nullptr && scopeStart->lexeme == "{") {
            scopeStarted = true;
        } else {
            extends = scopeStart; // only for reporting error
        }
    } else if (extends != nullptr && extends->lexeme == "{") {
        extends = nullptr;
        scopeStarted = true;
    }

    if (!scopeStarted) {
        error("Failed to parse class " + className->lexeme + ", Expected {", extends);
    }

    Class clazz = Class(className->lexeme, extends == nullptr ? "" : extends->lexeme);
    parseClassScope(project, clazz, streamer);
    project.addClass(clazz);
    return true;
}