#include "../../include/ast.h"

CodeBlock::CodeBlock() = default;

void CodeBlock::addCode(std::unique_ptr<ASTNode> &node) {
    codes.push_back(std::move(node));
}

void CodeBlock::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "CodeBlock" << std::endl;
    for (auto &c: codes) {
        c->print(strm, depth + 1);
    }
}

void CodeBlock::analyseSemantics(SymbolTable &symbolTable) {
    auto scopeTable = SymbolTable(&symbolTable, symbolTable.getReturnType());
    for (auto &code: codes) {
        code->analyseSemantics(scopeTable);
    }
    type = "void";
}

void CodeBlock::analyseSemanticsWithSameScope(SymbolTable &symbolTable) {
    for (auto &code: codes) {
        code->analyseSemantics(symbolTable);
    }
    type = "void";
}