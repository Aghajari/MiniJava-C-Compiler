#include <utility>

#include "../include/symbol_table.h"

std::unordered_map<std::string, std::shared_ptr<SymbolTable>> SymbolTable::classSymbolTables;

SymbolTable::SymbolTable(SymbolTable *parent)
        : parentScope(parent) {}

SymbolTable::SymbolTable(SymbolTable *parent, std::string returnType_)
        : parentScope(parent), returnType(std::move(returnType_)) {}

SymbolTable::SymbolTable(std::string className_, SymbolTable *parent)
        : parentScope(parent), className(std::move(className_)) {}

void SymbolTable::addSymbol(const std::string &name, const Symbol &symbol) {
    if (symbols.find(name) != symbols.end()) {
        error("Symbol '" + name + "' is already declared in this scope.");
    }
    symbols.insert({name, symbol});
}

Symbol *SymbolTable::lookup(const std::string &name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    if (parentScope) {
        return parentScope->lookup(name);
    }
    return nullptr;
}

Symbol *SymbolTable::find(const std::string &name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

void SymbolTable::addClassSymbolTable(const std::string &className, const SymbolTable &table) {
    if (classSymbolTables.find(className) != classSymbolTables.end()) {
        error("Class '" + className + "' is already declared.");
    }
    classSymbolTables[className] = std::make_shared<SymbolTable>(table);
}

SymbolTable *SymbolTable::getClassSymbolTable(const std::string &className) {
    auto it = classSymbolTables.find(className);
    if (it != classSymbolTables.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool SymbolTable::canCast(const std::string &from, const std::string &to) {
    if (from == to) {
        return true;
    }
    SymbolTable *table = SymbolTable::getClassSymbolTable(from);
    while (table) {
        if (table->getClassName() == to) {
            return true;
        }
        table = table->getParent();
    }
    return false;
}

SymbolTable *SymbolTable::getCurrentClassSymbolTable() {
    SymbolTable *current = this;
    while (current) {
        if (current->isClassScope()) {
            return current;
        }
        current = current->parentScope;
    }
    return nullptr;
}