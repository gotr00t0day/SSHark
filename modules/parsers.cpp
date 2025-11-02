#include <filesystem>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <vector>

// filesystem doesn't interpret ~ so you need to expand it yourself
std::string expandPath(const std::string& path) {
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);  // replace ~ with $HOME
        }
    }
    return path;
}

// Check if paths exists
int checkPaths(const std::string& path) {
    std::string resolved = expandPath(path);
    if (std::filesystem::exists(std::filesystem::path(resolved))) {
        return 1;
    }
    return -1;
}

std::vector<std::string> parseDependencies(const std::string& file, const std::string& key) {
    std::ifstream in(file);
    std::string line;
    std::vector<std::string> deps;

    while (std::getline(in, line)) {
        if (line.find(key) != std::string::npos) {
            auto pos = line.find("=");
            if (pos != std::string::npos) {
                std::string list = line.substr(pos + 1);
                std::stringstream ss(list);
                std::string cmd;
                while (std::getline(ss, cmd, ',')) {
                    cmd.erase(0, cmd.find_first_not_of(" \t"));
                    cmd.erase(cmd.find_last_not_of(" \t") + 1);
                    deps.push_back(cmd);                
                }
            }
        }
    }
    return deps;
}

std::vector<std::string> parsePaths(const std::string& file, const std::string& key) {
    std::ifstream in(file);
    std::string line;
    std::vector<std::string> paths;

    while (std::getline(in, line)) {
        if (line.find(key) != std::string::npos) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::stringstream ss(line.substr(pos + 1));
                std::string path;
                while (std::getline(ss, path, ',')) {
                    path.erase(0, path.find_first_not_of(" \t"));
                    path.erase(path.find_last_not_of(" \t") + 1);
                    paths.push_back(path);
                }
            }
        }
    }

    return paths;
}