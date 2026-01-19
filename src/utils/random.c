#include "utils.h"

static bool initialized = false;

void random_init(void) {
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = true;
    }
}

double random_double(void) {
    if (!initialized) random_init();
    return (double)rand() / (double)RAND_MAX;
}

int random_int(int min, int max) {
    if (!initialized) random_init();
    if (min > max) return min;
    return min + (rand() % (max - min + 1));
}

bool random_bool(void) {
    if (!initialized) random_init();
    return (rand() % 2) == 1;
}

int random_choice(int n) {
    if (!initialized) random_init();
    if (n <= 0) return 0;
    return rand() % n;
}
