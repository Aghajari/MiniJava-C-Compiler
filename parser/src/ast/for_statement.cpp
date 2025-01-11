#include "../../include/ast.h"

ForStatement::ForStatement(
        std::unique_ptr<CodeBlock> initialization_,
        std::unique_ptr<ASTNode> condition_,
        std::unique_ptr<CodeBlock> update_,
        std::unique_ptr<CodeBlock> body_
) : initialization(std::move(initialization_)),
    condition(std::move(condition_)),
    update(std::move(update_)),
    body(std::move(body_)) {}

void ForStatement::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "ForStatement:" << std::endl;

    if (initialization) {
        strm << std::string(depth + 1, '\t') << "Initialization:" << std::endl;
        initialization->print(strm, depth + 2);
    }
    if (condition) {
        strm << std::string(depth + 1, '\t') << "Condition:" << std::endl;
        condition->print(strm, depth + 2);
    }
    if (update) {
        strm << std::string(depth + 1, '\t') << "Update:" << std::endl;
        update->print(strm, depth + 2);
    }
    if (body) {
        strm << std::string(depth + 1, '\t') << "Body:" << std::endl;
        body->print(strm, depth + 2);
    }
}

void ForStatement::analyseSemantics(SymbolTable &symbolTable) {
    auto scopeTable = SymbolTable(&symbolTable, symbolTable.getReturnType());
    if (initialization) {
        initialization->analyseSemanticsWithSameScope(scopeTable);
    }
    if (condition) {
        condition->analyseSemantics(scopeTable);
        if (condition->type != "boolean") {
            error("The condition in a for-loop must evaluate to 'boolean', found '" + condition->type + "'.");
        }
    }
    if (update) {
        update->analyseSemantics(scopeTable);
    }
    if (body) {
        body->analyseSemantics(scopeTable);
    }
    type = "void";
}