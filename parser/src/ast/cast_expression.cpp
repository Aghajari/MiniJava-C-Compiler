#include "../../include/ast.h"

CastExpression::CastExpression(
        Token cast_,
        std::unique_ptr<ASTNode> expr_
) : cast(std::move(cast_)),
    expr(std::move(expr_)) {}

void CastExpression::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "CastExpression (" + cast.lexeme + ")"
         << " (Type:" << type << ")" << std::endl;
    expr->print(strm, depth + 1);
}

void CastExpression::analyseSemantics(SymbolTable &symbolTable) {
    expr->analyseSemantics(symbolTable);
    if (!SymbolTable::getClassSymbolTable(cast.lexeme)) {
        error("Undefined type in CastExpression: '" + cast.lexeme + "'");
    }
    type = cast.lexeme;
}