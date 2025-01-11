#ifndef SIMPLEMINIJAVACOMPILERTOC_GENERATOR_H
#define SIMPLEMINIJAVACOMPILERTOC_GENERATOR_H

#include "../../parser/include/project.h"

/**
 * @brief Main entry point for generating C code from a Mini-Java project.
 *
 * This function coordinates the complete code generation process, including:
 * 1. Generating header and source files for each class
 * 2. Creating support files for arrays
 * 3. Setting up the build system
 *
 * The generation process creates the following structure:
 * ```
 * compile/
 * ├── CMakeLists.txt           // Build system configuration
 * ├── __int_array.h           // Support for int[] operations
 * ├── __int_array.c
 * ├── ClassA.h                // Generated class headers
 * ├── ClassA.c                // Generated class implementations
 * ├── ClassB.h
 * ├── ClassB.c
 * └── ...
 * ```
 *
 * Example Input:
 * ```java
 * class A {
 *     int x;
 *     void method() { x = 42; }
 * }
 *
 * class B extends A {
 *     void test() { super.method(); }
 * }
 * ```
 *
 * Generated Files:
 * 1. Class Headers (A.h, B.h):
 * ```c
 * // A.h
 * struct A {
 *     int x;
 *     void (*$_function_method)(void*);
 * };
 * typedef struct A A;
 *
 * // B.h
 * struct B {
 *     A super;
 *     void (*$_function_test)(void*);
 * };
 * typedef struct B B;
 * ```
 *
 * 2. Class Sources (A.c, B.c):
 * ```c
 * // A.c
 * A *$_new_A() {
 *     A *self = (A *) malloc(sizeof(A));
 *     self->x = 0;
 *     self->$_function_method = A_method;
 *     return self;
 * }
 *
 * void A_method(void* $this) {
 *     A *super = (A *) $this;
 *     super->x = 24;
 * }
 *
 * // B.c
 * B *$_new_B() {
 *     B *self = (B *) malloc(sizeof(B));
 *     self->$_function_test = B_test;
 *     self->super.x = 0;
 *     self->super.$_function_method = A_method;
 *     return self;
 * }
 *
 * void B_test(void* $this) {
 *     B *super = (B *) $this;
 *     super->super.$_function_method(this);
 * }
 * ```
 *
 * @param project Pointer to the validated Project AST
 *
 * Generation Process:
 * 1. For each class:
 *    - Generate class header (.h)
 *      + Generate inheritance with structs
 *      + Declare initialization and functions
 *    - Generate class implementation (.c)
 *      + Generate initialization and functions
 *      + Generate TAC code from AST
 *    - Track included dependencies
 * 2. Generate int array support files
 * 3. Generate CMake build configuration
 */
void generate(Project *project);

#endif //SIMPLEMINIJAVACOMPILERTOC_GENERATOR_H
