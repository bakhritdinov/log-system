#ifndef LOG_COLLECTOR_INTERNAL_H
#define LOG_COLLECTOR_INTERNAL_H

#define C_RESET "\x1b[0m"
#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_CYAN "\x1b[36m"
#define C_MAGENTA "\x1b[35m"

const char* get_color(const char* line);
void setup_console();

#endif