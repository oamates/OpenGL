#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

void printErrorsGL(const char *func, int line);

#define logError(...) {printf("Error (%s:%i): ", __FUNCTION__, __LINE__);printf(__VA_ARGS__);printf("\n");}
#define logWarning(...) {printf("Warning (%s:%i): ", __FUNCTION__, __LINE__);printf(__VA_ARGS__);printf("\n");}
#define logNote(...) {printf("Note: ");printf(__VA_ARGS__);printf("\n");}
#define logErrorsGL() printErrorsGL(__FUNCTION__, __LINE__)
