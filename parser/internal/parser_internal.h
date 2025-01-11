#ifndef SIMPLEMINIJAVACOMPILERTOC_PARSER_INTERNAL_H
#define SIMPLEMINIJAVACOMPILERTOC_PARSER_INTERNAL_H

#include "../../common/include/error_handler.h"
#include "../include/project.h"
#include "streamer.h"

struct ParamSignature {
    bool isStatic;
    bool isField;
    MiniJavaType type;
    std::string type_lexeme;
    Identifier name;
};


bool isAssignment(Token *token);

bool isValidType(
        Token *token,
        bool canBeVoid
);

Token *parseParam(
        ParamSignature *sign,
        Project &project,
        TokenStreamer &streamer
);

void parseFieldOrMethod(
        ParamSignature *sign,
        Project &project,
        TokenStreamer &streamer
);

void parseMethodParams(
        Method *method,
        Project &project,
        TokenStreamer &streamer
);

void parseMethodBody(
        Method *method,
        Project &project,
        TokenStreamer &streamer
);

void parseCodeBlock(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
);

bool parseClass(
        Project &project,
        TokenStreamer &streamer
);

void parseLocalVariableCode(
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
);

void parseAssignmentForLocalVariable(
        CodeBlock *codeBlock,
        Token *fieldNameToken,
        Token *assignmentToken,
        Project &project,
        TokenStreamer &streamer
);

void parseUnary(
        Token *reference,
        ReferenceChain *referenceChain,
        CodeBlock *codeBlock,
        Project &project,
        TokenStreamer &streamer
);

void parseAssignment(
        CodeBlock *codeBlock,
        Token *reference,
        Project &project,
        TokenStreamer &streamer
);

std::unique_ptr<ASTNode> parseIfStatement(
        Project &project,
        TokenStreamer &streamer
);

std::unique_ptr<ASTNode> parseWhileStatement(
        Project &project,
        TokenStreamer &streamer
);

std::unique_ptr<ASTNode> parseDoWhileStatement(
        Project &project,
        TokenStreamer &streamer
);

std::unique_ptr<ASTNode> parseForStatement(
        Project &project,
        TokenStreamer &streamer
);

std::unique_ptr<ASTNode> parseExpression(
        Project &project,
        TokenStreamer &streamer
);

void parseSimpleStatement(
        CodeBlock *codeBlock,
        Token *token,
        Project &project,
        TokenStreamer &streamer
);

void parseStatement(
        CodeBlock *codeBlock,
        Token *token,
        Project &project,
        TokenStreamer &streamer
);

std::unique_ptr<CodeBlock> parseCodeBlockOrStatement(
        Token *token,
        Project &project,
        TokenStreamer &streamer
);

#endif //SIMPLEMINIJAVACOMPILERTOC_PARSER_INTERNAL_H
