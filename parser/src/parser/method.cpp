#include "../../internal/parser_internal.h"

/**
 * @brief Parses the body of a method and its statements.
 *
 * This function validates the opening `{` token for the method body and delegates
 * the parsing of the body contents to `parseCodeBlock()`. It ensures the method body
 * is correctly structured within a code block.
 *
 * @param method The `Method` object to populate with the parsed method body.
 * @param project The `Project` context being parsed.
 * @param streamer The `TokenStreamer` used to process tokens sequentially.
 */
void parseMethodBody(
        Method *method,
        Project &project,
        TokenStreamer &streamer
) {
    Token *token = streamer.read();
    if (token == nullptr || token->lexeme != "{") {
        error("Failed to parse method " + method->getName() + ", Expected {", token);
    }

    parseCodeBlock(method->getCodeBlock(), project, streamer);
}