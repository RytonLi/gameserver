#pragma once

#define LOG_DEBUG(msg, ...) Log(0, __LINE__, __FILE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Log(1, __LINE__, __FILE__, msg, ##__VA_ARGS__)

void Log(int level, int lineno, const char* filename, const char *msg, ...);