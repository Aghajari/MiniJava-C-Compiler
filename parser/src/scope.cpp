#include "../include/scope.h"

Field::Field(MiniJavaType type, Identifier type_lexeme, Identifier name) :
        type(type), type_lexeme(std::move(type_lexeme)), name(std::move(name)) {}

Identifier Field::getName() {
    return name;
}

MiniJavaType Field::getType() {
    return type;
}

Identifier Field::getTypeLexeme() {
    return type_lexeme;
}

Method::Method(MiniJavaType type, Identifier type_lexeme, Identifier name, bool main) :
        type(type), type_lexeme(std::move(type_lexeme)), name(std::move(name)), main(main) {}

void Method::addParam(const Field &field) {
    params.push_back(field);
}

std::vector<Field> *Method::getParams() {
    return &params;
}

bool Method::containsParam(const Identifier &paramName) {
    for (auto &p: params) {
        if (p.getName() == paramName) {
            return true;
        }
    }
    return false;
}

CodeBlock *Method::getCodeBlock() {
    return &code;
}

bool Method::isMain() {
    return main;
}

Identifier Method::getName() {
    return name;
}

MiniJavaType Method::getReturnType() {
    return type;
}

Identifier Method::getReturnTypeLexeme() {
    return type_lexeme;
}

Class::Class(Identifier name, Identifier extends)
        : Scope(), name(std::move(name)), extends(std::move(extends)) {}

void Class::addField(Field &field) {
    unsigned long index = fields.size();
    fields.push_back(field);
    fieldsMap.insert({field.getName(), index});
}

void Class::addMethod(Method &method) {
    unsigned long index = methods.size();
    methods.push_back(std::move(method));
    methodsMap.insert({methods[index].getName(), index});
}

std::vector<Field> *Class::getFields() {
    return &fields;
}

std::vector<Method> *Class::getMethods() {
    return &methods;
}

bool Class::containsField(const Identifier &fieldName) {
    return fieldsMap.contains(fieldName);
}

Field *Class::getField(const Identifier &fieldName) {
    return &fields[fieldsMap[fieldName]];
}

bool Class::containsMethod(const Identifier &methodName) {
    return methodsMap.contains(methodName);
}

Identifier Class::getName() {
    return name;
}

Identifier Class::getExtends() {
    return extends;
}