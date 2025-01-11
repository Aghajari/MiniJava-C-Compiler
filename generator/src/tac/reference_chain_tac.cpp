#include "../../internal/generator_tac.h"
#include "../../../lexer/include/token_matcher.h"

/**
 * @brief Generates TAC for object creation expressions.
 *
 * Handles both class instantiation and array creation:
 * - Regular objects: `new ClassName()`
 * - Integer arrays: `new int[size]`
 *
 * Example:
 * ```java
 * new MyClass()     // Creates object
 * new int[24]       // Creates integer array
 * ```
 * Generated TAC:
 * ```java
 * $_new_MyClass()
 * $_new___int_array(24)
 * ```
 *
 * @param gen TAC generator context
 * @param node NewObject AST node
 * @return Temporary variable containing the new object reference
 */
std::string generateNewObject(ThreeAddressCodeGenerator &gen, NewObject *node) {
    std::string tmp = gen.tempGen.newTemp();
    if (node->classType.lexeme == "int" && node->arraySize) {
        std::string value = generate(gen, &node->arraySize);
        gen.emit("__int_array *" + tmp + " = $_new___int_array(" + value + ")");
    } else {
        gen.newObject(node->classType.lexeme);
        gen.emit(node->classType.lexeme + " *" + tmp + " = $_new_" + node->classType.lexeme + "()");
    }
    return tmp;
}

/**
 * @brief Generates TAC for array access operations.
 *
 * Handles array indexing operations, generating appropriate pointer arithmetic
 * and bounds checking if required.
 *
 * Example:
 * ```java
 * array[2 + 4]
 * ```
 * Generated TAC:
 * ```java
 * int tmp = 2 + 4;
 * array->data[tmp]
 * ```
 *
 * @param gen TAC generator context
 * @param node ArrayCall AST node
 * @param caller The array reference expression
 * @return Generated array access expression
 */
std::string generateArrayCall(
        ThreeAddressCodeGenerator &gen,
        ArrayCall *node,
        const std::string &caller
) {
    std::string argTemp = generate(gen, node->bracket.get());
    return caller + node->arrayName + "->data[" + argTemp + "]";
}

/**
 * @brief Generates TAC for method calls.
 *
 * Handles method invocation including:
 * - Parameter passing
 * - Return value handling
 * - Virtual method dispatch
 *
 * Example:
 * ```java
 * obj.method(arg1, arg2)
 * ```
 * Generated TAC:
 * ```java
 * obj->$_function_method(arg1, arg2)
 * ```
 *
 * @param gen TAC generator context
 * @param node MethodCall AST node
 * @param climbed Whether inheritance hierarchy was climbed
 * @param caller The object reference
 * @param caller_org Original caller for super calls
 * @return Temporary variable containing the return value (if any)
 */
std::string generateMethodCall(
        ThreeAddressCodeGenerator &gen,
        MethodCall *node,
        bool climbed,
        const std::string &caller,
        const std::string &caller_org
) {
    std::vector<std::string> argumentTemps;
    std::string callerTmp;
    std::string callerTmpArg;

    if (isIdentifier(caller) || climbed) {
        callerTmp = caller;
        callerTmpArg = caller_org;
    } else {
        callerTmpArg = callerTmp = gen.tempGen.newTemp();
        gen.emit(get_type(node->callerType) + callerTmp + " = " + caller);
    }
    argumentTemps.push_back(callerTmpArg);

    for (const auto &arg: node->arguments) {
        std::string argTemp = generate(gen, arg.get());
        argumentTemps.push_back(argTemp);
    }

    std::string argumentList;
    for (size_t i = 0; i < argumentTemps.size(); ++i) {
        if (i > 0) argumentList += ", ";
        argumentList += argumentTemps[i];
    }

    std::string resultTemp;
    std::string method = callerTmp + (climbed ? "." : "->") + "$_function_" + node->methodName;

    if (node->type != "void") {
        resultTemp = gen.tempGen.newTemp();
        gen.emit(get_type(node->type) + resultTemp + " = " + method + "(" + argumentList + ")");
        return resultTemp;
    } else {
        gen.emit(method + "(" + argumentList + ")");
        return "";
    }
}

