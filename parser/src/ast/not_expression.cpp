#include <utility>

#include "../../include/ast.h"

NotExpression::NotExpression(
        Token op_,
        std::unique_ptr<ASTNode> expr_
) : op(std::move(op_)), expr(std::move(expr_)) {}

void NotExpression::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "NotExpression (" << op.lexeme << ")"
         << " (Type:" << type << ")" << std::endl;
    expr->print(strm, depth + 1);
}

void NotExpression::analyseSemantics(SymbolTable &symbolTable) {
    expr->analyseSemantics(symbolTable);
    if (op.lexeme == "!") {
        if (expr->type != "boolean") {
            error("Type error in NotExpression: logical negation (!) requires a 'boolean' operand, but found '" +
                  expr->type + "'");
        }
        type = "boolean";
    } else {
        if (expr->type != "int") {
            error("Type error in NotExpression: bitwise not (~) requires an 'int' operand, but found '" +
                  expr->type + "'");
        }
        type = "int";
    }
}