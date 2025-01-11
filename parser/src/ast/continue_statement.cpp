#include "../../include/ast.h"

void ContinueStatement::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "ContinueStatement" << std::endl;
}

void ContinueStatement::analyseSemantics(SymbolTable &symbolTable) {
    type = "void";
}