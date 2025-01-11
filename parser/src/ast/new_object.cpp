#include <utility>

#include "../../include/ast.h"

NewObject::NewObject(
        Token classType_,
        std::unique_ptr<ASTNode> array_size_
) : classType(std::move(classType_)),
    arraySize(std::move(array_size_)) {}

void NewObject::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "NewObject (" + classType.lexeme + ")"
         << " (Type:" << type << ")" << std::endl;
    if (arraySize) {
        strm << std::string(depth + 1, '\t') << "Array:" << std::endl;
        arraySize->print(strm, depth + 2);
    }
}

void NewObject::analyseSemantics(SymbolTable &symbolTable) {
    if (arraySize) {
        arraySize->analyseSemantics(symbolTable);
        if (arraySize->type != "int") {
            error("Array size must be type of 'int' but got '" + arraySize->type + "'");
        }
        type = "int[]";
    } else {
        auto symbol = SymbolTable::getClassSymbolTable(classType.lexeme);
        if (!symbol) {
            error("Undefined class type in NewObject: '" + classType.lexeme + "'");
        }
        type = classType.lexeme;
    }
}