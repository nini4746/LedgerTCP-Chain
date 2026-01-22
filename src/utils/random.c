#include "utils.h"

void random_init(void)
{
	srand((unsigned int)time(NULL));
}

double random_double(void)
{
	return (double)rand() / (double)RAND_MAX;
}
