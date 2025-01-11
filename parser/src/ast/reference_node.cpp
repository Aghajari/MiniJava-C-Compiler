#include "../../include/ast.h"

ReferenceASTNode::ReferenceASTNode(
        ReferenceChain reference_
) : reference(std::move(reference_)) {}

void ReferenceASTNode::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "Reference " << "(Type:" << type << "):" << std::endl;
    reference.print(strm, depth + 1);
}

void ReferenceASTNode::analyseSemantics(SymbolTable &symbolTable) {
    reference.analyseSemantics(symbolTable);
    type = reference.type;
}