/**
 * @brief Handles System.out.print operations.
 *
 * Special case handler for System.out.print/println/printf operations,
 * converting them to appropriate C printf calls.
 *
 * Example:
 * ```java
 * System.out.println(24)
 * ```
 * Generated TAC:
 * ```java
 * printf("%d\\n", 24);
 * ```
 *
 * @param gen TAC generator context
 * @param reference The reference chain
 * @return true if handled as print operation, false otherwise
 */
bool generatePrint(
        ThreeAddressCodeGenerator &gen,
        ReferenceChain *reference
) {
    if (reference->chain.size() != 3 ||
        reference->chain[0].first.lexeme != "System" ||
        reference->chain[1].first.lexeme != "out" ||
        !reference->chain[2].second ||
        reference->chain[2].second->getType() != ASTType::AST_MethodCall ||
        (
                reference->chain[2].first.lexeme != "print" &&
                reference->chain[2].first.lexeme != "printf" &&
                reference->chain[2].first.lexeme != "println"
        )) {
        return false;
    }
    MethodCall *mc = ((MethodCall *) reference->chain[2].second.get());
    if (mc->arguments.size() != 1 || mc->arguments[0]->type != "int") {
        return false;
    }
    std::string format = (reference->chain[2].first.lexeme == "println") ? "%d\\n" : "%d";
    std::string value = generate(gen, &mc->arguments[0]);
    gen.emit("printf(\"" + format + "\", " + value + ")");
    return true;
}

/**
 * @brief Generates Three-Address Code (TAC) for complex reference chains.
 *
 * This function handles the generation of TAC for complex reference expressions including:
 * - Method calls (e.g., `obj.method()`)
 * - Field access (e.g., `obj.field`)
 * - Array access (e.g., `arr[index]`)
 * - Object creation (e.g., `new Class()`)
 * - System.out.print operations
 * - Chained operations (e.g., `obj.field.method().array[index]`)
 *
 * The function maintains type information throughout the chain and handles:
 * - Inheritance (climbing the class hierarchy)
 * - Pointer vs. dot notation in generated code
 * - Special cases like `this` and `System.out.println`
 *
 * Example Mini-Java:
 * ```java
 * obj.field.method().array[index].length
 * new MyClass().method()
 * System.out.println(24)
 * ```
 *
 * This function specifically handles the complexity of accessing fields through multiple
 * levels of inheritance by generating the correct chain of `super` references.
 *
 * Key Inheritance Rules:
 * 1. First 'super->' uses arrow (->) operator because it's a pointer to the struct
 * 2. Subsequent 'super' accesses use dot (.) operator because they're struct members
 * 3. The number of 'super' references matches the inheritance depth
 *
 * The function determines the number of 'super' references needed by:
 * 1. Looking up the field in the current scope (local scope)
 * 2. Looking up the field in the current class
 * 3. If not found, climbing the inheritance tree
 * 4. Counting the number of steps needed to reach the class containing the field
 *
 * Example Inheritance Chain:
 * ```java
 * class A {
 *     int[] arr;
 * }
 * class B extends A { }
 * class C extends B {
 *     void test() {
 *         arr[2] = 4;  // Accessing arr from grandparent class A
 *     }
 * }
 * ```
 * Generated TAC:
 * ```c
 * super->super.super.arr->data[2]
 * // Where:
 * //   First super->       : Accesses C's struct pointer
 * //   super.super         : Climbs through B to reach A
 * //   .arr                : Accesses the array field
 * //   ->data[2]          : Accesses array element
 * ```
 * Generated Structures:
 * ```c
 * struct A {
 *     __int_array* arr;
 * };
 * struct B {
 *     struct A super;  // Inherits from A
 * };
 * struct C {
 *     struct B super;  // Inherits from B
 * };
 *
 * void C_test(void* $this) {
 *     struct C *super = (struct C*) $this;
 *     super->super.super.arr->data[2] = 4;
 * }
 * ```
 *
 * @param gen The TAC generator context
 * @param reference The reference chain to process
 * @param getMethod Whether to include the final method in chain processing
 * @param currentType Output parameter tracking the current type being processed
 * @return Generated TAC code as string
 */
