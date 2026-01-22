#include "ledger.h"

bool state_compare(const ledger_t *a, const ledger_t *b)
{
	size_t i;

	if (!a || !b)
		return false;
	if (a->account_count != b->account_count)
		return false;
	for (i = 0; i < a->account_count; i++)
	{
		if (a->accounts[i].account_id != b->accounts[i].account_id)
			return false;
		if (a->accounts[i].balance != b->accounts[i].balance)
			return false;
	}
	return true;
}

uint64_t state_hash(const ledger_t *ledger)
{
	uint64_t hash;
	size_t i;

	if (!ledger)
		return 0;
	hash = 5381;
	for (i = 0; i < ledger->account_count; i++)
	{
		hash = ((hash << 5) + hash) + ledger->accounts[i].account_id;
		hash = ((hash << 5) + hash) + (uint64_t)ledger->accounts[i].balance;
	}
	return hash;
}
