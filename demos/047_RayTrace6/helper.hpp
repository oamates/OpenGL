// Helper functions

#ifndef HELPER_COMMON
#define HELPER_COMMON

#include <string>
#include <vector>

// Function that replaces all the strings by Base found in the string byReplace
std::string replaceAll(const std::string& parString, const std::string& parBase, const std::string& parReplace);

//Function that splits the string byString with parSeparator and returns a vector of the resulting strings
std::vector<std::string> split(const std::string& parString, char parSeparator);

// Function that convert a string to int
int convertToInt(const std::string& parToConvert);

void CheckGLState(const std::string& desc);

std::string convertToString(int parToConvert);

#endif // HELPER_COMMON
