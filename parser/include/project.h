#ifndef SIMPLEMINIJAVACOMPILERTOC_PROJECT_H
#define SIMPLEMINIJAVACOMPILERTOC_PROJECT_H

#include "scope.h"
#include <map>

/**
 * @class Project
 * @brief Represents the overall Mini-Java project being compiled, containing all the classes and their relationships.
 *
 * The `Project` class serves as an entry point for managing all the classes in a Mini-Java program. It:
 * - Stores the list of all classes in the program.
 * - Provides utilities for adding classes, resolving class dependencies, and retrieving classes by name.
 * - Ensures that classes are processed in the correct order based on their inheritance hierarchy (via `getTopologicalSort`).
 *
 * Responsibilities:
 * - Maintain a list of all `Class` objects in the project.
 * - Resolve class dependencies to ensure correct semantic analysis order.
 * - Support lookups for individual classes by name.
 */
class Project {
private:
    /// A list of all classes in the program.
    std::vector<Class> classes;

    /// A map for faster lookup of class indices by name (`Identifier`).
    std::map<Identifier, int> classesMap;
public:
    /**
     * @brief Adds a new class to the project.
     * @param clazz A reference to the `Class` object to add.
     *
     * Adds the given class to the internal list of `classes`
     * and updates the `classesMap` for efficient lookup by name.
     * ```
     */
    void addClass(Class &clazz);

    /**
     * @brief Retrieves a pointer to the list of all classes in the project.
     * @return A pointer to the vector of `Class` objects.
     */
    std::vector<Class> *getClasses();

    /**
     * @brief Produces a topological ordering of classes based on their inheritance relationships.
     *
     * The result ensures that if `Class A` extends `Class B`, then `Class B` appears before `Class A`.
     * This ordering is critical before performing semantic analysis to resolve inheritance hierarchies and dependencies.
     *
     * **Why Topological Sorting is Important**:
     * Classes must be processed in a specific order during semantic analysis:
     * - If a class `A` extends another class `B`, `B` must be processed **before** `A` during semantic checks.
     * - The `getTopologicalSort` method ensures that all such inheritance dependencies are resolved properly.
     *
     * Example Mini-Java Code:
     * ```java
     * class A {}
     * class B extends A {}
     * class C extends B {}
     * ```
     * After sorting, the classes would appear in the order: `A -> B -> C`.
     *
     * @return A vector of `Identifier` objects representing class names in topological order.
     *
     * Throws:
     * - if a dependency could not found (e.g., `A extends B` and `B does not exists`).
     * - if a cyclic inheritance relationship is detected (e.g., `A extends B` and `B extends A`).
     */
    std::vector<Identifier> getTopologicalSort();

    /**
     * @brief Retrieves a pointer to a class by its name.
     * @param name The name of the class to retrieve.
     * @return A pointer to the `Class` object if found, otherwise `nullptr`.
     */
    Class *getClassByName(const Identifier &name);

    /**
     * @brief Checks if the project contains a class with the given name.
     * @param className The name of the class to check.
     * @return `true` if the class exists, otherwise `false`.
     */
    bool containsClass(const Identifier &className);

    friend std::ostream &operator<<(std::ostream &strm, const Project &project) {
        for (auto &cz: project.classes) {
            strm << cz << std::endl;
        }
        return strm;
    }
};

#endif //SIMPLEMINIJAVACOMPILERTOC_PROJECT_H
