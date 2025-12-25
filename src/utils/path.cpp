#include "path.h"
#include <sstream>
#include <string>
#include <stdexcept>

int PathUtils::getIdFromPath(const std::string &path) {
    std::stringstream ss(path);
    std::string segment;
    std::string last;
    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) last = segment;
    }
    return std::stoi(last);
}