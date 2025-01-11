#ifndef SIMPLEMINIJAVACOMPILERTOC_GENERATOR_TAC_H
#define SIMPLEMINIJAVACOMPILERTOC_GENERATOR_TAC_H

#include "generator_internal.h"
#include <stack>

/**
 * @struct TempVariableGenerator
 * @brief Generates unique temporary variable names for TAC.
 *
 * Example:
 * ```cpp
 * $_t_0 = a + b;
 * $_t_1 = $_t_0 * c;
 * ```
 */
struct TempVariableGenerator {
    int counter = 0;

    /**
     * @brief Generates a new unique temporary variable name.
     * @return String in format "$_t_X" where X is an incrementing number.
     */
    std::string newTemp() {
        return "$_t_" + std::to_string(counter++);
    }
};

/**
 * @struct LabelGenerator
 * @brief Generates unique labels for control flow in TAC.
 *
 * Example:
 * ```cpp
 * if_0:
 * while_1:
 * end_2:
 * ```
 */
struct LabelGenerator {
    int counter = 0;

    /**
     * @brief Generates a new unique label with given prefix.
     * @param prefix The prefix for the label (e.g., "if", "while", "end").
     * @return String in format "prefix_X" where X is an incrementing number.
     */
    std::string newLabel(const std::string &prefix) {
        return prefix + "_" + std::to_string(counter++);
    }
};

/**
 * @struct ThreeAddressCodeGenerator
 * @brief Main TAC generator that manages code generation, scoping, and control flow.
 *
 * This structure maintains:
 * - Temporary variable generation
 * - Label generation
 * - Scope management
 * - Variable tracking
 * - Control flow (break/continue) support
 *
 * Example Usage:
 * ```java
 * // Mini-Java code
 * while (x < 10) {
 *     if (x == 5) break;
 *     x = x + 1;
 * }
 * ```
 *
 * Generated TAC:
 * ```c
 * while_0:
 *     $_t_0 = x < 10;
 *     if (!$_t_0) goto end_0;
 *     $_t_1 = x == 5;
 *     if (!$_t_1) goto if_end_0;
 *     goto end_0;
 * if_end_0:
 *     x = x + 1; *     goto while_0;
 * end_0:
 * ```
 */
struct ThreeAddressCodeGenerator {
    /// Generates temporary variables
    TempVariableGenerator tempGen;
    /// Generates unique labels
    LabelGenerator labelGen;
    /// Current project context
    Project *project;
    /// Current class context
    Class *clazz;
    /// Generated TAC code
    std::string code;
    /// Tracks used types (may need to include header files later)
    std::map<Identifier, bool> *types;
    /// Current block depth
    int depth = -1;
    /// Controls block creation
    bool blockFreeze = false;
    /// Scope variable tracking
    std::vector<std::unordered_map<Identifier, Identifier>> localVariables;
    /// Break/continue labels (pair of <start, end> labels)
    std::stack<std::pair<std::string, std::string>> labelStack;

    /**
     * @brief Opens a new scope block.
     *
     * Creates a new scope for local variables and increases indentation.
     * Example:
     * ```c
     * {
     *     // New scope
     * ```
     */
    void openBlock() {
        if (blockFreeze) return;
        if (depth >= 1) {
            emit("{");
        }
        depth++;
        localVariables.emplace_back();
    }

    /**
     * @brief Closes current scope block.
     *
     * Removes local variables from current scope and decreases indentation.
     * Example:
     * ```c
     * }  // End scope
     * ```
     */
    void closeBlock() {
        if (blockFreeze) return;
        depth--;
        if (depth >= 1) {
            emit("}");
        }
        localVariables.pop_back();
    }

    /**
     * @brief Controls block creation during initialization phases.
     * @param freeze If true, prevents new block creation.
     */
    void freeze(bool freeze) {
        blockFreeze = freeze;
    }

    /**
     * @brief Pushes labels for break/continue statements.
     * @param start Label for continue (loop start)
     * @param end Label for break (loop end)
     */
    void pushLabel(const std::string &start, const std::string &end) {
        labelStack.push(std::pair(start, end));
    }

    /**
     * @brief Removes top label pair from stack.
     */
    void popLabel() {
        labelStack.pop();
    }

    /**
     * @brief Generates break statement.
     * Emits: goto end_label
     */
    void breakNow() {
        if (labelStack.empty()) {
            error("Failed to call break, break statement must be called inside a loop");
        }

        emit("goto " + labelStack.top().second);
    }

    /**
     * @brief Generates continue statement.
     * Emits: goto start_label
     */
    void continueNow() {
        if (labelStack.empty()) {
            error("Failed to call continue, continue statement must be called inside a loop");
        }

        emit("goto " + labelStack.top().first);
    }

    /**
     * @brief Emits a line of TAC code with proper indentation.
     * @param line The code line to emit
     */
    void emit(const std::string &line) {
        code += std::string(depth, '\t') + line + (line.length() > 1 ? ";\n" : "\n");
    }

    /**
     * @brief Adds empty line for readability.
     */
    void newLine() {
        code += '\n';
    }

    /**
     * @brief Emits a label in the TAC code.
     * @param label The label to emit
     */
    void emitLabel(const std::string &label) {
        code += std::string(depth, '\t') + label + ":;\n";
    }

    /**
     * @brief Adds variable to current scope.
     * @param name Variable name
     * @param type Variable type
     */
    void addVariable(const Identifier &name, const Identifier &type) {
        localVariables.front().insert({name, type});
        types->insert({type, true});
    }

    /**
     * @brief Records object creation of given type.
     * @param type The type being instantiated
     */
    void newObject(const Identifier &type) const {
        types->insert({type, true});
    }

    /**
     * @brief Looks up variable type in current and parent scopes.
     * @param name Variable name to look up
     * @return Type of variable or empty string if not found
     */
    std::string lookup(const Identifier &name) {
        for (auto &l: localVariables) {
            auto it = l.find(name);
            if (it != l.end()) {
                return it->second;
            }
        }
        return "";
    }

    /**
     * @brief Counts inheritance levels to reach field.
     * @param name Field name to look up
     * @param type Output parameter for field's type
     * @return Number of inheritance levels to reach field
     */
    int lookupClassNestedCount(const Identifier &name, Identifier &type) {
        Class *clazz_ = clazz;
        int c = 1;
        while (clazz_) {
            if (clazz_->containsField(name)) {
                type = clazz_->getField(name)->getTypeLexeme();
                return c;
            }
            if (clazz_->getExtends().empty()) {
                break;
            }
            clazz_ = project->getClassByName(clazz_->getExtends());
            c++;
        }
        return 0;
    }
};

std::string generate(
        ThreeAddressCodeGenerator &gen,
        ASTNode *node
);

std::string generate(
        ThreeAddressCodeGenerator &gen,
        std::unique_ptr<ASTNode> *node
);

std::string generate(
        ThreeAddressCodeGenerator &gen,
        CodeBlock *node
);

std::string generate(
        ThreeAddressCodeGenerator &gen,
        ReferenceChain *reference,
        bool getMethod,
        std::string &currentType
);

std::string generate(
        ThreeAddressCodeGenerator &gen,
        ReferenceChain *chain
);

#endif //SIMPLEMINIJAVACOMPILERTOC_GENERATOR_TAC_H
