# LedgerTCP-Chain Code Structure Guidelines

## Project Structure Rules

### 1. Organization Rules
- **폴더별 구분**: 레이어별로 독립적인 폴더 생성
- **파일 제한**: 한 폴더당 최대 5개 파일
- **함수 제한**: 한 파일당 최대 5개 함수
- **라인 제한**: 한 함수당 최대 25줄

### 2. Directory Structure

```
LedgerTCP-Chain/
├── README.md
├── claude.md
├── Makefile
├── main.c
├── include/
│   └── (all header files)
└── src/
    ├── tcp/          # TCP Simulator Layer
    │   ├── simulator.c
    │   ├── packet.c
    │   ├── queue.c
    │   ├── node.c
    │   └── network.c
    ├── ledger/       # Ledger Layer
    │   ├── ledger.c
    │   ├── account.c
    │   ├── transaction.c
    │   ├── history.c
    │   └── state.c
    ├── block/        # Block Layer
    │   ├── block.c
    │   ├── chain.c
    │   ├── hash.c
    │   ├── genesis.c
    │   └── validation.c
    ├── consensus/    # Consensus Layer
    │   ├── consensus.c
    │   ├── node.c
    │   ├── fork.c
    │   ├── reorg.c
    │   └── sync.c
    └── utils/        # Utility Functions
        ├── memory.c
        ├── random.c
        ├── time.c
        ├── logger.c
        └── error.c
```

## Layer Breakdown

### TCP Simulator Layer (`src/tcp/`)
1. **simulator.c**: 시뮬레이터 생성/파괴 및 메인 로직
   - tcp_simulator_create()
   - tcp_simulator_destroy()
   - tcp_simulator_step()
   - tcp_simulator_set_probabilities()
   - tcp_simulator_get_stats()

2. **packet.c**: 패킷 관리
   - packet_create()
   - packet_destroy()
   - packet_copy()
   - packet_validate()
   - packet_serialize()

3. **queue.c**: 패킷 큐 관리
   - queue_create()
   - queue_destroy()
   - queue_enqueue()
   - queue_dequeue()
   - queue_is_empty()

4. **node.c**: 노드 관리
   - node_init()
   - node_cleanup()
   - node_activate()
   - node_deactivate()
   - node_get_status()

5. **network.c**: 네트워크 통신
   - network_send()
   - network_receive()
   - network_broadcast()
   - network_apply_delay()
   - network_apply_events()

### Ledger Layer (`src/ledger/`)
1. **ledger.c**: 원장 코어 로직
   - ledger_create()
   - ledger_destroy()
   - ledger_clone()
   - ledger_clear()
   - ledger_validate()

2. **account.c**: 계정 관리
   - account_create()
   - account_find()
   - account_get_balance()
   - account_set_balance()
   - account_update_balance()

3. **transaction.c**: 트랜잭션 처리
   - transaction_create()
   - transaction_validate()
   - transaction_apply()
   - transaction_revert()
   - transaction_serialize()

4. **history.c**: 트랜잭션 히스토리
   - history_create()
   - history_destroy()
   - history_add()
   - history_contains()
   - history_get_count()

5. **state.c**: 상태 관리
   - state_save()
   - state_restore()
   - state_compare()
   - state_hash()
   - state_export()

### Block Layer (`src/block/`)
1. **block.c**: 블록 코어
   - block_create()
   - block_destroy()
   - block_add_transaction()
   - block_validate()
   - block_serialize()

2. **chain.c**: 블록체인 관리
   - chain_create()
   - chain_destroy()
   - chain_add_block()
   - blockchain_clone()
   - blockchain_get_block_by_hash()
   - (chain_get_length, chain_get_head는 block.h에 static inline)

3. **hash.c**: 해시 함수
   - hash_compute()
   - hash_copy()
   - hash_equals()
   - hash_zero()
   - hash_is_zero()

4. **genesis.c**: 제네시스 블록
   - genesis_create()
   - genesis_validate()
   - genesis_get_hash()
   - genesis_init_state_custom()
   - genesis_export()

5. **validation.c**: 블록 검증
   - validate_block_structure()
   - validate_block_transactions()
   - validate_block_hash()
   - validate_prev_hash()
   - block_compute_hash()

### Consensus Layer (`src/consensus/`)
1. **consensus.c**: 합의 메인 로직
   - consensus_init()
   - consensus_cleanup()
   - consensus_process_block()
   - consensus_get_state()
   - consensus_verify()

2. **node.c**: 합의 노드
   - cnode_create()
   - cnode_destroy()
   - cnode_get_chain()
   - cnode_get_ledger()
   - cnode_update_state()

3. **fork.c**: 포크 관리
   - fork_create()
   - fork_destroy()
   - fork_add_candidate()
   - fork_get_longest()
   - fork_cleanup()

4. **reorg.c**: 체인 재조정
   - reorg_execute()
   - reorg_rebuild_ledger()
   - reorg_validate()
   - reorg_rollback()
   - reorg_commit()

5. **sync.c**: 노드 동기화
   - sync_start()
   - sync_receive_block()
   - sync_broadcast_block()
   - sync_check_consensus()
   - sync_finalize()

### Utility Layer (`src/utils/`)
1. **memory.c**: 메모리 관리
   - safe_malloc()
   - safe_free()
   - memory_check()
   - memory_stats()
   - memory_cleanup()

2. **random.c**: 난수 생성
   - random_init()
   - random_double()
   - random_int()
   - random_bool()
   - random_choice()

3. **time.c**: 시간 관리
   - time_current()
   - time_elapsed()
   - time_diff()
   - time_to_string()
   - time_sleep()

4. **logger.c**: 로깅
   - log_init()
   - log_info()
   - log_warn()
   - log_error()
   - log_debug()

