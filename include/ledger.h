#ifndef LEDGER_H
#define LEDGER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 1000
#define MAX_TX_HISTORY 10000

typedef uint64_t tx_id_t;
typedef uint64_t account_id_t;
typedef int64_t balance_t;

typedef struct {
    tx_id_t tx_id;
    account_id_t from;
    account_id_t to;
    balance_t amount;
} transaction_t;

typedef struct {
    account_id_t account_id;
    balance_t balance;
} account_t;

typedef struct {
    tx_id_t tx_ids[MAX_TX_HISTORY];
    size_t tx_count;
} tx_history_t;

typedef struct {
    account_t accounts[MAX_ACCOUNTS];
    size_t account_count;
    tx_history_t *history;
} ledger_t;

ledger_t *ledger_create(void);
void ledger_destroy(ledger_t *ledger);
ledger_t *ledger_clone(const ledger_t *ledger);
void ledger_clear(ledger_t *ledger);
bool ledger_validate(const ledger_t *ledger);

account_t *account_create(account_id_t account_id);
account_t *account_find(ledger_t *ledger, account_id_t account_id);
balance_t account_get_balance(ledger_t *ledger, account_id_t account_id);
int account_set_balance(ledger_t *ledger, account_id_t account_id, balance_t balance);
account_t *account_get_or_create(ledger_t *ledger, account_id_t account_id);

transaction_t *transaction_create(tx_id_t tx_id, account_id_t from, account_id_t to, balance_t amount);
bool transaction_validate(const transaction_t *tx);
int transaction_apply(ledger_t *ledger, const transaction_t *tx);
int transaction_revert(ledger_t *ledger, const transaction_t *tx);
size_t transaction_serialize(const transaction_t *tx, void *buffer, size_t buffer_size);

tx_history_t *history_create(void);
void history_destroy(tx_history_t *history);
int history_add(tx_history_t *history, tx_id_t tx_id);
bool history_contains(tx_history_t *history, tx_id_t tx_id);
size_t history_get_count(const tx_history_t *history);

int state_save(const ledger_t *ledger, const char *filename);
ledger_t *state_restore(const char *filename);
bool state_compare(const ledger_t *a, const ledger_t *b);
uint64_t state_hash(const ledger_t *ledger);
int state_export(const ledger_t *ledger, const char *filename);

#endif
