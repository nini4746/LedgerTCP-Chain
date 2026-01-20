#!/bin/bash

# LedgerTCP-Chain Comprehensive Test Suite

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASSED=0
FAILED=0
TOTAL=0

test_result() {
    local name="$1"
    local result="$2"
    TOTAL=$((TOTAL + 1))

    if [ "$result" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}[FAIL]${NC} $name"
        FAILED=$((FAILED + 1))
    fi
}

echo "======================================"
echo "LedgerTCP-Chain Test Suite"
echo "======================================"
echo ""

# Clean and build
echo -e "${YELLOW}Building project...${NC}"
make clean > /dev/null 2>&1
if ! make 2>&1 | grep -q "Build complete"; then
    echo -e "${RED}Build failed!${NC}"
    make
    exit 1
fi
echo -e "${GREEN}Build successful${NC}"
echo ""

# Create test directories
TEST_DIR="test_results"
TESTS_DIR="tests"
mkdir -p $TEST_DIR $TESTS_DIR
rm -f $TEST_DIR/*.log

echo "======================================"
echo "Test Category 1: Basic Functionality"
echo "======================================"

# Test 1: Basic execution
echo -n "Test 1.1: Basic program execution... "
if ./ledger_tcp_chain > $TEST_DIR/output.log 2>&1; then
    test_result "Basic execution" 0
else
    test_result "Basic execution" 1
    echo "  Output saved to $TEST_DIR/output.log"
fi

# Test 2: Check if consensus is reached
echo -n "Test 1.2: Consensus verification... "
if grep -q "SUCCESS: All nodes reached consensus" $TEST_DIR/output.log; then
    test_result "Consensus reached" 0
else
    test_result "Consensus reached" 1
fi

# Test 3: Memory leak check with valgrind
echo -n "Test 1.3: Memory leak check... "
if command -v valgrind &> /dev/null; then
    if valgrind --leak-check=full --error-exitcode=1 ./ledger_tcp_chain > /dev/null 2>&1; then
        test_result "No memory leaks" 0
    else
        test_result "No memory leaks" 1
        valgrind --leak-check=full ./ledger_tcp_chain > $TEST_DIR/valgrind.log 2>&1
        echo "  Valgrind output saved to $TEST_DIR/valgrind.log"
    fi
else
    echo -e "${YELLOW}  Valgrind not installed, skipping${NC}"
fi

echo ""
echo "======================================"
echo "Test Category 2: Unit Tests"
echo "======================================"

# Create unit test programs
cat > $TESTS_DIR/test_ledger.c << 'EOF'
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
EOF

cat > $TESTS_DIR/test_block.c << 'EOF'
#include "block.h"
#include <stdio.h>

int main(void) {
    int failed = 0;

    hash_t hash1, hash2;
    hash_zero(hash1);
    if (!hash_is_zero(hash1)) { printf("FAIL: Hash zero\n"); failed = 1; }

    hash_copy(hash2, hash1);
    if (!hash_equals(hash1, hash2)) { printf("FAIL: Hash copy\n"); failed = 1; }

    block_t *genesis = genesis_create();
    if (!genesis || !genesis_validate(genesis) || genesis->height != 0) {
        printf("FAIL: Genesis\n"); failed = 1;
        if (genesis) block_destroy(genesis);
        return 1;
    }

    block_t *block1 = block_create(genesis->hash, 1);
    if (!block1) { printf("FAIL: Block creation\n"); failed = 1; block_destroy(genesis); return 1; }

    transaction_t tx = {1, 1, 2, 100};
    if (block_add_transaction(block1, &tx) != 0 || block1->tx_count != 1) {
        printf("FAIL: Add transaction\n"); failed = 1;
    }

    block_compute_hash(block1);
    if (hash_is_zero(block1->hash)) { printf("FAIL: Hash compute\n"); failed = 1; }

    if (!validate_block_structure(block1)) { printf("FAIL: Block structure\n"); failed = 1; }
    if (!validate_prev_hash(block1, genesis)) { printf("FAIL: Prev hash\n"); failed = 1; }

    blockchain_t *chain = chain_create();
    if (!chain) { printf("FAIL: Chain creation\n"); failed = 1; }

    if (chain_add_block(chain, genesis) != 0) { printf("FAIL: Add genesis\n"); failed = 1; }
    if (chain_add_block(chain, block1) != 0) { printf("FAIL: Add block\n"); failed = 1; }
    if (chain_get_length(chain) != 2) { printf("FAIL: Chain length\n"); failed = 1; }

    block_t *found = blockchain_get_block_by_hash(chain, block1->hash);
    if (!found || found != block1) { printf("FAIL: Find by hash\n"); failed = 1; }

    chain_destroy(chain);
    if (failed) return 1;
    return 0;
}
EOF

cat > $TESTS_DIR/test_tcp.c << 'EOF'
#include "tcp_simulator.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int failed = 0;

    tcp_simulator_t *sim = tcp_simulator_create(3);
    if (!sim) { printf("FAIL: Simulator creation\n"); return 1; }

    if (tcp_simulator_create(0) != NULL) { printf("FAIL: Should reject 0 nodes\n"); failed = 1; }
    if (tcp_simulator_create(MAX_NODES + 1) != NULL) { printf("FAIL: Should reject too many nodes\n"); failed = 1; }

    tcp_simulator_set_probabilities(sim, 0.5, 0.3, 0.2, 0.1);

    char data[] = "test";
    if (network_send(sim, 0, 1, data, strlen(data) + 1) != 0) { printf("FAIL: Send\n"); failed = 1; }
    if (network_send(sim, 0, 10, data, strlen(data)) == 0) { printf("FAIL: Invalid node check\n"); failed = 1; }

    for (int i = 0; i < 100; i++) tcp_simulator_step(sim);

    char buffer[256];
    int received = network_receive(sim, 1, buffer, sizeof(buffer));
    if (received < 0) { printf("FAIL: Receive error\n"); failed = 1; }

    packet_queue_t *queue = queue_create();
    if (!queue || !queue_is_empty(queue)) { printf("FAIL: Queue\n"); failed = 1; }

    packet_t *pkt = packet_create(0, 1, data, strlen(data) + 1, 0);
    if (!pkt) { printf("FAIL: Packet creation\n"); failed = 1; }

    queue_enqueue(queue, pkt);
    if (queue_is_empty(queue)) { printf("FAIL: Queue empty after enqueue\n"); failed = 1; }

    packet_t *dequeued = queue_dequeue(queue);
    if (!dequeued) { printf("FAIL: Dequeue\n"); failed = 1; }
    else packet_destroy(dequeued);

    queue_destroy(queue);
    tcp_simulator_destroy(sim);

    if (failed) return 1;
    return 0;
}
EOF

echo -n "Test 2.1: Ledger unit tests... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_ledger.c \
    obj/ledger/*.o -o $TESTS_DIR/test_ledger 2>&1 | grep -q error; then
    test_result "Ledger unit tests (compile)" 1
    gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_ledger.c obj/ledger/*.o -o $TESTS_DIR/test_ledger
elif ! $TESTS_DIR/test_ledger 2>&1; then
    test_result "Ledger unit tests" 1
    $TESTS_DIR/test_ledger
else
    test_result "Ledger unit tests" 0
fi

echo -n "Test 2.2: Block unit tests... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_block.c \
    obj/block/*.o obj/ledger/*.o -o $TESTS_DIR/test_block 2>&1 | grep -q error; then
    test_result "Block unit tests (compile)" 1
    gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_block.c obj/block/*.o obj/ledger/*.o -o $TESTS_DIR/test_block
elif ! $TESTS_DIR/test_block 2>&1; then
    test_result "Block unit tests" 1
    $TESTS_DIR/test_block
else
    test_result "Block unit tests" 0
fi

echo -n "Test 2.3: TCP unit tests... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_tcp.c \
    obj/tcp/*.o obj/utils/random.o -o $TESTS_DIR/test_tcp 2>&1 | grep -q error; then
    test_result "TCP unit tests (compile)" 1
    gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_tcp.c obj/tcp/*.o obj/utils/random.o -o $TESTS_DIR/test_tcp
elif ! $TESTS_DIR/test_tcp 2>&1; then
    test_result "TCP unit tests" 1
    $TESTS_DIR/test_tcp
else
    test_result "TCP unit tests" 0
fi

echo ""
echo "======================================"
echo "Test Category 3: Edge Cases"
echo "======================================"

cat > $TESTS_DIR/test_edge.c << 'EOF'
#include "ledger.h"
#include "block.h"
#include "tcp_simulator.h"
#include <stdio.h>
#include <limits.h>

int main(void) {
    int failed = 0;

    // NULL pointer tests
    if (ledger_validate(NULL)) { printf("FAIL: NULL ledger\n"); failed = 1; }
    if (block_validate_structure(NULL)) { printf("FAIL: NULL block\n"); failed = 1; }
    if (chain_get_length(NULL) != 0) { printf("FAIL: NULL chain length\n"); failed = 1; }

    transaction_t tx = {1, 1, 2, 100};
    if (transaction_apply(NULL, &tx) == 0) { printf("FAIL: NULL ledger in tx\n"); failed = 1; }

    // Zero/invalid value tests
    ledger_t *ledger = ledger_create();
    transaction_t zero_tx = {0, 0, 0, 0};
    if (transaction_validate(&zero_tx)) { printf("FAIL: Zero tx\n"); failed = 1; }

    // Negative amount test
    transaction_t neg_tx = {99, 1, 2, -1000};
    if (transaction_validate(&neg_tx)) { printf("FAIL: Negative amount\n"); failed = 1; }

    // Self-transfer test
    transaction_t self_tx = {100, 1, 1, 100};
    if (transaction_validate(&self_tx)) { printf("FAIL: Self transfer\n"); failed = 1; }

    // Max values test
    transaction_t max_tx = {UINT64_MAX, 1, 2, INT64_MAX};
    account_set_balance(ledger, 1, INT64_MAX);
    if (transaction_apply(ledger, &max_tx) != 0) { printf("FAIL: Max values\n"); failed = 1; }

    // Overflow test - balance + amount > INT64_MAX
    ledger_t *overflow_ledger = ledger_create();
    account_set_balance(overflow_ledger, 1, INT64_MAX);
    account_set_balance(overflow_ledger, 2, INT64_MAX - 100);
    transaction_t overflow_tx = {200, 1, 2, 200};  // Would overflow account 2
    transaction_apply(overflow_ledger, &overflow_tx);  // Should handle gracefully

    // Insufficient balance test
    ledger_t *empty_ledger = ledger_create();
    account_set_balance(empty_ledger, 1, 0);
    transaction_t empty_tx = {201, 1, 2, 100};
    if (transaction_apply(empty_ledger, &empty_tx) == 0) { printf("FAIL: Empty balance\n"); failed = 1; }

    // Non-existent account test
    if (account_get_balance(empty_ledger, 9999) != 0) { printf("FAIL: Non-existent account\n"); failed = 1; }

    // MAX limits tests
    ledger_t *ledger2 = ledger_create();
    for (size_t i = 0; i < MAX_ACCOUNTS + 10; i++) {
        account_set_balance(ledger2, i, 100);
    }

    ledger_t *ledger3 = ledger_create();
    account_set_balance(ledger3, 1, 1000000);
    for (size_t i = 0; i < MAX_TX_HISTORY + 10; i++) {
        transaction_t tx_i = {i, 1, 2, 1};
        transaction_apply(ledger3, &tx_i);
    }

    block_t *block = genesis_create();
    for (size_t i = 0; i < MAX_TX_PER_BLOCK + 10; i++) {
        transaction_t tx_i = {i, 1, 2, 1};
        block_add_transaction(block, &tx_i);
    }

    // Network edge cases
    tcp_simulator_t *sim = tcp_simulator_create(2);

    // Oversized packet
    char large_data[MAX_PACKET_SIZE + 100];
    if (network_send(sim, 0, 1, large_data, sizeof(large_data)) == 0) {
        printf("FAIL: Oversized packet\n"); failed = 1;
    }

    // Zero-size packet
    if (network_send(sim, 0, 1, large_data, 0) == 0) {
        printf("FAIL: Zero size packet\n"); failed = 1;
    }

    // Invalid node IDs
    if (network_send(sim, -1, 1, "test", 4) == 0) { printf("FAIL: Negative src node\n"); failed = 1; }
    if (network_send(sim, 0, 999, "test", 4) == 0) { printf("FAIL: Invalid dst node\n"); failed = 1; }

    // Block with invalid prev_hash
    hash_t fake_hash;
    memset(fake_hash, 0xFF, HASH_SIZE);
    block_t *orphan = block_create(fake_hash, 100);
    blockchain_t *chain = chain_create();
    block_t *genesis = genesis_create();
    chain_add_block(chain, genesis);
    // orphan block should not link properly
    if (validate_prev_hash(orphan, genesis)) { printf("FAIL: Invalid prev_hash accepted\n"); failed = 1; }

    // Empty chain operations
    blockchain_t *empty_chain = chain_create();
    if (chain_get_head(empty_chain) != NULL) { printf("FAIL: Empty chain head\n"); failed = 1; }
    if (blockchain_get_block_by_height(empty_chain, 0) != NULL) { printf("FAIL: Empty chain search\n"); failed = 1; }

    // Cleanup
    ledger_destroy(ledger);
    ledger_destroy(ledger2);
    ledger_destroy(ledger3);
    ledger_destroy(overflow_ledger);
    ledger_destroy(empty_ledger);
    block_destroy(block);
    block_destroy(orphan);
    chain_destroy(chain);
    chain_destroy(empty_chain);
    tcp_simulator_destroy(sim);

    if (failed) return 1;
    return 0;
}
EOF

echo -n "Test 3.1: Edge cases... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_edge.c \
    obj/ledger/*.o obj/block/*.o obj/tcp/*.o obj/utils/random.o \
    -o $TESTS_DIR/test_edge 2>&1 | grep -q error; then
    test_result "Edge cases (compile)" 1
    gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_edge.c obj/ledger/*.o obj/block/*.o obj/tcp/*.o obj/utils/random.o -o $TESTS_DIR/test_edge
elif ! $TESTS_DIR/test_edge 2>&1; then
    test_result "Edge cases" 1
    $TESTS_DIR/test_edge
else
    test_result "Edge cases" 0
fi

echo ""
echo "======================================"
echo "Test Category 4: Stress Tests"
echo "======================================"

cat > $TESTS_DIR/test_stress.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    int failed = 0;
    consensus_init();

    // Test: Process 100 transactions with network issues
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.05, 0.02, 0.02, 0.02);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i, 10000000);
    }

    // Send 100 transactions with interleaved network processing
    for (int i = 0; i < 100; i++) {
        transaction_t tx;
        tx.tx_id = i + 1;
        tx.from = (i % 3) + 1;
        tx.to = ((i + 1) % 3) + 1;
        tx.amount = 1;

        int creator = i % 3;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);

        consensus_process_block(nodes[creator], block);
        sync_broadcast_block(sim, creator, block, 3);

        free(block);

        // Process network packets between blocks
        for (int step = 0; step < 20; step++) {
            tcp_simulator_step(sim);

            for (int j = 0; j < 3; j++) {
                block_t recv_block;
                int received = network_receive(sim, j, &recv_block, sizeof(block_t));
                if (received > 0) {
                    consensus_process_block(nodes[j], &recv_block);
                }
            }
        }
    }

    // Final network processing for remaining packets
    for (int step = 0; step < 1000; step++) {
        tcp_simulator_step(sim);

        for (int i = 0; i < 3; i++) {
            block_t recv_block;
            int received = network_receive(sim, i, &recv_block, sizeof(block_t));
            if (received > 0) {
                consensus_process_block(nodes[i], &recv_block);
            }
        }

        // Periodically sync nodes
        if (step % 200 == 0) {
            for (int i = 0; i < 3; i++) {
                cnode_update_state(nodes[i]);
            }
        }
    }

    sync_finalize(nodes, 3);

    if (!consensus_verify(nodes, 3)) {
        printf("FAIL: Consensus not reached\n");
        for (int i = 0; i < 3; i++) {
            printf("Node %d: chain_length=%zu, balance1=%ld, balance2=%ld, balance3=%ld, forks=%zu\n",
                   i,
                   chain_get_length(nodes[i]->main_chain),
                   account_get_balance(nodes[i]->ledger, 1),
                   account_get_balance(nodes[i]->ledger, 2),
                   account_get_balance(nodes[i]->ledger, 3),
                   nodes[i]->fork_count);
        }
        failed = 1;
    }

    for (int i = 0; i < 3; i++) {
        cnode_destroy(nodes[i]);
    }
    tcp_simulator_destroy(sim);

    consensus_cleanup();

    if (failed) return 1;
    return 0;
}
EOF

echo -n "Test 4.1: Stress tests (100 transactions)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_stress.c \
    obj/*/*.o -o $TESTS_DIR/test_stress 2>&1 | grep -q error; then
    test_result "Stress tests (compile)" 1
    gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_stress.c obj/*/*.o -o $TESTS_DIR/test_stress
elif ! timeout 60s $TESTS_DIR/test_stress 2>&1; then
    test_result "Stress tests" 1
    timeout 60s $TESTS_DIR/test_stress
else
    test_result "Stress tests" 0
fi

echo ""
echo "======================================"
echo "Test Category 5: Integration Tests"
echo "======================================"

echo -n "Test 5.1: Consistency across runs... "
./ledger_tcp_chain > /dev/null 2>&1
result1=$?
./ledger_tcp_chain > /dev/null 2>&1
result2=$?
./ledger_tcp_chain > /dev/null 2>&1
result3=$?

if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $result3 -eq 0 ]; then
    test_result "Multiple runs consistency" 0
else
    test_result "Multiple runs consistency" 1
fi

echo -n "Test 5.2: Address sanitizer... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g -fsanitize=address \
    main.c obj/*/*.o -o $TESTS_DIR/test_asan 2>&1 | grep -q error; then
    echo -e "${YELLOW}  Sanitizer not available${NC}"
