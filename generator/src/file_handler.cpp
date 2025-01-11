#include "../internal/generator_internal.h"
#include <filesystem>
#include <fstream>

/**
 * @brief Creates or overwrites a file with the given content inside the "compile" directory.
 *
 * This function ensures that all generated files are written into a dedicated `compile` directory,
 * isolating them from other parts of the filesystem. It handles:
 * - Creating the `compile` directory if it does not already exist.
 * - Writing the provided content (`source`) into the specified file (`fileName`).
 * - Catching and reporting any filesystem-related errors during directory creation or file writing.
 *
 * **Parameters**:
 * - `fileName`: The name of the file to create or overwrite (e.g., "output.c").
 * - `source`: The content to write into the file.
 *
 * **Generated Directory**:
 * - All files are stored in a directory named `compile`, created in the current working directory.
 * - Example layout:
 *   ```
 *   compile/
 *   ├── classA.c
 *   ├── classA.h
 *   ├── CMakeLists.txt
 *   ```
 *
 * **Behavior**:
 * - If the `compile` directory does not already exist, it is created.
 * - If the specified file exists, it is overwritten.
 *
 * **Error Handling**:
 * - Uses `std::filesystem::create_directory()` to ensure the `compile` directory exists.
 * - If the directory or file cannot be created, a descriptive error message is printed to `std::cerr`.
 *
 * **Example Usage**:
 * ```cpp
 * write_file("example.c", "#include <stdio.h>\n\nint main() {\n    return 0;\n}");
 * ```
 * Result:
 * - Creates `compile/example.c` with the provided source code.
 *
 * **Example Output** (`compile/example.c`):
 * ```c
 * #include <stdio.h>
 *
 * int main() {
 *     return 0;
 * }
 * ```
 */
void write_file(const std::string &fileName, const std::string &source) {
    std::string dirName = "compile";
    try {
        std::filesystem::create_directory(dirName);
        std::ofstream outFile(dirName + "/" + fileName);

        if (outFile.is_open()) {
            outFile << source;
            outFile.close();
        } else {
            std::cerr << "Error: Unable to create or open the file." << std::endl;
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "FileSystem error: " << e.what() << std::endl;
    }
}