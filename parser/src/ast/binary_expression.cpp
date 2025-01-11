#include "../../include/ast.h"

BinaryExpression::BinaryExpression(
        Token op_,
        std::unique_ptr<ASTNode> left_,
        std::unique_ptr<ASTNode> right_
) : op(std::move(op_)),
    left(std::move(left_)),
    right(std::move(right_)) {}

void BinaryExpression::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "BinaryExpression (" << op.lexeme << ")"
         << " (Type:" << type << ")" << std::endl;
    left->print(strm, depth + 1);
    right->print(strm, depth + 1);
}

void BinaryExpression::analyseSemantics(SymbolTable &symbolTable) {
    left->analyseSemantics(symbolTable);
    right->analyseSemantics(symbolTable);

    if (left->type != right->type) {
        error("Type mismatch in BinaryExpression: '" + left->type + "' and '" + right->type + "'");
    }

    if (op.lexeme == "+" ||
        op.lexeme == "-" ||
        op.lexeme == "*" ||
        op.lexeme == "/" ||
        op.lexeme == "%" ||
        op.lexeme == "&" ||
        op.lexeme == "^" ||
        op.lexeme == "|") {
        if (left->type != "int") {
            error("Arithmetic operators require 'int', found '" + left->type + "'");
        }
        type = "int";
    } else if (op.lexeme == "&&" || op.lexeme == "||") {
        if (left->type != "boolean") {
            error("Logical operators require 'boolean', found '" + left->type + "'");
        }
        type = "boolean";
    } else {
        if (op.lexeme == "<" || op.lexeme == ">" || op.lexeme == "<=" || op.lexeme == ">=") {
            if (left->type != "int" || right->type != "int") {
                error("Relational operators require 'int', found '" +
                      left->type + "' and '" + right->type + "'");
            }
        } else if (op.lexeme == "==" || op.lexeme == "!=") {
            if (left->type != right->type) {
                error("Equality operators require matching types, found '" +
                      left->type + "' and '" + right->type + "'");
            }
        } else {
            error("Unsupported relational operator: " + op.lexeme);
        }

        type = "boolean";
    }
}