elif ! timeout 15s $TESTS_DIR/test_asan > /dev/null 2>&1; then
    test_result "Address sanitizer" 1
    timeout 15s $TESTS_DIR/test_asan
else
    test_result "Address sanitizer" 0
fi

echo ""
echo "======================================"
echo "Test Category 6: Extreme Stress Tests"
echo "======================================"

# Extreme test 1: 500 transactions
cat > $TESTS_DIR/test_extreme_500tx.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.1, 0.05, 0.05, 0.05);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i, 100000000);
    }

    for (int i = 0; i < 500; i++) {
        transaction_t tx = {i + 1, (i % 3) + 1, ((i + 1) % 3) + 1, 1};
        int creator = i % 3;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);
        consensus_process_block(nodes[creator], block);
        sync_broadcast_block(sim, creator, block, 3);
        free(block);

        for (int step = 0; step < 10; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 3; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }
    }

    for (int step = 0; step < 2000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 3; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
        if (step % 400 == 0) {
            for (int i = 0; i < 3; i++) cnode_update_state(nodes[i]);
        }
    }

    sync_finalize(nodes, 3);
    int result = consensus_verify(nodes, 3) ? 0 : 1;

    for (int i = 0; i < 3; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 6.1: Extreme (500 transactions)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_extreme_500tx.c \
    obj/*/*.o -o $TESTS_DIR/test_extreme_500tx 2>&1 | grep -q error; then
    test_result "500 TX (compile)" 1
elif ! timeout 120s $TESTS_DIR/test_extreme_500tx 2>&1; then
    test_result "500 transactions" 1
else
    test_result "500 transactions" 0
fi

# Extreme test 2: 5 nodes with high instability
cat > $TESTS_DIR/test_extreme_5nodes.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(5);
    tcp_simulator_set_probabilities(sim, 0.2, 0.15, 0.1, 0.1);

    consensus_node_t *nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i] = cnode_create(i, 10000000);
    }

    for (int i = 0; i < 200; i++) {
        transaction_t tx = {i + 1, (i % 3) + 1, ((i + 1) % 3) + 1, 1};
        int creator = i % 5;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);
        consensus_process_block(nodes[creator], block);
        sync_broadcast_block(sim, creator, block, 5);
        free(block);

        for (int step = 0; step < 15; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 5; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }
    }

    for (int step = 0; step < 3000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 5; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
        if (step % 500 == 0) {
            for (int i = 0; i < 5; i++) cnode_update_state(nodes[i]);
        }
    }

    sync_finalize(nodes, 5);
    int result = consensus_verify(nodes, 5) ? 0 : 1;

    for (int i = 0; i < 5; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 6.2: Extreme (5 nodes, high instability)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_extreme_5nodes.c \
    obj/*/*.o -o $TESTS_DIR/test_extreme_5nodes 2>&1 | grep -q error; then
    test_result "5 nodes (compile)" 1
elif ! timeout 120s $TESTS_DIR/test_extreme_5nodes 2>&1; then
    test_result "5 nodes high instability" 1
else
    test_result "5 nodes high instability" 0
fi

# Extreme test 3: 30% packet drop rate
cat > $TESTS_DIR/test_extreme_highdrop.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.3, 0.3, 0.1, 0.1);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i, 10000000);
    }

    for (int i = 0; i < 50; i++) {
        transaction_t tx = {i + 1, (i % 3) + 1, ((i + 1) % 3) + 1, 1};
        int creator = i % 3;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);
        consensus_process_block(nodes[creator], block);

        // Broadcast multiple times due to high drop rate
        for (int retry = 0; retry < 3; retry++) {
            sync_broadcast_block(sim, creator, block, 3);
        }
        free(block);

        for (int step = 0; step < 50; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 3; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }
    }

    for (int step = 0; step < 5000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 3; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
        if (step % 500 == 0) {
            for (int i = 0; i < 3; i++) cnode_update_state(nodes[i]);
        }
    }

    sync_finalize(nodes, 3);
    int result = consensus_verify(nodes, 3) ? 0 : 1;

    for (int i = 0; i < 3; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 6.3: Extreme (30% packet drop)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_extreme_highdrop.c \
    obj/*/*.o -o $TESTS_DIR/test_extreme_highdrop 2>&1 | grep -q error; then
    test_result "30% drop (compile)" 1
elif ! timeout 120s $TESTS_DIR/test_extreme_highdrop 2>&1; then
    test_result "30% packet drop" 1
else
    test_result "30% packet drop" 0
fi

# Extreme test 4: Rapid consecutive blocks (fork stress)
cat > $TESTS_DIR/test_extreme_forks.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.5, 0.1, 0.2, 0.3);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i, 10000000);
    }

    // Each node creates blocks simultaneously to stress fork handling
    for (int round = 0; round < 30; round++) {
        for (int creator = 0; creator < 3; creator++) {
            transaction_t tx = {round * 3 + creator + 1, (creator % 3) + 1, ((creator + 1) % 3) + 1, 1};
            blockchain_t *chain = cnode_get_chain(nodes[creator]);
            block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
            block_add_transaction(block, &tx);
            block_compute_hash(block);
            consensus_process_block(nodes[creator], block);
            sync_broadcast_block(sim, creator, block, 3);
            free(block);
        }

        for (int step = 0; step < 30; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 3; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }

        for (int i = 0; i < 3; i++) cnode_update_state(nodes[i]);
    }

    for (int step = 0; step < 2000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 3; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
    }

    sync_finalize(nodes, 3);
    int result = consensus_verify(nodes, 3) ? 0 : 1;

    for (int i = 0; i < 3; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 6.4: Extreme (concurrent forks)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_extreme_forks.c \
    obj/*/*.o -o $TESTS_DIR/test_extreme_forks 2>&1 | grep -q error; then
    test_result "Forks (compile)" 1
elif ! timeout 120s $TESTS_DIR/test_extreme_forks 2>&1; then
    test_result "Concurrent forks" 1
else
    test_result "Concurrent forks" 0
fi

# Extreme test 5: 1000 transactions endurance
cat > $TESTS_DIR/test_extreme_1000tx.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.05, 0.02, 0.02, 0.02);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i, 1000000000);
    }

    for (int i = 0; i < 1000; i++) {
        transaction_t tx = {i + 1, (i % 3) + 1, ((i + 1) % 3) + 1, 1};
        int creator = i % 3;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);
        consensus_process_block(nodes[creator], block);
        sync_broadcast_block(sim, creator, block, 3);
        free(block);

        for (int step = 0; step < 5; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 3; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }

        if (i % 100 == 99) {
            for (int k = 0; k < 3; k++) cnode_update_state(nodes[k]);
        }
    }

    for (int step = 0; step < 3000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 3; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
    }

    sync_finalize(nodes, 3);
    int result = consensus_verify(nodes, 3) ? 0 : 1;

    for (int i = 0; i < 3; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 6.5: Extreme (1000 transactions)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_extreme_1000tx.c \
    obj/*/*.o -o $TESTS_DIR/test_extreme_1000tx 2>&1 | grep -q error; then
    test_result "1000 TX (compile)" 1
elif ! timeout 180s $TESTS_DIR/test_extreme_1000tx 2>&1; then
    test_result "1000 transactions" 1
else
    test_result "1000 transactions" 0
fi

echo ""
echo "======================================"
echo "Test Category 7: Nightmare Mode"
echo "======================================"

# Nightmare 1: 500 TX + 5 nodes + high instability
cat > $TESTS_DIR/test_nightmare_1.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(5);
    tcp_simulator_set_probabilities(sim, 0.25, 0.2, 0.15, 0.15);

    consensus_node_t *nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i] = cnode_create(i, 1000000000);
    }

    for (int i = 0; i < 500; i++) {
        transaction_t tx = {i + 1, (i % 3) + 1, ((i + 1) % 3) + 1, 1};
        int creator = i % 5;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);
        consensus_process_block(nodes[creator], block);

        for (int r = 0; r < 2; r++) sync_broadcast_block(sim, creator, block, 5);
        free(block);

        for (int step = 0; step < 15; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 5; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }
        if (i % 50 == 49) {
            for (int k = 0; k < 5; k++) cnode_update_state(nodes[k]);
        }
    }

    for (int step = 0; step < 5000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 5; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
        if (step % 500 == 0) {
            for (int i = 0; i < 5; i++) cnode_update_state(nodes[i]);
        }
    }

    sync_finalize(nodes, 5);
    int result = consensus_verify(nodes, 5) ? 0 : 1;

    for (int i = 0; i < 5; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 7.1: Nightmare (500TX + 5nodes + chaos)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_nightmare_1.c \
    obj/*/*.o -o $TESTS_DIR/test_nightmare_1 2>&1 | grep -q error; then
    test_result "Nightmare 1 (compile)" 1
elif ! timeout 300s $TESTS_DIR/test_nightmare_1 2>&1; then
    test_result "500TX + 5nodes + chaos" 1
else
    test_result "500TX + 5nodes + chaos" 0
fi

# Nightmare 2: 1000 TX + 30% drop + concurrent forks
cat > $TESTS_DIR/test_nightmare_2.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.4, 0.3, 0.2, 0.2);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i, 10000000000LL);
    }

    for (int round = 0; round < 333; round++) {
        // Each node creates block simultaneously (fork stress)
        for (int creator = 0; creator < 3; creator++) {
            transaction_t tx = {round * 3 + creator + 1, (creator % 3) + 1, ((creator + 1) % 3) + 1, 1};
            blockchain_t *chain = cnode_get_chain(nodes[creator]);
            block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
            block_add_transaction(block, &tx);
            block_compute_hash(block);
            consensus_process_block(nodes[creator], block);

            for (int r = 0; r < 3; r++) sync_broadcast_block(sim, creator, block, 3);
            free(block);
        }

        for (int step = 0; step < 30; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 3; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }

        if (round % 33 == 32) {
            for (int i = 0; i < 3; i++) cnode_update_state(nodes[i]);
        }
    }

    for (int step = 0; step < 8000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 3; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
        if (step % 1000 == 0) {
            for (int i = 0; i < 3; i++) cnode_update_state(nodes[i]);
        }
    }

    sync_finalize(nodes, 3);
    int result = consensus_verify(nodes, 3) ? 0 : 1;

    for (int i = 0; i < 3; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 7.2: Nightmare (1000TX + 30%drop + forks)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_nightmare_2.c \
    obj/*/*.o -o $TESTS_DIR/test_nightmare_2 2>&1 | grep -q error; then
    test_result "Nightmare 2 (compile)" 1
elif ! timeout 300s $TESTS_DIR/test_nightmare_2 2>&1; then
    test_result "1000TX + 30%drop + forks" 1
else
    test_result "1000TX + 30%drop + forks" 0
fi

# Nightmare 3: Ultimate hell - 10 nodes + 500TX + 40% drop + concurrent forks
cat > $TESTS_DIR/test_nightmare_3.c << 'EOF'
#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    consensus_init();
    tcp_simulator_t *sim = tcp_simulator_create(10);
    tcp_simulator_set_probabilities(sim, 0.5, 0.4, 0.25, 0.25);

    consensus_node_t *nodes[10];
    for (int i = 0; i < 10; i++) {
        nodes[i] = cnode_create(i, 100000000000LL);
    }

    for (int round = 0; round < 50; round++) {
        // All 10 nodes create blocks simultaneously
        for (int creator = 0; creator < 10; creator++) {
            transaction_t tx = {round * 10 + creator + 1, (creator % 3) + 1, ((creator + 1) % 3) + 1, 1};
            blockchain_t *chain = cnode_get_chain(nodes[creator]);
            block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
            block_add_transaction(block, &tx);
            block_compute_hash(block);
            consensus_process_block(nodes[creator], block);

            for (int r = 0; r < 5; r++) sync_broadcast_block(sim, creator, block, 10);
            free(block);
        }

        for (int step = 0; step < 100; step++) {
            tcp_simulator_step(sim);
            for (int j = 0; j < 10; j++) {
                block_t recv;
                if (network_receive(sim, j, &recv, sizeof(block_t)) > 0) {
                    consensus_process_block(nodes[j], &recv);
                }
            }
        }

        for (int i = 0; i < 10; i++) cnode_update_state(nodes[i]);
    }

    for (int step = 0; step < 10000; step++) {
        tcp_simulator_step(sim);
        for (int i = 0; i < 10; i++) {
            block_t recv;
            if (network_receive(sim, i, &recv, sizeof(block_t)) > 0) {
                consensus_process_block(nodes[i], &recv);
            }
        }
        if (step % 1000 == 0) {
            for (int i = 0; i < 10; i++) cnode_update_state(nodes[i]);
        }
    }

    sync_finalize(nodes, 10);
    int result = consensus_verify(nodes, 10) ? 0 : 1;

    for (int i = 0; i < 10; i++) cnode_destroy(nodes[i]);
    tcp_simulator_destroy(sim);
    consensus_cleanup();
    return result;
}
EOF

echo -n "Test 7.3: Nightmare (10nodes + 500TX + 40%drop)... "
if gcc -Wall -Wextra -Werror -std=c11 -Iinclude -g $TESTS_DIR/test_nightmare_3.c \
    obj/*/*.o -o $TESTS_DIR/test_nightmare_3 2>&1 | grep -q error; then
    test_result "Nightmare 3 (compile)" 1
elif ! timeout 300s $TESTS_DIR/test_nightmare_3 2>&1; then
    test_result "10nodes + 500TX + 40%drop" 1
else
    test_result "10nodes + 500TX + 40%drop" 0
fi

echo ""
echo "======================================"
echo "Test Summary"
echo "======================================"
echo -e "Total tests:  $TOTAL"
echo -e "${GREEN}Passed:       $PASSED${NC}"
echo -e "${RED}Failed:       $FAILED${NC}"
echo ""

# Cleanup compiled files
echo ""
echo -e "${YELLOW}Cleaning up...${NC}"
rm -f $TESTS_DIR/test_* 2>/dev/null
rm -rf $TEST_DIR 2>/dev/null
rm -f ledger_tcp_chain 2>/dev/null
make clean > /dev/null 2>&1
echo -e "${GREEN}Cleanup complete${NC}"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}ALL TESTS PASSED!${NC}"
    echo -e "${GREEN}========================================${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}SOME TESTS FAILED!${NC}"
    echo -e "${RED}========================================${NC}"
    exit 1
fi
