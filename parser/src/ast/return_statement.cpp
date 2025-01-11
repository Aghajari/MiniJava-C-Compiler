#include "../../include/ast.h"

ReturnStatement::ReturnStatement(
        std::unique_ptr<ASTNode> expr_
) : expr(std::move(expr_)) {}

void ReturnStatement::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "Return" << std::endl;
    if (expr) {
        expr->print(strm, depth + 1);
    }
}

void ReturnStatement::analyseSemantics(SymbolTable &symbolTable) {
    std::string lhsType = symbolTable.getReturnType();

    type = "void";
    if (expr) {
        expr->analyseSemantics(symbolTable);
        if (expr->type == "void") {
            error("Return type expression expected to be '" + lhsType
                  + "' but got '" + expr->type + "'");
        }

        if (lhsType != expr->type) {
            if (lhsType == "int" ||
                lhsType == "int[]" ||
                lhsType == "boolean" ||
                !SymbolTable::canCast(expr->type, lhsType)) {
                if (expr->getType() != ASTType::AST_CastExpression) {
                    error("Type mismatch in return: Cannot return value of type '" + expr->type +
                          "' to variable/field of type '" + lhsType + "'");
                }
            }
        }
    } else if (symbolTable.getReturnType() != "void") {
        error("Return type expression expected to be '" + symbolTable.getReturnType()
              + "' but got 'void'");
    }
}