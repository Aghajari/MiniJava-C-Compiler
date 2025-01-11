#include "../../include/ast.h"

ArrayCall::ArrayCall(
        std::string array_name_,
        std::unique_ptr<ASTNode> bracket_
) : arrayName(std::move(array_name_)),
    bracket(std::move(bracket_)) {}

void ArrayCall::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "ArrayCall: " << arrayName << std::endl;
    strm << std::string(depth + 1, '\t') << "Index: " << std::endl;
    bracket->print(strm, depth + 2);
}

void ArrayCall::analyseSemantics(SymbolTable &symbolTable) {
    SymbolTable *objectClassSymbolTable = callerType.empty() ? &symbolTable : SymbolTable::getClassSymbolTable(callerType);
    if (!objectClassSymbolTable) {
        error("Type error: Object of type '" + callerType + "' is not a valid class or does not exist.");
    }

    Symbol *symbol = objectClassSymbolTable->lookup(arrayName);
    if (!symbol) {
        error("Undefined array: '" + arrayName + "'");
    }

    if (symbol->type != "int[]") {
        error("'" + arrayName + "' is not an array.");
    }

    bracket->analyseSemantics(symbolTable);
    if (bracket->type != "int") {
        error("Type mismatch for array index '" + arrayName +
              "': expected 'int', but got '" + bracket->type + "'.");
    }

    type = "int";
}