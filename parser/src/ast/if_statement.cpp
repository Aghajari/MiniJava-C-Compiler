#include "../../include/ast.h"

IfStatement::IfStatement(
        std::unique_ptr<ASTNode> condition_,
        std::unique_ptr<CodeBlock> body_,
        std::unique_ptr<ASTNode> else_
) : condition(std::move(condition_)),
    body(std::move(body_)),
    elseBody(std::move(else_)) {}

void IfStatement::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "IfStatement" << " (Type:" << type << ")" << std::endl;
    strm << std::string(depth + 1, '\t') << "Condition:" << " (Type:" << condition->type << ")" << std::endl;
    condition->print(strm, depth + 2);
    strm << std::string(depth + 1, '\t') << "Body:" << std::endl;
    body->print(strm, depth + 2);
    if (elseBody) {
        strm << std::string(depth + 1, '\t') << "ElseBody:" << std::endl;
        elseBody->print(strm, depth + 2);
    }
}

void IfStatement::analyseSemantics(SymbolTable &symbolTable) {
    condition->analyseSemantics(symbolTable);
    if (condition->type != "boolean") {
        error("Condition in 'if' statement must be of type 'boolean', "
              "but got '" + condition->type + "'.");
    }
    body->analyseSemantics(symbolTable);
    if (elseBody) {
        elseBody->analyseSemantics(symbolTable);
    }
    type = "void";
}