#include "../internal/generator_internal.h"
#include "../internal/generator_tac.h"
#include <map>

/**
 * @brief Determines whether a type requires an additional header file inclusion.
 *
 * Basic types such as `int`, `boolean`, `int[]`, and `void` are excluded, as they do not require custom headers.
 * Custom class types are included based on their names.
 *
 * @param type The type identifier to check.
 * @return `true` if the type requires a header file, otherwise `false`.
 *
 * Example:
 * - For `MyClass`, returns `true` (header `MyClass.h`).
 * - For `int`, returns `false`.
 */
bool should_include_header(const Identifier &type) {
    return type != "int" &&
           type != "boolean" &&
           type != "bool" &&
           type != "int[]" &&
           type != "void";
}

/**
 * @brief Translates Mini-Java type identifiers to corresponding C types.
 *
 * This function maps Mini-Java types like `int`, `boolean`, `void`, and `int[]` to their
 * respective C representations. Additional logic is used for custom class types (pointers).
 *
 * @param type The Mini-Java type identifier (e.g., `int`, `boolean`).
 * @return The equivalent C type as a string.
 *
 * Example:
 * - `int` → `"int "`
 * - `boolean` → `"bool "`
 * - `int[]` → `"__int_array *"`
 * - `MyClass` → `"MyClass *"`
 */
std::string get_type(const Identifier &type) {
    if (type == "boolean") {
        return "bool ";
    } else if (type == "int[]") {
        return "__int_array *";
    } else if (type == "int") {
        return "int ";
    } else if (type == "void") {
        return "void ";
    } else {
        return type + " *";
    }
}

/**
 * @brief Converts Mini-Java types into C-like types based on an enum and identifier.
 *
 * This overload handles specific `MiniJavaType` enums with additional context
 * provided by the type's lexeme.
 *
 * @param type The Mini-Java type enum value.
 * @param lexeme The type lexeme (e.g., `int` or a class name).
 * @return The equivalent C type as a string.
 */
std::string get_type(MiniJavaType type, const Identifier &lexeme) {
    if (type == MiniJavaType::MiniJavaType_CLASS) {
        return lexeme + " *";
    } else if (type == MiniJavaType::MiniJavaType_BOOLEAN) {
        return "bool ";
    } else if (type == MiniJavaType::MiniJavaType_INT) {
        return "int ";
    } else if (type == MiniJavaType::MiniJavaType_INT_ARRAY) {
        return "__int_array *";
    } else if (type == MiniJavaType::MiniJavaType_VOID) {
        return "void ";
    }
    return "void *";
}

std::string get_type(Field *field) {
    return get_type(field->getType(), field->getTypeLexeme());
}

std::string get_type(Method *method) {
    return get_type(method->getReturnType(), method->getReturnTypeLexeme());
}

/**
 * @brief Generates the full signature of a method as a C function.
 *
 * Handles method names, return types, and parameter lists. The `$this` pointer is included in
 * all non-static methods to maintain object context.
 *
 * Example Output:
 * ```c
 * int MyClass_myMethod(void *$this, int param1, bool param2);
 * ```
 *
 * @param method The method definition.
 * @param clazz The class containing the method.
 * @param included A map tracking the included header files for dependency management.
 * @return The complete C function signature for the given method.
 */
std::string get_method_sign(Method *method, Class *clazz, std::map<Identifier, bool> &included) {
    if (method->isMain()) {
        return "int main()";
    }
    included.insert({method->getReturnTypeLexeme(), true});

    std::string sign;
    sign += get_type(method);
    unsigned long paramLen = method->getParams()->size();
    sign += clazz->getName() + "_" + method->getName();
    if (paramLen == 0) {
        sign += "(\n\tvoid *$this\n)";
    } else {
        sign += "(\n\tvoid *$this,\n\t";
    }

    for (int i = 0; i < paramLen; i++) {
        auto param = method->getParams()->at(i);
        included.insert({param.getTypeLexeme(), true});

        sign += get_type(&param);
        sign += param.getName();

        if (i != paramLen - 1) {
            sign += ",\n\t";
        } else {
            sign += "\n)";
        }
    }
    return sign;
}

/**
 * @brief Generates a method signature suitable for use as a function pointer.
 *
 * This is required for implementing inheritance and method overriding.
 * Example Output:
 * ```c
 * int (*$_function_methodName)(void *, int, bool);
 * ```
 *
 * @param method The method definition.
 * @param clazz The class containing the method.
 * @param included The map tracking header files for dependencies.
 * @return The C signature for the method as a function pointer.
 */
