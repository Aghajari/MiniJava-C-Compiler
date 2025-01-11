#ifndef SIMPLEMINIJAVACOMPILERTOC_METHOD_H
#define SIMPLEMINIJAVACOMPILERTOC_METHOD_H

#include "field.h"
#include "ast.h"

/**
 * @class Method
 * @brief Represents a method in Mini-Java programs.
 *
 * A `Method` encapsulates the essential components of a method in Mini-Java, including:
 * - **Return Type**: Specifies the type of value the method returns (or `void` for no return value).
 * - **Name**: The name identifier for the method.
 * - **Parameters**: A list of `Field` objects representing the method's parameters, including their names and types.
 * - **Body**: The method's implementation, represented as a `CodeBlock`.
 * - **Main Method Indicator**: A flag indicating if the method is the special `main` entry point in Mini-Java.
 * ```
 */
class Method {
    /// The return type of the method (e.g., `int`, `boolean`, or `void` for no return).
    MiniJavaType type;

    /// The string representation of the return type from the source code (e.g., `int`, `MyClass`).
    Identifier type_lexeme;

    /// The name identifier of the method.
    Identifier name;

    /// A list of parameter fields (name and type) of the method.
    std::vector<Field> params;

    /// The body of the method, represented as a `CodeBlock`.
    CodeBlock code;

    /// A flag to indicate if this method is the `main` method of the program.
    bool main = false;
public:
    Method(const Method&) = delete;
    Method(Method&&) = default;
    Method(MiniJavaType type, Identifier type_lexeme, Identifier name, bool main);

    /**
     * @brief Adds a parameter to the method's parameter list.
     * @param param The parameter to add (as a `Field` object).
     */
    void addParam(const Field &param);

    /**
     * @brief Checks if a parameter with the given name exists in the method.
     * @param paramName The name of the parameter to check.
     * @return `true` if a parameter with the given name exists, otherwise `false`.
     */
    bool containsParam(const Identifier &paramName);

    /**
     * @brief Retrieves the list of parameters of the method.
     * @return A pointer to the vector of `Field` objects representing the parameters.
     */
    std::vector<Field> *getParams();

    /**
     * @brief Retrieves the name of the method.
     * @return The name of the method (as an `Identifier` object).
     */
    Identifier getName();

    /**
     * @brief Retrieves the return type of the method.
     * @return The return type of the method (as a `MiniJavaType` enumeration value).
     */
    MiniJavaType getReturnType();

    /**
     * @brief Retrieves the string representation of the return type.
     * @return The string representation of the return type (as an `Identifier`).
     */
    Identifier getReturnTypeLexeme();

    /**
     * @brief Retrieves the `CodeBlock` representing the method's body.
     * @return A pointer to the method's `CodeBlock`.
     */
    CodeBlock *getCodeBlock();

    /**
     * @brief Checks if this is the `main` method.
     * @return `true` if this is the `main` method, otherwise `false`.
     */
    bool isMain();

    friend std::ostream &operator<<(std::ostream &strm, const Method &method) {
        strm << "Method{Name: " << method.name
             << ", Type: " << method.type_lexeme
             << " Params: (";
        unsigned long s = method.params.size();
        for (int i = 0; i < s; i++) {
            strm << method.params[i];
            if (i != s - 1) {
                strm << ", ";
            }
        }
        strm << ")} {" << std::endl;
        method.code.print(strm, 3);
        return strm << "\t\t}";
    }
};

#endif //SIMPLEMINIJAVACOMPILERTOC_METHOD_H
