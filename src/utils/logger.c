#include "utils.h"

static FILE *log_file = NULL;
static log_level_t min_level = LOG_INFO;

void log_init(const char *filename) {
    if (log_file) {
        fclose(log_file);
    }
    if (filename) {
        log_file = fopen(filename, "a");
    }
}

static void log_write(log_level_t level, const char *format,
                      va_list args) {
    FILE *out = log_file ? log_file : stdout;
    const char *level_str[] = {"DEBUG", "INFO", "WARN", "ERROR"};

    if (level >= min_level) {
        fprintf(out, "[%s] ", level_str[level]);
        vfprintf(out, format, args);
        fprintf(out, "\n");
        fflush(out);
    }
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_write(LOG_INFO, format, args);
    va_end(args);
}

void log_warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_write(LOG_WARN, format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_write(LOG_ERROR, format, args);
    va_end(args);
}

void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_write(LOG_DEBUG, format, args);
    va_end(args);
}
