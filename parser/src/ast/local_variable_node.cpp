#include "../../include/ast.h"

LocalVariableASTNode::LocalVariableASTNode(Field field_) :
        field(std::move(field_)) {}

void LocalVariableASTNode::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "LocalVariable: " << field
         << " (Type:" << type << ")" << std::endl;
}

void LocalVariableASTNode::analyseSemantics(SymbolTable &symbolTable) {
    if (field.getTypeLexeme() != "int" && field.getTypeLexeme() != "int[]" && field.getTypeLexeme() != "boolean") {
        SymbolTable *typeSymbol = SymbolTable::getClassSymbolTable(field.getTypeLexeme());
        if (!typeSymbol) {
            error("Invalid type in variable declaration: '" + field.getTypeLexeme() + "'");
        }
    }

    symbolTable.addSymbol(field.getName(), Symbol(field.getName(), field.getTypeLexeme()));
    type = field.getTypeLexeme();
}