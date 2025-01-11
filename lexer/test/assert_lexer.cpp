#include "assert_lexer.h"
#include "../include/lexer.h"

void assertLexer(
        const std::string &testName,
        const std::string &input,
        const std::vector <Token> &expected
) {
    auto tokens = tokenize(input);
    int index = 0;
    for (const auto &token: tokens) {
        if (token.type == TokenType::WHITESPACE) continue;

        if (index >= expected.size()) {
            std::cerr << "Test failed (" << testName << "): Expected null"
                      << ", got " << token << ".\n";
            return;
        } else {
            const auto &expectedToken = expected[index++];
            if (expectedToken.type != token.type || expectedToken.lexeme != token.lexeme) {
                std::cerr << "Test failed (" << testName << "): Expected " << expectedToken
                          << ", got " << token << ".\n";
                return;
            }
        }
    }

    if (index < expected.size()) {
        std::cerr << "Test failed (" << testName << "): Expected " << expected[index]
                  << ", got null.\n";
        return;
    }
    std::cout << "Test passed (" << testName << ").\n";
}

void test_operators() {
    assertLexer("Simple a+b", "a+b", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::IDENTIFIER, "b"),
    });
    assertLexer("Whitespace a + b", "\ta + b\n", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::IDENTIFIER, "b"),
    });
    assertLexer("Complex expression", "(a+b)*c", {
            Token(TokenType::SYMBOL, "("),
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::IDENTIFIER, "b"),
            Token(TokenType::SYMBOL, ")"),
            Token(TokenType::OPERATOR, "*"),
            Token(TokenType::IDENTIFIER, "c"),
    });
    assertLexer("Complex operator sequence", "a--+-b++-~a", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "--"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::OPERATOR, "-"),
            Token(TokenType::IDENTIFIER, "b"),
            Token(TokenType::OPERATOR, "++"),
            Token(TokenType::OPERATOR, "-"),
            Token(TokenType::OPERATOR, "~"),
            Token(TokenType::IDENTIFIER, "a"),
    });
}

void test_strings() {
    assertLexer("Simple string literal", "\"Hello, World!\"", {
            Token(TokenType::STRING, "\"Hello, World!\""),
    });
    assertLexer("String with escape sequences", R"("Hello\nWorld\t!")", {
            Token(TokenType::STRING, R"("Hello\nWorld\t!")"),
    });
    assertLexer("Empty string literal", "\"\"", {
            Token(TokenType::STRING, "\"\""),
    });
    assertLexer("Character literal", "'a'", {
            Token(TokenType::CHAR, "'a'"),
    });
    assertLexer("Character with escape sequence", "'\\n'", {
            Token(TokenType::CHAR, "'\\n'"),
    });
    assertLexer("Unterminated string literal", "\"Hello", {
            Token(TokenType::UNKNOWN, "\"Hello"),
    });
    assertLexer("Unterminated character literal", "'a", {
            Token(TokenType::UNKNOWN, "'a"),
    });
    assertLexer("String with special characters", "\"!@#$%^&*()_+-=<>?\"", {
            Token(TokenType::STRING, "\"!@#$%^&*()_+-=<>?\""),
    });
}

void test_numbers() {
    assertLexer("Floating-point literal with f suffix", "1f", {
            Token(TokenType::NUMBER, "1f"),
    });
    assertLexer("Floating-point literal with underscores and f suffix", "1_2f", {
            Token(TokenType::NUMBER, "1_2f"),
    });
    assertLexer("Floating-point literal with leading dot and d suffix", ".0d", {
            Token(TokenType::NUMBER, ".0d"),
    });
    assertLexer("Floating-point literal in scientific notation", "0.1e-2f", {
            Token(TokenType::NUMBER, "0.1e-2f"),
    });
    assertLexer("Integer literal with underscores", "1_234", {
            Token(TokenType::NUMBER, "1_234"),
    });
    assertLexer("Simple integer literal", "24", {
            Token(TokenType::NUMBER, "24"),
    });
    assertLexer("Hexadecimal literal", "0x1A3", {
            Token(TokenType::HEX_NUMBER, "0x1A3"),
    });
    assertLexer("Binary literal", "0b1010", {
            Token(TokenType::BINARY_NUMBER, "0b1010"),
    });
    assertLexer("Floating-point literal with underscores", "3_1.1___________141_592_653", {
            Token(TokenType::NUMBER, "3_1.1___________141_592_653"),
    });
}

void test_comments() {
    assertLexer("Single-Line Comment", "a + b // This is a comment", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::IDENTIFIER, "b"),
            Token(TokenType::LINE_COMMENT, "// This is a comment"),
    });
    assertLexer("Multi-Line Comment", "a + b /* This is a \n multi-line comment */ d", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::IDENTIFIER, "b"),
            Token(TokenType::BLOCK_COMMENT, "/* This is a \n multi-line comment */"),
            Token(TokenType::IDENTIFIER, "d"),
    });
}

void test_others() {
    assertLexer("Empty input", "", {});
    assertLexer("Annotation", "@Test", {
            Token(TokenType::ANNOTATION, "@Test"),
    });
    assertLexer("Double Colon", "a::b", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::SYMBOL, "::"),
            Token(TokenType::IDENTIFIER, "b"),
    });
    assertLexer("Invalid characters", "a + #", {
            Token(TokenType::IDENTIFIER, "a"),
            Token(TokenType::OPERATOR, "+"),
            Token(TokenType::UNKNOWN, "#"),
    });
    assertLexer("Method declaration with annotations", "@Override public void test() {}", {
            Token(TokenType::ANNOTATION, "@Override"),
            Token(TokenType::KEYWORD, "public"),
            Token(TokenType::KEYWORD, "void"),
            Token(TokenType::IDENTIFIER, "test"),
            Token(TokenType::SYMBOL, "("),
            Token(TokenType::SYMBOL, ")"),
            Token(TokenType::SYMBOL, "{"),
            Token(TokenType::SYMBOL, "}"),
    });
    assertLexer("Lambda Expression", "($arg1)->{/*Comment*/}", {
            Token(TokenType::SYMBOL, "("),
            Token(TokenType::IDENTIFIER, "$arg1"),
            Token(TokenType::SYMBOL, ")"),
            Token(TokenType::SYMBOL, "->"),
            Token(TokenType::SYMBOL, "{"),
            Token(TokenType::BLOCK_COMMENT, "/*Comment*/"),
            Token(TokenType::SYMBOL, "}"),
    });
}

void test_lexer() {
    test_operators();
    test_strings();
    test_numbers();
    test_comments();
    test_others();
}