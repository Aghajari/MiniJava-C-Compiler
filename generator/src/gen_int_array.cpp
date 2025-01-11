#include "../internal/generator_internal.h"

/**
 * @brief Writes the default implementation of the `int[]` type for C code compilation.
 *
 * Since C does not natively support arrays with a `length` property (as in Mini-Java),
 * this function generates a custom implementation for the `int[]` type to simulate the behavior
 * expected in Mini-Java. It writes two supporting files:
 *
 * - **`__int_array.h`**:
 *   - Defines a struct (`__int_array`) representing the `int[]` type.
 *     - `length`: Stores the size of the array.
 *     - `data`: A pointer to the dynamically-allocated array data.
 *   - Declares a function `$_new___int_array`, which constructs instances of the struct.
 *
 * - **`__int_array.c`**:
 *   - Implements the `$_new___int_array(int size)` function for allocating and initializing an array.
 *   - Includes:
 *     - Allocation of memory for the struct (`__int_array`).
 *     - Initialization of the `length` field with the specified size.
 *     - Allocation and zero-initialization of the `data` array.
 *
 * **Usage in Generated C Code:**
 * - This implementation allows the generated C code to use `int[]` transparently by calling `$_new___int_array(size)`.
 * - The `length` field enables Mini-Java's `array.length` property to be correctly implemented in C.
 *
 * Example:
 * ```java
 * int[] arr = new int[10];
 * System.out.println(arr.length);
 * ```
 * Translated to C:
 * ```c
 * __int_array *arr = $_new___int_array(10);
 * printf("%d\n", arr->length);
 * ```
 *
 * @note This function writes the `__int_array` files directly to disk and must be called
 * before generating code that references arrays.
 */
void write_int_array() {
    write_file("__int_array.h", "#ifndef __INT_ARRAY_H\n"
                                "#define __INT_ARRAY_H\n"
                                "\n"
                                "typedef struct {\n"
                                "    int length;\n"
                                "    int *data;\n"
                                "} __int_array;\n"
                                "\n"
                                "__int_array *$_new___int_array(int size);\n"
                                "\n"
                                "#endif //__INT_ARRAY_H\n");

    write_file("__int_array.c", "#include \"__int_array.h\"\n"
                                "\n"
                                "#include <stdio.h>\n"
                                "#include <stdlib.h>\n"
                                "\n"
                                "__int_array *$_new___int_array(int size) {\n"
                                "    __int_array *arr = (__int_array *) malloc(sizeof(__int_array));\n"
                                "    arr->length = size;\n"
                                "    arr->data = (int *) calloc(size, sizeof(int));\n"
                                "    return arr;\n"
                                "}\n");
}