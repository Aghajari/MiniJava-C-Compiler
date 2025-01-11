#ifndef SIMPLEMINIJAVACOMPILERTOC_SCOPE_H
#define SIMPLEMINIJAVACOMPILERTOC_SCOPE_H

#include "identifier.h"
#include "method.h"
#include <map>

/**
 * @class Scope
 * @brief Represents a generic scope in Mini-Java.
 *
 * The `Scope` class acts as the base class for all scoped structures in Mini-Java.
 * It is used as a parent class for more specific scopes, such as classes, methods,
 * and other bounded contexts where variables and fields need to be tracked.
 *
 * In its default form, `Scope` does not provide concrete functionality, but it gives
 * a base type for derived scopes, such as the `Class` scope.
 */
class Scope {
public:
    Scope() = default;
};


/**
 * @class Class
 * @brief Represents a class in Mini-Java.
 *
 * The `Class` class extends `Scope` and models a user-defined class in Mini-Java.
 * It encapsulates:
 * - `name`: The identifier of the class.
 * - `extends`: The optional superclass of the class (if any).
 * - `fields`: A list of fields (instance variables) defined in the class.
 * - `methods`: A list of methods defined in the class.
 *
 * The class also manages mappings of fields and methods for efficient lookup and ensures
 * that fields and methods are unique within the class scope.
 */
class Class : public Scope {
private:
    /// The name of the class as an `Identifier`.
    Identifier name;

    /// The name of the superclass (if any), defaulting to an empty `Identifier` if not extended.
    Identifier extends;

    /// A list of fields (instance variables) defined in the class.
    std::vector<Field> fields;

    /// Maps field names to their positions in the `fields` vector for fast lookups.
    std::map<Identifier, int> fieldsMap;

    /// A list of methods defined in the class.
    std::vector<Method> methods;

    /// Maps method names to their positions in the `methods` vector for fast lookups.
    std::map<Identifier, int> methodsMap;
public:
    /**
     * @brief Constructs a `Class` object.
     * @param name The name of the class.
     * @param extends The name of the superclass the class extends (optional).
     */
    Class(Identifier name, Identifier extends);

    /**
     * @brief Adds a field (instance variable) to the class.
     * @param field The field to add, represented as a `Field` object.
     *
     * After adding the field:
     * - It is appended to the `fields` vector.
     * - It is added to the `fieldsMap` to enable efficient lookups by name.
     * ```
     */
    void addField(Field &field);

    /**
     * @brief Adds a method to the class.
     * @param method The method to add, represented as a `Method` object.
     *
     * After adding the method:
     * - It is appended to the `methods` vector.
     * - It is added to the `methodsMap` to enable efficient lookups by name.
     * ```
     */
    void addMethod(Method &method);

    /**
     * @brief Retrieves all fields of the class.
     * @return A pointer to the vector of fields in the class.
     */
    std::vector<Field> *getFields();

    /**
     * @brief Retrieves all methods of the class.
     * @return A pointer to the vector of methods in the class.
     */
    std::vector<Method> *getMethods();

    /**
     * @brief Checks if the class contains a field with the given name.
     * @param fieldName The name of the field to check for.
     * @return `true` if the field exists, otherwise `false`.
     */
    bool containsField(const Identifier &fieldName);

    /**
     * @brief Checks if the class contains a method with the given name.
     * @param methodName The name of the method to check for.
     * @return `true` if the method exists, otherwise `false`.
     */
    bool containsMethod(const Identifier &methodName);

    /**
     * @brief Retrieves the field with the given name.
     * @param fieldName The name of the field to retrieve.
     * @return A pointer to the `Field` object, or `nullptr` if the field does not exist.
     */
    Field *getField(const Identifier &fieldName);

    /**
     * @brief Retrieves the name of the class.
     * @return The name of the class as an `Identifier`.
     */
    Identifier getName();

    /**
     * @brief Retrieves the name of the superclass the class extends.
     * @return The name of the superclass as an `Identifier`,
     *  or an empty `Identifier` if no superclass is extended.
     */
    Identifier getExtends();

    friend std::ostream &operator<<(std::ostream &strm, const Class &clazz) {
        strm << "Class{" << std::endl << "\tName: " << clazz.name << std::endl
             << "\tExtends: " << clazz.extends << std::endl
             << "\tFields: (" << clazz.fields.size() << ")" << std::endl;
        for (auto &f: clazz.fields) {
            strm << "\t\t" << f << std::endl;
        }
        strm << "\tMethods: (" << clazz.methods.size() << ")" << std::endl;
        for (auto &m: clazz.methods) {
            strm << "\t\t" << m << std::endl;
        }
        return strm << "}";
    }
};

#endif //SIMPLEMINIJAVACOMPILERTOC_SCOPE_H
