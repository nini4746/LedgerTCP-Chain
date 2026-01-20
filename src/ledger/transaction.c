#include "ledger.h"

transaction_t *transaction_create(tx_id_t tx_id, account_id_t from,
                                  account_id_t to, balance_t amount) {
    transaction_t *tx = malloc(sizeof(transaction_t));
    if (!tx) return NULL;

    tx->tx_id = tx_id;
    tx->from = from;
    tx->to = to;
    tx->amount = amount;
    return tx;
}

bool transaction_validate(const transaction_t *tx) {
    if (!tx) return false;
    if (tx->amount < 0) return false;
    if (tx->from == tx->to) return false;
    return true;
}

int transaction_apply(ledger_t *ledger, const transaction_t *tx) {
    if (!ledger || !tx) return -1;
    if (!transaction_validate(tx)) return -1;

    if (history_contains(ledger->history, tx->tx_id)) {
        return 0;
    }

    account_t *from = account_get_or_create(ledger, tx->from);
    account_t *to = account_get_or_create(ledger, tx->to);

    if (!from || !to) return -1;
    if (from->balance < tx->amount) return -1;

    from->balance -= tx->amount;
    to->balance += tx->amount;

    return history_add(ledger->history, tx->tx_id);
}

int transaction_revert(ledger_t *ledger, const transaction_t *tx) {
    if (!ledger || !tx) return -1;

    account_t *from = account_find(ledger, tx->from);
    account_t *to = account_find(ledger, tx->to);

    if (!from || !to) return -1;
    if (to->balance < tx->amount) return -1;

    to->balance -= tx->amount;
    from->balance += tx->amount;
    return 0;
}

size_t transaction_serialize(const transaction_t *tx, void *buffer,
                             size_t buffer_size) {
    if (!tx || !buffer) return 0;

    size_t needed = sizeof(transaction_t);
    if (buffer_size < needed) return 0;

    memcpy(buffer, tx, needed);
    return needed;
}
