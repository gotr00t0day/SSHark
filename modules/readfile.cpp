#include "readfile.h"
#include <fstream>
#include <sstream>

std::string readFile(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs) return {};
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}