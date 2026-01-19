#include "utils.h"

static error_code_t last_error = ERR_NONE;
static char error_message[256] = {0};

void error_set(error_code_t code, const char *message) {
    last_error = code;
    if (message) {
        strncpy(error_message, message, sizeof(error_message) - 1);
        error_message[sizeof(error_message) - 1] = '\0';
    } else {
        error_message[0] = '\0';
    }
}

error_code_t error_get(void) {
    return last_error;
}

void error_clear(void) {
    last_error = ERR_NONE;
    error_message[0] = '\0';
}

const char *error_to_string(error_code_t code) {
    switch (code) {
        case ERR_NONE: return "No error";
        case ERR_MEMORY: return "Memory error";
        case ERR_INVALID_PARAM: return "Invalid parameter";
        case ERR_NOT_FOUND: return "Not found";
        case ERR_NETWORK: return "Network error";
        case ERR_CONSENSUS: return "Consensus error";
        default: return "Unknown error";
    }
}

bool error_is_fatal(error_code_t code) {
    return code == ERR_MEMORY || code == ERR_CONSENSUS;
}
