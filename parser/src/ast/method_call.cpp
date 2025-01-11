#include "../../include/ast.h"

MethodCall::MethodCall(std::string methodName_)
        : methodName(std::move(methodName_)) {}

void MethodCall::addArgument(std::unique_ptr<ASTNode> argument) {
    arguments.push_back(std::move(argument));
}

void MethodCall::print(std::ostream &strm, int depth) const {
    strm << std::string(depth, '\t') << "MethodCall: " << methodName << std::endl;
    strm << std::string(depth + 1, '\t') << "Params: " << std::endl;
    for (auto &arg: arguments) {
        arg->print(strm, depth + 2);
    }
}

void MethodCall::analyseSemantics(SymbolTable &symbolTable) {
    SymbolTable *objectClassSymbolTable = SymbolTable::getClassSymbolTable(callerType);

    if (!objectClassSymbolTable) {
        error("Type error: Object of type '" + callerType + "' is not a valid class or does not exist.");
    }

    Symbol *methodSymbol = objectClassSymbolTable->lookup(methodName);

    if (!methodSymbol) {
        error("Undefined method: '" + methodName + "' in type '" + callerType + "'.");
    }

    if (!methodSymbol->isMethod) {
        error("'" + methodName + "' is not a method.");
    }

    const std::vector<std::string> &expectedParams = methodSymbol->params;
    if (arguments.size() != expectedParams.size()) {
        error("Argument mismatch in method call to '" + methodName +
              "': expected " + std::to_string(expectedParams.size()) +
              " arguments, but got " + std::to_string(arguments.size()) + ".");
    }

    for (size_t i = 0; i < arguments.size(); ++i) {
        arguments[i]->analyseSemantics(symbolTable);
        if (arguments[i]->type != expectedParams[i]) {
            error("Type mismatch for argument " + std::to_string(i + 1) +
                  " in method call to '" + methodName + "': expected '" +
                  expectedParams[i] + "', but got '" + arguments[i]->type + "'.");
        }
    }

    type = methodSymbol->returnType;
}