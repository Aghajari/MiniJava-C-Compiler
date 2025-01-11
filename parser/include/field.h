#ifndef SIMPLEMINIJAVACOMPILERTOC_FIELD_H
#define SIMPLEMINIJAVACOMPILERTOC_FIELD_H

#include "identifier.h"

/**
 * @enum MiniJavaType
 * @brief Enumerates the types supported in Mini-Java.
 *
 * This enum represents all the types available in Mini-Java:
 * - `MiniJavaType_INT`: Represents the `int` type.
 * - `MiniJavaType_BOOLEAN`: Represents the `boolean` type.
 * - `MiniJavaType_INT_ARRAY`: Represents an array of integers `int[]`.
 * - `MiniJavaType_CLASS`: Represents a user-defined class type.
 * - `MiniJavaType_VOID`: Represents a `void` type (used for methods without return values).
 */
enum MiniJavaType {
    MiniJavaType_INT,
    MiniJavaType_BOOLEAN,
    MiniJavaType_INT_ARRAY,
    MiniJavaType_CLASS,
    MiniJavaType_VOID,
};

/**
 * @class Field
 * @brief Represents a variable or field in the Mini-Java language.
 *
 * The `Field` class models a variable declaration or class field in Mini-Java,
 * containing information about its:
 * - **Type**: The type of the field (from `MiniJavaType`).
 * - **Type Lexeme**: The string representation of the type from the parsed source code.
 * - **Name**: The identifier representing the name of the field.
 *
 * Such fields are used in various contexts like:
 * - Local variable declarations in method bodies.
 * - Fields of user-defined classes.
 * - Parameters of methods.
 *
 * Example:
 * ```java
 * int x;          // Field with type `MiniJavaType_INT` and name `x`.
 * boolean flag;   // Field with type `MiniJavaType_BOOLEAN` and name `flag`.
 * int[] arr;      // Field with type `MiniJavaType_INT_ARRAY` and name `arr`.
 * MyClass obj;    // Field with type `MiniJavaType_CLASS` and name `obj`.
 * ```
 */
class Field {
    /// The MiniJava type of the field (e.g., `int`, `boolean`, `int[]`, or a class type).
    MiniJavaType type;

    /// The string representation of the type as declared in the source code (e.g., `int`, `MyClass`).
    Identifier type_lexeme;

    /// The name of the field (as an `Identifier`).
    Identifier name;
public:
    /**
     * @brief Constructs a new `Field` object.
     * @param type The MiniJava type of the field.
     * @param type_lexeme The string representation of the type (as an `Identifier`).
     * @param name The name of the field (as an `Identifier`).
     *
     * Example Usage:
     * ```cpp
     * Field field(MiniJavaType_INT, Identifier("int"), Identifier("x"));
     * ```
     */
    Field(MiniJavaType type, Identifier type_lexeme, Identifier name);

    /**
     * @brief Retrieves the name of the field.
     * @return The name of the field (as an `Identifier`).
     *
     * Example:
     * ```cpp
     * Field field(MiniJavaType_INT, Identifier("int"), Identifier("x"));
     * Identifier name = field.getName(); // Returns "x".
     * ```
     */
    Identifier getName();

    /**
     * @brief Retrieves the type of the field.
     * @return The type of the field (as a `MiniJavaType` enum value).
     *
     * Example:
     * ```cpp
     * Field field(MiniJavaType_INT, Identifier("int"), Identifier("x"));
     * MiniJavaType type = field.getType(); // Returns MiniJavaType_INT.
     * ```
     */
    MiniJavaType getType();

    /**
     * @brief Retrieves the string representation of the field's type.
     * @return The string form of the type as declared in the source code (as an `Identifier`).
     *
     * This is especially useful for custom class types or displaying the literal type name.
     *
     * Example:
     * ```cpp
     * Field field(MiniJavaType_INT, Identifier("int"), Identifier("x"));
     * Identifier typeStr = field.getTypeLexeme(); // Returns "int".
     * ```
     */
    Identifier getTypeLexeme();

    friend std::ostream &operator<<(std::ostream &strm, const Field &field) {
        return strm << "Field{Name: " << field.name
                    << ", Type: " << field.type_lexeme
                    << "}";
    }
};

#endif //SIMPLEMINIJAVACOMPILERTOC_FIELD_H
