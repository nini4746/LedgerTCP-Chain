#include "ledger.h"

account_t *account_create(account_id_t account_id) {
    account_t *account = malloc(sizeof(account_t));
    if (!account) return NULL;

    account->account_id = account_id;
    account->balance = 0;
    return account;
}

account_t *account_find(ledger_t *ledger, account_id_t account_id) {
    if (!ledger) return NULL;

    for (size_t i = 0; i < ledger->account_count; i++) {
        if (ledger->accounts[i].account_id == account_id) {
            return &ledger->accounts[i];
        }
    }
    return NULL;
}

balance_t account_get_balance(ledger_t *ledger, account_id_t account_id) {
    if (!ledger) return 0;

    account_t *account = account_find(ledger, account_id);
    return account ? account->balance : 0;
}

int account_set_balance(ledger_t *ledger, account_id_t account_id,
                        balance_t balance) {
    if (!ledger || balance < 0) return -1;

    account_t *account = account_get_or_create(ledger, account_id);
    if (!account) return -1;

    account->balance = balance;
    return 0;
}

account_t *account_get_or_create(ledger_t *ledger,
                                 account_id_t account_id) {
    if (!ledger) return NULL;

    account_t *account = account_find(ledger, account_id);
    if (account) return account;

    if (ledger->account_count >= MAX_ACCOUNTS) return NULL;

    account = &ledger->accounts[ledger->account_count++];
    account->account_id = account_id;
    account->balance = 0;
    return account;
}
