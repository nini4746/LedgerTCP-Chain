#include "ledger.h"
#include <stdio.h>

static int write_accounts(const ledger_t *ledger, FILE *f)
{
	if (fwrite(&ledger->account_count, sizeof(size_t), 1, f) != 1)
		return -1;
	if (ledger->account_count > 0)
	{
		if (fwrite(ledger->accounts, sizeof(account_t), ledger->account_count, f) != ledger->account_count)
			return -1;
	}
	return 0;
}

static int write_history(const ledger_t *ledger, FILE *f)
{
	size_t tx_count;

	tx_count = ledger->history->tx_count;
	if (fwrite(&tx_count, sizeof(size_t), 1, f) != 1)
		return -1;
	if (tx_count > 0)
	{
		if (fwrite(ledger->history->tx_ids, sizeof(tx_id_t), tx_count, f) != tx_count)
			return -1;
	}
	return 0;
}

int state_save(const ledger_t *ledger, const char *filename)
{
	FILE *f;

	if (!ledger || !filename)
		return -1;
	f = fopen(filename, "wb");
	if (!f)
		return -1;
	if (write_accounts(ledger, f) != 0 || write_history(ledger, f) != 0)
	{
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}
