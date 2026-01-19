#include "ledger.h"
#include <stdio.h>

int state_save(const ledger_t *ledger, const char *filename) {
    if (!ledger || !filename) return -1;

    FILE *f = fopen(filename, "wb");
    if (!f) return -1;

    fwrite(&ledger->account_count, sizeof(size_t), 1, f);
    fwrite(ledger->accounts, sizeof(account_t),
           ledger->account_count, f);

    size_t tx_count = ledger->history->tx_count;
    fwrite(&tx_count, sizeof(size_t), 1, f);
    fwrite(ledger->history->tx_ids, sizeof(tx_id_t), tx_count, f);

    fclose(f);
    return 0;
}

ledger_t *state_restore(const char *filename) {
    if (!filename) return NULL;

    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    ledger_t *ledger = ledger_create();
    if (!ledger) {
        fclose(f);
        return NULL;
    }

    fread(&ledger->account_count, sizeof(size_t), 1, f);
    fread(ledger->accounts, sizeof(account_t),
          ledger->account_count, f);

    fread(&ledger->history->tx_count, sizeof(size_t), 1, f);
    fread(ledger->history->tx_ids, sizeof(tx_id_t),
          ledger->history->tx_count, f);

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
