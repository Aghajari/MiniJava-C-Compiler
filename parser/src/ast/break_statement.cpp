#include "../../include/ast.h"

void BreakStatement::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "BreakStatement" << std::endl;
}

void BreakStatement::analyseSemantics(SymbolTable &symbolTable) {
    type = "void";
}