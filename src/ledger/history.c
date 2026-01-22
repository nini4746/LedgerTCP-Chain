#include "ledger.h"

tx_history_t *history_create(void)
{
	tx_history_t *history;

	history = malloc(sizeof(tx_history_t));
	if (!history)
		return NULL;
	history->tx_count = 0;
	memset(history->tx_ids, 0, sizeof(history->tx_ids));
	return history;
}

void history_destroy(tx_history_t *history)
{
	if (history)
		free(history);
}

int history_add(tx_history_t *history, tx_id_t tx_id)
{
	if (!history)
		return -1;
	if (history->tx_count >= MAX_TX_HISTORY)
		return -1;
	history->tx_ids[history->tx_count++] = tx_id;
	return 0;
}

bool history_contains(tx_history_t *history, tx_id_t tx_id)
{
	size_t i;

	if (!history)
		return false;
	for (i = 0; i < history->tx_count; i++)
	{
		if (history->tx_ids[i] == tx_id)
			return true;
	}
	return false;
}

size_t history_get_count(const tx_history_t *history)
{
	if (!history)
		return 0;
	return history->tx_count;
}
