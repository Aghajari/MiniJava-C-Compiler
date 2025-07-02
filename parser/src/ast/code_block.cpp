#include "../../include/ast.h"

CodeBlock::CodeBlock() = default;

void CodeBlock::addCode(std::unique_ptr<ASTNode> &node) {
    codes.push_back(std::move(node));
}

void CodeBlock::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "CodeBlock (Type:" << type << ")" << std::endl;
    for (auto &c: codes) {
        c->print(strm, depth + 1);
    }
}

void CodeBlock::analyseSemantics(SymbolTable &symbolTable) {
    auto scopeTable = SymbolTable(&symbolTable, symbolTable.getReturnType());
    type = "void";
    bool returns = false;
    for (auto &code: codes) {
        if (returns) {
            error("Unreachable statement");
        }
        code->analyseSemantics(scopeTable);
        if (!returns) {
            if (code->getType() == AST_ReturnStatement) {
                type = symbolTable.getReturnType();
                if (type == "void") {
                    type = "return-void";
                }
                returns = true;
            } else if (code->getType() == AST_IfStatement && code->type != "void") {
                type = code->type;
                returns = true;
            }
        }
    }
}

void CodeBlock::analyseSemanticsWithSameScope(SymbolTable &symbolTable) {
    for (auto &code: codes) {
        code->analyseSemantics(symbolTable);
    }
    type = "void";
}