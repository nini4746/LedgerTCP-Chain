#include "ledger.h"
#include <stdio.h>

static int restore_accounts(ledger_t *ledger, FILE *f)
{
	if (fread(&ledger->account_count, sizeof(size_t), 1, f) != 1)
		return -1;
	if (ledger->account_count > MAX_ACCOUNTS)
		return -1;
	if (ledger->account_count > 0)
	{
		if (fread(ledger->accounts, sizeof(account_t), ledger->account_count, f) != ledger->account_count)
			return -1;
	}
	return 0;
}

static int restore_history(ledger_t *ledger, FILE *f)
{
	if (fread(&ledger->history->tx_count, sizeof(size_t), 1, f) != 1)
		return -1;
	if (ledger->history->tx_count > MAX_TX_HISTORY)
		return -1;
	if (ledger->history->tx_count > 0)
	{
		if (fread(ledger->history->tx_ids, sizeof(tx_id_t), ledger->history->tx_count, f) != ledger->history->tx_count)
			return -1;
	}
	return 0;
}

ledger_t *state_restore(const char *filename)
{
	FILE *f;
	ledger_t *ledger;

	if (!filename)
		return NULL;
	f = fopen(filename, "rb");
	if (!f)
		return NULL;
	ledger = ledger_create();
	if (!ledger)
	{
		fclose(f);
		return NULL;
	}
	if (restore_accounts(ledger, f) != 0 || restore_history(ledger, f) != 0)
	{
		fclose(f);
		ledger_destroy(ledger);
		return NULL;
	}
	fclose(f);
	return ledger;
}
