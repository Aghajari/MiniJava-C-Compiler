#include <iostream>
#include "assert_lexer.h"
#include "../include/lexer.h"

int main() {
    printf("-------\n");

    std::string source_code = R"(
        public class A {
            int[] a;
        }
    )";

    auto tokens = tokenize(source_code);
    for (const auto &token: tokens) {
        if (token.type == TokenType::WHITESPACE) continue;

        std::cout << token << std::endl;
    }
    return 0;
}