std::string get_method_as_param_sign(Method *method, Class *clazz, std::map<Identifier, bool> &included) {
    std::string sign;
    sign += "\t" + get_type(method);
    unsigned long paramLen = method->getParams()->size();
    sign += "(*$_function_" + method->getName() + ")";
    if (paramLen == 0) {
        sign += "(void *)";
    } else {
        sign += "(void *, ";
    }

    for (int i = 0; i < paramLen; i++) {
        auto param = method->getParams()->at(i);
        sign += get_type(&param);

        if (i != paramLen - 1) {
            sign += ", ";
        } else {
            sign += ")";
        }

        if (param.getType() == MiniJavaType::MiniJavaType_CLASS) {
            included.insert({param.getTypeLexeme(), true});
        }
    }
    return sign;
}

/**
 * @brief Writes the fields of a class to its header file.
 *
 * Fields include:
 * - Instance variables (e.g., `int x`).
 * - Function pointers for method dispatch (e.g., method overriding support).
 * - A `super` pointer if the class extends another class.
 *
 * Example Output:
 * ```c
 * struct MyClass {
 *     ParentClass super;
 *     int x;
 *     int y;
 *     int (*$_function_myMethod)(void *, int);
 * };
 * ```
 *
 * @param hSource The header source string being generated.
 * @param clazz The class definition being processed.
 * @param included A map tracking header files for field dependencies.
 */
void write_fields(std::string &hSource, Class *clazz, std::map<Identifier, bool> &included) {
    if (!clazz->getExtends().empty()) {
        hSource += "\t" + clazz->getExtends() + " super;\n";
        included.insert({clazz->getExtends(), true});
    }

    for (auto &field: *clazz->getFields()) {
        if (field.getTypeLexeme() == clazz->getName()) {
            hSource += "\tstruct " + clazz->getName() + " *" + field.getName() + ";\n";
        } else {
            hSource += "\t" + get_type(&field) + field.getName() + ";\n";
        }

        if (field.getType() == MiniJavaType::MiniJavaType_CLASS) {
            included.insert({field.getTypeLexeme(), true});
        }
    }

    hSource += "\n";

    for (auto &method: *clazz->getMethods()) {
        if (method.isMain()) continue;
        hSource += get_method_as_param_sign(&method, clazz, included) + ";\n";
    }
}

/**
 * @brief Generates the header file for a given class, including its fields and method signatures.
 *
 * Example Header Output:
 * ```c
 * #ifndef COMPILED_MyClass_H
 * #define COMPILED_MyClass_H
 *
 * #include <stdbool.h>
 * #include "__int_array.h"
 *
 * struct MyClass {
 *     int x;
 *     ParentClass super;
 *     int (*$_function_myMethod)(void *, int);
 * };
 *
 * typedef struct MyClass MyClass;
 *
 * MyClass *$_new_MyClass();
 * int MyClass_myMethod(void *$this, int param);
 *
 * #endif //COMPILED_MyClass_H
 * ```
 *
 * @param project The project being processed.
 * @param clazz The class for which the header file is generated.
 * @param included A map tracking global included dependency headers.
 */
void generate_class_header(Project *project, Class *clazz, std::map<Identifier, bool> &included) {
    std::string hKey = "COMPILED_" + clazz->getName() + "_H";
    std::string hSource = "#ifndef " + hKey + "\n#define " + hKey + "\n\n";

    hSource += "#include <stdbool.h>\n";
    hSource += "#include \"__int_array.h\"\n";
    unsigned long include_start = hSource.length();

    hSource += "struct " + clazz->getName() + " {\n";
    write_fields(hSource, clazz, included);
    hSource += "};\n\n";

    hSource += "typedef struct " + clazz->getName() + " " + clazz->getName() + ";\n\n";

    for (auto &method: *clazz->getMethods()) {
        if (method.isMain()) continue;
        hSource += get_method_sign(&method, clazz, included) + ";\n\n";
    }

    hSource += clazz->getName() + " *$_new_" + clazz->getName() + "();\n\n";

    hSource += "#endif //" + hKey + "\n";

    if (!included.empty()) {
        std::string include_headers;
        for (auto &inc: included) {
            if (inc.first == clazz->getName() || !should_include_header(inc.first)) {
                continue;
            }

            include_headers += "#include \"" + inc.first + ".h\"\n";
        }
        include_headers += "\n";
        hSource.insert(include_start, include_headers);
    } else {
        hSource.insert(include_start, "\n");
    }
    write_file(clazz->getName() + ".h", hSource);
}

std::string get_method_reference_name(Project *project, Class *clazz, const Identifier &method) {
    if (clazz->containsMethod(method)) {
        return clazz->getName() + "_" + method;
    } else if (!clazz->getExtends().empty()) {
        return get_method_reference_name(project, project->getClassByName(clazz->getExtends()), method);
    }
    return "";
}

