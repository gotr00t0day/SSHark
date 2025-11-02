#pragma once
#include <string>
#include <vector>

std::string expandPath(const std::string& path);
int checkPaths(const std::string& path);
std::vector<std::string> parseDependencies(const std::string& file, const std::string& key);
std::vector<std::string> parsePaths(const std::string& file, const std::string& key);