std::string generate(
        ThreeAddressCodeGenerator &gen,
        ReferenceChain *reference,
        bool getMethod,
        std::string &currentType
) {
    if (generatePrint(gen, reference)) {
        return "";
    }

    SymbolTable *currentTable = nullptr;
    std::string output;
    bool isPointer = true;

    for (size_t i = 0; i < reference->chain.size() - (getMethod ? 0 : 1); ++i) {
        const auto &entry = reference->chain[i];

        if (i == 0) {
            if (entry.first.lexeme == "this") {
                output += "super";
                currentType = gen.clazz->getName();
                currentTable = SymbolTable::getClassSymbolTable(currentType);
                continue;
            }

            if (entry.second) {
                if (entry.second->getType() == ASTType::AST_MethodCall) {
                    output += "super";
                    currentType = gen.clazz->getName();
                } else if (entry.second->getType() == ASTType::AST_ArrayCall) {
                    std::string localType = gen.lookup(entry.first.lexeme);
                    if (localType.empty()) {
                        int nestedCount = gen.lookupClassNestedCount(entry.first.lexeme, currentType);
                        for (int j = 0; j < nestedCount; ++j) {
                            if (j == 0) {
                                output += "super->";
                            } else {
                                output += "super.";
                            }
                        }
                    } else {
                        currentType = localType;
                    }
                    ArrayCall *ac = ((ArrayCall *) entry.second.get());
                    output = generateArrayCall(gen, ac, output);
                    continue;
                } else if (entry.second->getType() == ASTType::AST_NewObject) {
                    NewObject *no = ((NewObject *) entry.second.get());
                    output = generateNewObject(gen, no);
                    currentType = no->type;
                    if (currentType != "int[]") {
                        currentTable = SymbolTable::getClassSymbolTable(currentType);
                        if (!currentTable) {
                            error("Type '" + currentType + "' is not a valid class.");
                        }
                    }
                    continue;
                }
            } else {
                std::string localType = gen.lookup(entry.first.lexeme);
                if (localType.empty()) {
                    int nestedCount = gen.lookupClassNestedCount(entry.first.lexeme, currentType);
                    for (int j = 0; j < nestedCount; ++j) {
                        if (j == 0) {
                            output += "super->";
                        } else {
                            output += "super.";
                        }
                    }
                    output += entry.first.lexeme;
                } else {
                    output = entry.first.lexeme;
                    currentType = localType;
                }
            }

            if (currentType == "int" || currentType == "int[]" || currentType == "boolean") {
                continue;
            } else {
                currentTable = SymbolTable::getClassSymbolTable(currentType);
                if (!currentTable) {
                    error("Type '" + currentType + "' is not a valid class.");
                }
                if (!entry.second) {
                    continue;
                }
            }
        }

        const std::string &fieldOrMethod = entry.first.lexeme;

        if (currentType == "int[]" && fieldOrMethod == "length" && !entry.second) {
            currentType = "int";
            output += "->length";
            continue;
        }

        std::string beforeClimb = output;
        bool climbed = false;
        Symbol *field = nullptr;
        while (!field && currentTable) {
            field = currentTable->find(fieldOrMethod);
            if (!field) {
                // Climb the hierarchy (handle `super`)
                currentTable = currentTable->getParent();
                if (currentTable) {
                    output += isPointer ? "->" : ".";
                    output += "super";
                    currentType = currentTable->getClassName();
                    isPointer = false;
                    climbed = true;
                }
            }
        }

        if (!field) {
            error("Field '" + fieldOrMethod + "' not found in class hierarchy.");
        }

        if (!entry.second) {
            output += isPointer ? "->" : ".";
            output += fieldOrMethod;
            isPointer = true;

            currentType = field->type;
            currentTable = SymbolTable::getClassSymbolTable(currentType);
        } else {
            auto &caller = entry.second;
            if (caller->getType() == ASTType::AST_MethodCall) {
                MethodCall *mc = ((MethodCall *) caller.get());
                output = generateMethodCall(gen, mc, climbed, output, beforeClimb);
            } else if (caller->getType() == ASTType::AST_ArrayCall) {
                output += isPointer ? "->" : ".";
                ArrayCall *ac = ((ArrayCall *) caller.get());
                output = generateArrayCall(gen, ac, output);
            } else {
                output = generate(gen, caller.get());
            }
            currentType = caller->type;
            isPointer = true;
            currentTable = SymbolTable::getClassSymbolTable(currentType);
        }
    }

    return output;
}

std::string generate(ThreeAddressCodeGenerator &gen, ReferenceChain *chain) {
    std::string currentType;
    return generate(gen, chain, true, currentType);
}