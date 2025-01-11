#include "../../include/ast.h"

WhileStatement::WhileStatement(
        std::unique_ptr<ASTNode> condition_,
        std::unique_ptr<CodeBlock> body_
) : condition(std::move(condition_)),
    body(std::move(body_)) {}

void WhileStatement::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "WhileStatement" << " (Type:" << type << ")" << std::endl;
    strm << std::string(depth + 1, '\t') << "Condition:" << std::endl;
    condition->print(strm, depth + 2);
    strm << std::string(depth + 1, '\t') << "Body:" << std::endl;
    body->print(strm, depth + 2);
}

void WhileStatement::analyseSemantics(SymbolTable &symbolTable) {
    condition->analyseSemantics(symbolTable);
    if (condition->type != "boolean") {
        error("Condition in 'while' statement must be of type 'boolean', "
              "but got '" + condition->type + "'.");
    }
    body->analyseSemantics(symbolTable);
    type = "void";
}