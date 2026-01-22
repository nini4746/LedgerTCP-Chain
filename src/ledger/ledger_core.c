#include "ledger.h"

ledger_t *ledger_create(void)
{
	ledger_t *ledger;

	ledger = malloc(sizeof(ledger_t));
	if (!ledger)
	{
		return NULL;
	}
	ledger->account_count = 0;
	memset(ledger->accounts, 0, sizeof(ledger->accounts));
	ledger->history = history_create();
	if (!ledger->history)
	{
		free(ledger);
		return NULL;
	}
	return ledger;
}

void ledger_destroy(ledger_t *ledger)
{
	if (!ledger)
	{
		return;
	}
	history_destroy(ledger->history);
	free(ledger);
}

ledger_t *ledger_clone(const ledger_t *ledger)
{
	ledger_t *clone;

	if (!ledger)
	{
		return NULL;
	}
	clone = ledger_create();
	if (!clone)
	{
		return NULL;
	}
	memcpy(clone->accounts, ledger->accounts, sizeof(account_t) * ledger->account_count);
	clone->account_count = ledger->account_count;
	memcpy(clone->history->tx_ids, ledger->history->tx_ids, sizeof(tx_id_t) * ledger->history->tx_count);
	clone->history->tx_count = ledger->history->tx_count;
	return clone;
}

void ledger_clear(ledger_t *ledger)
{
	if (!ledger)
	{
		return;
	}
	ledger->account_count = 0;
	memset(ledger->accounts, 0, sizeof(ledger->accounts));
	ledger->history->tx_count = 0;
	memset(ledger->history->tx_ids, 0, sizeof(ledger->history->tx_ids));
}

bool ledger_validate(const ledger_t *ledger)
{
	size_t i;

	if (!ledger)
	{
		return false;
	}
	if (!ledger->history)
	{
		return false;
	}
	for (i = 0; i < ledger->account_count; i++)
	{
		if (ledger->accounts[i].balance < 0)
		{
			return false;
		}
	}
	return true;
}
