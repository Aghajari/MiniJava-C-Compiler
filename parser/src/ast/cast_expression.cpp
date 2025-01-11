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
    if (cast.lexeme != "int" &&
        cast.lexeme != "int[]" &&
        cast.lexeme != "boolean" &&
        !SymbolTable::getClassSymbolTable(cast.lexeme)) {
        error("Undefined type in CastExpression: '" + cast.lexeme + "'");
    }

    type = cast.lexeme;
    if (expr->type == cast.lexeme) {
        return;
    }

    bool rhsPrimitive = expr->type == "int" || expr->type == "int[]" || expr->type == "boolean";
    bool lhsPrimitive = cast.lexeme == "int" || cast.lexeme == "int[]" || cast.lexeme == "boolean";
    if (rhsPrimitive ||
        lhsPrimitive ||
        (!SymbolTable::canCast(cast.lexeme, expr->type) &&
         !SymbolTable::canCast(expr->type, cast.lexeme))) {
        error("Cannot cast type '" + expr->type + "' to type '" + cast.lexeme + "'");
    }
}