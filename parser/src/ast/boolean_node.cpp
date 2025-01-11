#include "../../include/ast.h"

NumberASTNode::NumberASTNode(Token token_) :
        token(std::move(token_)) {}

void NumberASTNode::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "Number: " << token.lexeme << std::endl;
}

void NumberASTNode::analyseSemantics(SymbolTable &symbolTable) {
    type = "int";
}