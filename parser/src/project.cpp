#include "../include/project.h"

#include <queue>
#include <set>

void Project::addClass(Class &clazz) {
    unsigned long index = classes.size();
    classes.push_back(std::move(clazz));
    classesMap.insert({classes[index].getName(), index});
}

Class *Project::getClassByName(const Identifier& name) {
    return &classes[classesMap[name]];
}

std::vector<Class> *Project::getClasses() {
    return &classes;
}

bool Project::containsClass(const Identifier &className) {
    return classesMap.contains(className);
}

std::vector<Identifier> Project::getTopologicalSort() {
    std::map<Identifier, std::set<Identifier>> adjList;
    std::map<Identifier, int> inDegree;

    for (const auto &[className, classNode]: classesMap) {
        adjList[className] = {};
        inDegree[className] = 0;
    }

    for (const auto &[className, classIndex]: classesMap) {
        auto &clazz = classes[classIndex];
        if (!clazz.getExtends().empty()) {
            if (!classesMap.contains(clazz.getExtends())) {
                error("Class '" + clazz.getExtends() + "' not found");
            }

            adjList[clazz.getExtends()].insert(className);
            inDegree[className]++;
        }
    }

    std::queue<Identifier> zeroInDegree;
    for (const auto &[className, degree]: inDegree) {
        if (degree == 0) {
            zeroInDegree.push(className);
        }
    }

    std::vector<Identifier> sortedClasses;

    while (!zeroInDegree.empty()) {
        Identifier current = zeroInDegree.front();
        zeroInDegree.pop();
        sortedClasses.push_back(current);

        for (const auto &dependent: adjList[current]) {
            inDegree[dependent]--;
            if (inDegree[dependent] == 0) {
                zeroInDegree.push(dependent);
            }
        }
    }

    if (sortedClasses.size() != classes.size()) {
        error("Cyclic inheritance detected");
    }
    return sortedClasses;
}