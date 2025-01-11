#ifndef SIMPLEMINIJAVACOMPILERTOC_GENERATOR_INTERNAL_H
#define SIMPLEMINIJAVACOMPILERTOC_GENERATOR_INTERNAL_H

#include "../../common/include/error_handler.h"
#include "../include/generator.h"
#include "../../lexer/include/lexer.h"

void write_file(const std::string& fileName, const std::string& source);

void write_cmake();

void write_int_array();

void generate_class_header(Project *project, Class *clazz, std::map<Identifier, bool> &included);

void generate_class_source(Project *project, Class *clazz, std::map<Identifier, bool> &included);

std::string get_type(const Identifier &type);

std::string get_type(Field *field);

#endif //SIMPLEMINIJAVACOMPILERTOC_GENERATOR_INTERNAL_H
