#include "block.h"

bool hash_is_zero(const hash_t hash)
{
	uint8_t zero[HASH_SIZE];

	memset(zero, 0, HASH_SIZE);
	return memcmp(hash, zero, HASH_SIZE) == 0;
}

uint32_t hash_compute_simple(const void *data, size_t len)
{
	const uint8_t *bytes;
	uint32_t hash;
	size_t i;

	bytes = (const uint8_t *)data;
	hash = 5381;
	for (i = 0; i < len; i++)
		hash = ((hash << 5) + hash) + bytes[i];
	return hash;
}
