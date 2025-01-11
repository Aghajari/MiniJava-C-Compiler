#include "../../include/ast.h"

BooleanASTNode::BooleanASTNode(Token token_) :
        token(std::move(token_)) {}

void BooleanASTNode::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "Boolean: " << token.lexeme << std::endl;
}

void BooleanASTNode::analyseSemantics(SymbolTable &symbolTable) {
    type = "boolean";
}