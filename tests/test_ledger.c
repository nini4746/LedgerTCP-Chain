#include "ledger.h"
#include <stdio.h>

int main(void) {
    int failed = 0;

    ledger_t *ledger = ledger_create();
    if (!ledger) { printf("FAIL: Ledger creation\n"); return 1; }

    if (account_set_balance(ledger, 1, 1000) != 0) { printf("FAIL: Set balance\n"); failed = 1; }
    if (account_get_balance(ledger, 1) != 1000) { printf("FAIL: Get balance\n"); failed = 1; }

    transaction_t tx1 = {1, 1, 2, 100};
    if (transaction_apply(ledger, &tx1) != 0) { printf("FAIL: Apply transaction\n"); failed = 1; }
    if (account_get_balance(ledger, 1) != 900) { printf("FAIL: Sender balance\n"); failed = 1; }
    if (account_get_balance(ledger, 2) != 100) { printf("FAIL: Receiver balance\n"); failed = 1; }

    if (transaction_apply(ledger, &tx1) != 0) { printf("FAIL: Idempotency\n"); failed = 1; }
    if (account_get_balance(ledger, 1) != 900) { printf("FAIL: Balance after duplicate\n"); failed = 1; }

    transaction_t tx2 = {2, 1, 2, 10000};
    if (transaction_apply(ledger, &tx2) == 0) { printf("FAIL: Insufficient balance check\n"); failed = 1; }

    transaction_t tx3 = {3, 1, 2, -100};
    if (transaction_validate(&tx3)) { printf("FAIL: Negative amount check\n"); failed = 1; }

    ledger_t *clone = ledger_clone(ledger);
    if (!clone || account_get_balance(clone, 1) != 900) { printf("FAIL: Clone\n"); failed = 1; }
    if (clone) ledger_destroy(clone);

    if (!ledger_validate(ledger)) { printf("FAIL: Validation\n"); failed = 1; }

    ledger_destroy(ledger);
    if (failed) return 1;
    return 0;
}
