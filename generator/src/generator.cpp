#include "../internal/generator_internal.h"

void generate(Project *project) {
    for (auto &clazz: *project->getClasses()) {
        std::map<Identifier, bool> included;
        generate_class_header(project, &clazz, included);
        generate_class_source(project, &clazz, included);
    }

    write_cmake();
    write_int_array();
}
