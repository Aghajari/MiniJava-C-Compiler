#include "../../include/ast.h"

void ReferenceChain::addField(const Token &token) {
    chain.emplace_back(token, nullptr);
}

void ReferenceChain::addNode(
        const Token &token,
        std::unique_ptr<ASTNode> node
) {
    chain.emplace_back(token, std::move(node));
}

void ReferenceChain::print(std::ostream &strm, int depth) const {
    unsigned long l = chain.size();
    strm << std::string(depth, '\t');
    for (int i = 0; i < l; i++) {
        if (chain[i].second) {
            strm << std::endl;
            chain[i].second->print(strm, depth);
            if (i != l - 1) {
                strm << std::string(depth, '\t');
            }
        } else {
            strm << chain[i].first.lexeme;
            if (i != l - 1) {
                strm << '.';
            }
        }
    }
    strm << std::endl;
}

void ReferenceChain::analyseSemantics(SymbolTable &symbolTable) {
    isArrayLength = false;
    if (chain.empty()) {
        error("Empty reference in ReferenceASTNode");
    }
    Symbol *currentSymbol;

    auto &front = chain.front();
    const std::string &name = front.first.lexeme;

    if (name == "this" || front.second) {
        SymbolTable *table = symbolTable.getCurrentClassSymbolTable();
        if (!table) {
            error("Failed to get current class symbol table");
        }
        type = table->getClassName();
    } else {
        currentSymbol = symbolTable.lookup(name);
        if (!currentSymbol) {
            error("Undefined reference: '" + name + "'");
        }
        type = currentSymbol->type;
    }

    if (front.second) {
        if (front.second->getType() == ASTType::AST_MethodCall) {
            ((MethodCall *) front.second.get())->callerType = type;
        } else if (front.second->getType() == ASTType::AST_ArrayCall) {
            ((ArrayCall *) front.second.get())->callerType = "";
        }
        front.second->analyseSemantics(symbolTable);
        type = front.second->type;
    }

    for (size_t i = 1; i < chain.size(); ++i) {
        auto &entry = chain[i];
        const std::string &member = entry.first.lexeme;
        if (entry.second) {
            if (entry.second->getType() == ASTType::AST_MethodCall) {
                ((MethodCall *) entry.second.get())->callerType = type;
            } else if (entry.second->getType() == ASTType::AST_ArrayCall) {
                ((ArrayCall *) entry.second.get())->callerType = type;
            }
            entry.second->analyseSemantics(symbolTable);
            type = entry.second->type;
        } else {
            SymbolTable *classSymbolTable = SymbolTable::getClassSymbolTable(type);
            if (!classSymbolTable) {
                error("Type '" + type + "' has no members. Cannot access '" + member + "'");
            }
            currentSymbol = classSymbolTable->lookup(member);

            if (!currentSymbol) {
                error("Undefined member '" + member + "'");
            }

            if (type == "int[]" && member == "length") {
                isArrayLength = true;
            }
            type = currentSymbol->type;
        }
    }
}