std::string get_field_default_value(Field *field) {
    if (field->getType() == MiniJavaType::MiniJavaType_INT) {
        return "0";
    } else if (field->getType() == MiniJavaType::MiniJavaType_BOOLEAN) {
        return "false";
    } else {
        return "NULL";
    }
}

void generate_new_object_function_initialization_source(
        std::string &source,
        const std::string &starter,
        Project *project,
        Class *clazz,
        Class *root
) {
    for (auto &method: *clazz->getMethods()) {
        if (method.isMain()) continue;
        source += "\t" + starter + "$_function_" + method.getName() + " = " +
                  get_method_reference_name(project, root, method.getName()) + ";\n";
    }
    if (!clazz->getExtends().empty()) {
        Class *c = project->getClassByName(clazz->getExtends());
        generate_new_object_function_initialization_source(source, starter + "super.", project, c, root);
    }
}

void generate_new_object_fields_initialization_source(
        std::string &source,
        const std::string &starter,
        Project *project,
        Class *clazz
) {
    for (auto &field: *clazz->getFields()) {
        source += "\t" + starter + field.getName() + " = " +
                  get_field_default_value(&field) + ";\n";
    }
    if (!clazz->getExtends().empty()) {
        Class *c = project->getClassByName(clazz->getExtends());
        generate_new_object_fields_initialization_source(source, starter + "super.", project, c);
    }
}

/**
 * @brief Generates the object instantiation function for a given class.
 *
 * This function implements the equivalent of the `new` keyword, allocating memory for
 * the class, initializing fields, and setting up function pointers for methods.
 *
 * Example Output:
 * ```c
 * MyClass *$_new_MyClass() {
 *     MyClass *self = (MyClass *) malloc(sizeof(MyClass));
 *     self->x = 0;
 *     self->$_function_myMethod = MyClass_myMethod;
 *     return self;
 * }
 * ```
 *
 * @param source The generated C source string to append to.
 * @param project The parsed project.
 * @param clazz The class being processed.
 */
void generate_new_object_source(std::string &source, Project *project, Class *clazz) {
    source += clazz->getName() + " *$_new_" + clazz->getName() + "() {\n";
    source += "\t" + clazz->getName() + " *self = (" + clazz->getName() +
              " *) malloc(sizeof(" + clazz->getName() + "));\n\n";
    generate_new_object_fields_initialization_source(source, "self->", project, clazz);
    source += "\n";
    generate_new_object_function_initialization_source(source, "self->", project, clazz, clazz);
    source += "\treturn self;\n";
    source += "}\n\n";
}

/**
 * @brief Generates the full source file for a given class, including its methods and constructor.
 *
 * This function generates a `.c` file that includes:
 * - The `new` function for object instantiation.
 * - Method implementation for the class.
 * - Inclusion of necessary headers.
 *
 * @param project The project containing the source.
 * @param clazz The class for which the source file is generated.
 * @param included A map tracking dependencies to include.
 */
void generate_class_source(Project *project, Class *clazz, std::map<Identifier, bool> &included) {
    std::string source = "#include <stdlib.h>\n"
                         "#include <stdio.h>\n"
                         "#include \"" + clazz->getName() + ".h\"\n";
    unsigned long include_start = source.size();
    source += "\n";

    generate_new_object_source(source, project, clazz);

    std::map<Identifier, bool> typesUsed;

    for (auto &method: *clazz->getMethods()) {
        source += get_method_sign(&method, clazz, included) + " {\n";
        if (!method.isMain()) {
            source += "\t" + clazz->getName() + " *super = (" + clazz->getName() + " *) $this;\n\n";
        }

        auto t = ThreeAddressCodeGenerator{};
        t.types = &typesUsed;
        t.project = project;
        t.clazz = clazz;
        t.openBlock();
        if (!method.isMain()) {
            for (auto &param: *method.getParams()) {
                t.addVariable(param.getName(), param.getTypeLexeme());
            }
        }
        generate(t, method.getCodeBlock());
        t.closeBlock();
        source += t.code;
        source += "}\n\n";
    }

    if (!typesUsed.empty()) {
        std::string include_headers;
        for (auto &inc: typesUsed) {
            if (inc.first == clazz->getName() ||
                included.contains(inc.first) ||
                !should_include_header(inc.first)) {
                continue;
            }

            include_headers += "#include \"" + inc.first + ".h\"\n";
        }
        include_headers += "\n";
        source.insert(include_start, include_headers);
    }

    write_file(clazz->getName() + ".c", source);
}