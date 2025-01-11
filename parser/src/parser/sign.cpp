#include "../../internal/parser_internal.h"

/**
 * @brief Checks if the given token represents a valid Mini-Java type.
 *
 * Valid types include:
 * - `int`, `boolean` (primitive types)
 * - `void` (only if `canBeVoid` is `true`)
 * - Any identifier (custom class names).
 *
 * @param token The token to validate.
 * @param canBeVoid Whether `void` is considered a valid type.
 * @return `true` if the token represents a valid type, `false` otherwise.
 */
bool isValidType(Token *token, bool canBeVoid) {
    if (token->type == TokenType::KEYWORD) {
        return token->lexeme == "int" || token->lexeme == "boolean" || (canBeVoid && token->lexeme == "void");
    } else {
        return token->type == TokenType::IDENTIFIER;
    }
}

/**
 * @brief Parses and validates the type of a parameter, field, or method return value.
 *
 * This function accepts optional modifiers (`public`, `static`) and supports the following types:
 * - Primitive types: `int`, `boolean`
 * - Arrays: `int[]`
 * - Void return types (for methods)
 * - Custom class types (identifiers)
 *
 * @param sign The `ParamSignature` object to store the parsed type information.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` for sequential token processing.
 * @param canHaveModifier Whether the type may have `public` or `static` modifiers.
 * @param canBeVoid Whether the type may be `void`.
 */
void parseType(
        ParamSignature *sign,
        Project &project,
        TokenStreamer &streamer,
        bool canHaveModifier,
        bool canBeVoid
) {
    Token *startToken = streamer.read();
    if (canHaveModifier) {
        // skip public modifier
        if (startToken->type == KEYWORD && startToken->lexeme == "public") {
            startToken = streamer.read();
        }

        if (startToken->type == KEYWORD && startToken->lexeme == "static") {
            startToken = streamer.read();
            sign->isStatic = true;
        }
    }

    if (!isValidType(startToken, canBeVoid)) {
        error("Failed to parse type, Expected a type", startToken);
    }

    sign->type_lexeme = startToken->lexeme;
    if (startToken->lexeme == "int") {
        Token *token = streamer.read();
        if (token != nullptr && token->lexeme == "[") {
            Token *token2 = streamer.read();
            if (token2 == nullptr || token2->lexeme != "]") {
                error("Failed to parse type, Expected int[]", token2 == nullptr ? token : token2);
            } else {
                sign->type = MiniJavaType_INT_ARRAY;
                sign->type_lexeme = "int[]";
            }
        } else {
            streamer.unread();
            sign->type = MiniJavaType_INT;
        }
    } else if (startToken->lexeme == "boolean") {
        sign->type = MiniJavaType_BOOLEAN;
    } else if (startToken->lexeme == "void") {
        sign->type = MiniJavaType_VOID;
    } else {
        sign->type = MiniJavaType_CLASS;
    }
}

/**
 * @brief Parses a single parameter definition (type and name) in a method.
 *
 * A valid parameter consists of:
 * - A valid type: `int`, `boolean`, `int[]`, or a class name.
 * - An identifier representing the parameter's name.
 *
 * @param sign The `ParamSignature` object to populate.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` for sequential token processing.
 * @return The last parsed token (parameter's identifier).
 */
Token *parseParam(
        ParamSignature *sign,
        Project &project,
        TokenStreamer &streamer
) {
    parseType(sign, project, streamer, false, false);

    Token *token = streamer.read();
    if (token == nullptr || token->type != IDENTIFIER) {
        if (token != nullptr && token->lexeme == "[") {
            token = streamer.read();
            if (token == nullptr || token->lexeme != "]") {
                error("Failed to parse param, Expected ]", token);
            }
            token = streamer.read();
            if (token == nullptr || token->type != IDENTIFIER) {
                error("Failed to parse param, Expected identifier", token);
            }
        } else {
            error("Failed to parse param, Expected identifier", token);
        }
    }
    sign->name = token->lexeme;
    return token;
}

/**
 * @brief Parses a field or method declaration and determines its type and modifiers.
 *
 * Differentiates between a field (`;`) and a method (`(`) based on the token following the identifier.
 * Validates optional modifiers (`public`, `static`) and ensures:
 * - Static fields are not allowed.
 * - Only the `main` method can be declared as `static`.
 *
 * @param sign The `ParamSignature` object to store parsed field or method information.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` for sequential token processing.
 */
void parseFieldOrMethod(
        ParamSignature *sign,
        Project &project,
        TokenStreamer &streamer
) {
    sign->isStatic = false;
    parseType(sign, project, streamer, true, true);

    Token *token = streamer.read();
    if (token == nullptr || token->type != IDENTIFIER) {
        error("Failed to parse field, Expected identifier", token);
    }
    sign->name = token->lexeme;

    token = streamer.read();
    if (token == nullptr || (token->lexeme != ";" && token->lexeme != "(")) {
        error("Failed to parse field, Expected ;", token);
    }
    sign->isField = token->lexeme == ";";

    if (sign->isStatic) {
        if (sign->isField) {
            error("Failed to parse field, Field can not be static", token);
        } else if (sign->type != MiniJavaType_VOID || sign->name != "main") {
            error("Failed to parse method, Only main method can be static", token);
        }
    }
}

/**
 * @brief Parses the parameter list of a method and validates their uniqueness.
 *
 * This function parses multiple parameter definitions within a method and ensures:
 * - Parameters must have unique names.
 * - The parameter list ends properly with a closing parenthesis (`)`).
 *
 * @param method The `Method` object to populate with parsed parameters.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` for sequential token processing.
 */
void parseMethodParams(
        Method *method,
        Project &project,
        TokenStreamer &streamer
) {
    Token *token;
    if (streamer.peek()->lexeme == ")") {
        token = streamer.read();
    } else {
        do {
            ParamSignature sign = {};
            parseParam(&sign, project, streamer);
            if (method->containsParam(sign.name)) {
                error("Param " + sign.name + " already exists in " + method->getName(), token);
            }
            method->addParam(Field(sign.type, sign.type_lexeme, sign.name));
            token = streamer.read();
        } while (token != nullptr && token->lexeme == ",");
    }


    if (token == nullptr || token->lexeme != ")") {
        error("Failed to parse method, expected , or )", token);
    }
}