#include "../../include/ast.h"

Assignment::Assignment(
        ReferenceChain reference_,
        Token token_,
        std::unique_ptr<ASTNode> expression_
) : reference(std::move(reference_)),
    assignmentToken(std::move(token_)),
    expression(std::move(expression_)) {}

void Assignment::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "Assignment (";
    strm << assignmentToken.lexeme << ")"
         << " (Type: " << reference.type << " to " << expression->type << ")"
         << std::endl;
    reference.print(strm, depth + 1);
    expression->print(strm, depth + 1);
}

void Assignment::analyseSemantics(SymbolTable &symbolTable) {
    reference.analyseSemantics(symbolTable);
    std::string lhsType = reference.type;

    if (reference.isArrayLength) {
        error("You can not set length of array '" +
              reference.chain[reference.chain.size() - 2].first.lexeme + "'");
    }

    expression->analyseSemantics(symbolTable);
    std::string rhsType = expression->type;

    if (assignmentToken.lexeme == "+=" ||
        assignmentToken.lexeme == "-=" ||
        assignmentToken.lexeme == "*=" ||
        assignmentToken.lexeme == "/=" ||
        assignmentToken.lexeme == "<<=" ||
        assignmentToken.lexeme == ">>=") {
        if (lhsType != "int") {
            error("Invalid compound assignment: '" + assignmentToken.lexeme +
                  "' requires 'int', but found '" + lhsType + "'");
        }
        if (rhsType != "int") {
            error("Invalid compound assignment: Cannot apply '" + assignmentToken.lexeme +
                  "' with incompatible right-hand side type '" + rhsType + "'");
        }
    } else if (assignmentToken.lexeme == "&=" ||
               assignmentToken.lexeme == "|=" ||
               assignmentToken.lexeme == "^=") {
        if (lhsType != "int" && lhsType != "boolean") {
            error("Invalid compound assignment: '" + assignmentToken.lexeme +
                  "' requires 'int' or 'boolean', but found '" + lhsType + "'");
        } else if (lhsType != rhsType) {
            error("Type mismatch in assignment: Cannot assign value of type '" + rhsType +
                  "' to variable/field of type '" + lhsType + "'");
        }
    } else if (assignmentToken.lexeme == "=") {
        if (lhsType == "void" || rhsType == "void") {
            error("Type mismatch in assignment: Cannot assign value of type void");
        }

        bool rhsPrimitive = rhsType == "int" || rhsType == "int[]" || rhsType == "boolean";
        bool lhsPrimitive = lhsType == "int" || lhsType == "int[]" || lhsType == "boolean";

        if (lhsType != rhsType) {
            if (lhsPrimitive || rhsPrimitive || !SymbolTable::canCast(rhsType, lhsType)) {
                error("Type mismatch in assignment: Cannot assign value of type '" + rhsType +
                      "' to variable/field of type '" + lhsType + "'");
            }
        }
    } else {
        error("Unsupported assignment operator: " + assignmentToken.lexeme);
    }
    type = "void";
}
