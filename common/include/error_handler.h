#ifndef SIMPLEMINIJAVACOMPILERTOC_ERROR_HANDLER_H
#define SIMPLEMINIJAVACOMPILERTOC_ERROR_HANDLER_H

#include "../../lexer/include/lexer.h"

void error(const std::string &message);

void error(const std::string &message, Token *token);

#endif //SIMPLEMINIJAVACOMPILERTOC_ERROR_HANDLER_H
