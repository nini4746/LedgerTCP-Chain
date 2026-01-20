#include "ledger.h"
#include <stdio.h>

int state_save(const ledger_t *ledger, const char *filename) {
    if (!ledger || !filename) return -1;

    FILE *f = fopen(filename, "wb");
    if (!f) return -1;

    if (fwrite(&ledger->account_count, sizeof(size_t), 1, f) != 1) goto error;
    if (ledger->account_count > 0) {
        if (fwrite(ledger->accounts, sizeof(account_t),
                   ledger->account_count, f) != ledger->account_count) goto error;
    }

    size_t tx_count = ledger->history->tx_count;
    if (fwrite(&tx_count, sizeof(size_t), 1, f) != 1) goto error;
    if (tx_count > 0) {
        if (fwrite(ledger->history->tx_ids, sizeof(tx_id_t),
                   tx_count, f) != tx_count) goto error;
    }

    fclose(f);
    return 0;

error:
    fclose(f);
    return -1;
}

static int restore_accounts(ledger_t *ledger, FILE *f) {
    if (fread(&ledger->account_count, sizeof(size_t), 1, f) != 1) return -1;
    if (ledger->account_count > MAX_ACCOUNTS) return -1;
    if (ledger->account_count > 0 &&
        fread(ledger->accounts, sizeof(account_t),
              ledger->account_count, f) != ledger->account_count) return -1;
    return 0;
}

static int restore_history(ledger_t *ledger, FILE *f) {
    if (fread(&ledger->history->tx_count, sizeof(size_t), 1, f) != 1) return -1;
    if (ledger->history->tx_count > MAX_TX_HISTORY) return -1;
    if (ledger->history->tx_count > 0 &&
        fread(ledger->history->tx_ids, sizeof(tx_id_t),
              ledger->history->tx_count, f) != ledger->history->tx_count) return -1;
    return 0;
}

ledger_t *state_restore(const char *filename) {
    if (!filename) return NULL;

    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    ledger_t *ledger = ledger_create();
    if (!ledger) { fclose(f); return NULL; }

    if (restore_accounts(ledger, f) != 0 || restore_history(ledger, f) != 0) {
        fclose(f);
        ledger_destroy(ledger);
        return NULL;
    }

    fclose(f);
    return ledger;
}

bool state_compare(const ledger_t *a, const ledger_t *b) {
    if (!a || !b) return false;
    if (a->account_count != b->account_count) return false;

    for (size_t i = 0; i < a->account_count; i++) {
        if (a->accounts[i].account_id != b->accounts[i].account_id)
            return false;
        if (a->accounts[i].balance != b->accounts[i].balance)
            return false;
    }
    return true;
}

uint64_t state_hash(const ledger_t *ledger) {
    if (!ledger) return 0;

    uint64_t hash = 5381;

    for (size_t i = 0; i < ledger->account_count; i++) {
        hash = ((hash << 5) + hash) + ledger->accounts[i].account_id;
        hash = ((hash << 5) + hash) + (uint64_t)ledger->accounts[i].balance;
    }

    return hash;
}

int state_export(const ledger_t *ledger, const char *filename) {
    if (!ledger || !filename) return -1;

    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    fprintf(f, "Ledger State Export\n");
    fprintf(f, "===================\n\n");

    for (size_t i = 0; i < ledger->account_count; i++) {
        fprintf(f, "Account %lu: %ld\n",
                ledger->accounts[i].account_id,
                ledger->accounts[i].balance);
    }

    fclose(f);
    return 0;
}
