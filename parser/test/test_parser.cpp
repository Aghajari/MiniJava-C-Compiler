#include <iostream>
#include "../include/parser.h"

int main() {
    std::string source_code = R"(
        public class A {
            int a;

            void hello(int t) {
                while(true){}{hi=2;}
            }
        }
    )";

    /*auto tokens = tokenize(source_code);
    for (const auto &token: tokens) {
        if (token.type == TokenType::WHITESPACE) continue;

        std::cout << token << std::endl;
    }*/

    printf("-------\n");

    auto project = parse(source_code);
    std::cout << project << std::endl;
    return 0;
}