5. **error.c**: 에러 처리
   - error_set()
   - error_get()
   - error_clear()
   - error_to_string()
   - error_is_fatal()

## Header Files Organization

모든 헤더 파일은 `include/` 디렉토리에 위치:
- `tcp_simulator.h`
- `ledger.h`
- `block.h`
- `consensus.h`
- `utils.h`

## Coding Standards

### Function Guidelines
- 함수는 단일 책임 원칙(Single Responsibility Principle) 준수
- 함수 이름은 동사로 시작 (create, destroy, add, get, set 등)
- 25줄 제한을 위해 복잡한 로직은 헬퍼 함수로 분리
- 에러 처리는 간결하게 (early return 패턴 사용)

### File Guidelines
- 각 파일은 관련된 기능만 포함
- static 함수는 내부 헬퍼로만 사용
- 파일 당 5개 함수 제한 (static 헬퍼 함수는 카운트에서 제외 가능)

### Memory Management
- 모든 동적 할당은 대응하는 해제 함수 필요
- NULL 체크 필수
- valgrind로 메모리 누수 검증

## Build System

Makefile은 각 레이어별로 별도 컴파일:
```make
LAYERS = tcp ledger block consensus utils
```

## 현재 작업 상태 (2026-01-20)

### 완료된 작업
✅ 프로젝트 초기 구조 생성 (5개 레이어, 각 레이어당 5개 파일)
✅ 전체 빌드 시스템 구축 (Makefile)
✅ MAX_CHAINS을 10에서 50으로 증가 (더 많은 포크 처리)
✅ sync_finalize에 deterministic 체인 선택 로직 추가 (해시 비교)
✅ test_results 디렉토리 정리 (코드/바이너리 제거, 로그만 유지)
✅ genesis_init_state_custom() 함수 추가 (파라미터화된 초기 잔액)
✅ consensus_node_t에 genesis_balance 필드 추가
✅ cnode_create() 시그니처 변경 (genesis_balance 파라미터 추가)
✅ reorg_rebuild_ledger()가 노드별 초기 잔액 사용하도록 수정
✅ **10/10 테스트 모두 통과**
✅ 코드 리팩토링 - 5함수/파일, 25줄/함수 규칙 준수

### 리팩토링 완료 내역 (2026-01-20 최신)
1. **chain.c** (7→5 함수): `chain_get_length`, `chain_get_head`를 header에 `static inline`으로 이동
2. **genesis.c** (6→5 함수): 사용되지 않는 `genesis_init_state()` 제거 (callers는 `genesis_init_state_custom(1000)` 사용)
3. **validation.c** (6→5 함수): 사용되지 않는 `validate_timestamp()` 제거
4. **sync.c** 함수 길이 수정: `sync_receive_block`과 `sync_finalize`를 static 헬퍼로 분리
5. **state.c** 함수 길이 수정: `state_restore`를 `restore_accounts`/`restore_history` 헬퍼로 분리
6. **chain.c** `blockchain_clone` 길이 수정: `block_clone` static 헬퍼로 분리
7. **test_edge.c** 수정: `blockchain_get_block_by_height` → `blockchain_get_block_by_hash` 변경

### 현재 상태
✅ 빌드 성공 (모든 컴파일 경고 없음)
⏳ 테스트 실행 필요

### 문제 상세 분석

#### 근본 원인
1. **genesis_init_state()가 하드코딩된 초기 잔액 사용**
   - 위치: `src/block/genesis.c:30-39`
   - 현재: 10000000으로 설정됨 (스트레스 테스트용)
   - 문제: 기본 테스트들은 1000을 기대함

2. **reorg_rebuild_ledger()가 genesis_init_state()를 호출**
   - 위치: `src/consensus/reorg.c:6`
   - 체인 재구성 시 항상 genesis 초기 잔액으로 리셋됨
   - 노드마다 설정된 초기 잔액과 불일치 발생

3. **테스트 결과 예시**
   ```
   Node 0: balance1=870 (기대값, genesis 1000에서 거래 처리)
   Node 1: balance1=9999870 (실제값, genesis 10000000에서 거래 처리)
   Node 2: balance1=9999870
   ```

#### 시도한 해결책들
1. ❌ genesis_init_state()를 10000000으로 변경
   - 결과: 스트레스 테스트는 개선되나 기본 테스트 실패

2. ❌ transaction_apply()의 잔액 체크 제거
   - 결과: 다른 테스트들 실패 (잔액 검증 필요)

3. ❌ 네트워크 파라미터만 조정 (확률 낮춤, 시뮬레이션 스텝 증가)
   - 결과: 근본 문제 해결 안됨

## 다음 작업 (Next Steps)

### 🎯 즉시 실행: 테스트 실행
```bash
./test.sh
```
- 리팩토링 완료 후 테스트 검증 필요
- 기대 결과: 10/10 테스트 통과

### 참고: 이미 완료된 작업들
- ✅ genesis_init_state_custom() 함수 추가됨
- ✅ consensus_node_t에 genesis_balance 필드 추가됨
- ✅ cnode_create() 시그니처 변경됨 (genesis_balance 파라미터)
- ✅ reorg_rebuild_ledger()가 노드별 초기 잔액 사용
- ✅ 코드 규칙 준수 리팩토링 완료 (5함수/파일, 25줄/함수)

## 참고사항

- **파일 구조**: 5개 파일/폴더, 5개 함수/파일, 25줄/함수 준수 중
- **메모리 관리**: valgrind 통과 확인됨
- **네트워크 파라미터**: delay=0.05, drop=0.02, dup=0.02, reorder=0.02
- **스트레스 테스트**: 100 트랜잭션, 3000 시뮬레이션 스텝 (20 인터리브 + 1000 파이널)
