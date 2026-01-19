#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

void random_init(void);
double random_double(void);
int random_int(int min, int max);
bool random_bool(void);
int random_choice(int n);

void *safe_malloc(size_t size);
void safe_free(void *ptr);
int memory_check(void);
void memory_stats(void);
void memory_cleanup(void);

uint64_t time_current(void);
uint64_t time_elapsed(uint64_t start);
uint64_t time_diff(uint64_t start, uint64_t end);
const char *time_to_string(uint64_t time);
void time_sleep(uint64_t ms);

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

void log_init(const char *filename);
void log_info(const char *format, ...);
void log_warn(const char *format, ...);
void log_error(const char *format, ...);
void log_debug(const char *format, ...);

typedef enum {
    ERR_NONE = 0,
    ERR_MEMORY,
    ERR_INVALID_PARAM,
    ERR_NOT_FOUND,
    ERR_NETWORK,
    ERR_CONSENSUS
} error_code_t;

void error_set(error_code_t code, const char *message);
error_code_t error_get(void);
void error_clear(void);
const char *error_to_string(error_code_t code);
bool error_is_fatal(error_code_t code);

#endif
