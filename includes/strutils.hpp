#ifndef UTILS_HPP_INCLUDE
#define UTILS_HPP_INCLUDE

#include <string>
#include <vector>

namespace strutils {

void trimLeft(std::string& str);
void trimRight(std::string& str);
void trim(std::string& str);

std::vector<std::string> split(const std::string& str, char delim);
std::vector<std::string> split(const std::string& str, const std::string& delim);

}

#endif
