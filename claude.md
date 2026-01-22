# LedgerTCP-Chain Code Structure Guidelines

## 코딩 규칙 (사용자 지정)

### 1. 함수 형태
```c
void function(void)
{
    // '{' 는 반드시 다음 줄에
}
```

### 2. 조건문/반복문
- `if`, `for` 문에 `{}`가 없어도 OK (두 줄로 분리된 경우)
```c
if (!ptr)
    return NULL;
for (i = 0; i < n; i++)
    do_something(i);
```
- **금지**: 한 줄에 다 적는 것
```c
// 금지
if (!sim) return;
if (!ptr) return NULL;

// 허용
if (!sim)
    return;
if (!ptr)
    return NULL;
```

### 3. 변수 선언
- 모든 변수는 **함수 맨 앞**에 선언
- for 블록 안에서 변수 선언 금지
```c
void function(void)
{
    int i;
    int j;
    char *ptr;

    // 로직 시작
}
```

### 4. 제한 사항
- **매개변수**: 함수당 최대 5개
- **지역변수**: 함수당 최대 5개
- **파일 수**: 폴더당 최대 5개
- **함수 수**: 파일당 최대 5개 (static 함수 포함!)
- **함수 길이**: 최대 25줄

### 5. 기타
- `goto`문 사용 금지
- 폴더/파일/함수 개수는 늘려도 됨 (제한을 맞추기 위해)
- 사용하지 않는 trivial wrapper 함수 제거

### 6. 전역 변수 금지
- 파일 내 `static` 전역 변수 사용 금지
- 모든 상태는 **구조체**를 통해 관리

---

## 현재 작업 상태 (2026-01-22)

### 완료된 작업

#### 1단계: 함수 형태 정리 ✅
- 모든 함수 `{`를 다음 줄로 이동
- goto문 제거 완료

#### 2단계: 변수/매개변수 제한 ✅
- 변수 선언 함수 맨앞으로 이동
- 변수 5개 초과 함수 분리

#### 3단계: 25줄/함수 분리 ✅
- 모든 함수가 25줄 이하로 분리 완료

#### 4단계: 정리 작업 ✅
- 한줄 조건문 두 줄로 분리 완료
- static 전역 변수 제거 완료
- 파일당 함수 5개 제한 적용 완료
- 스페이스 인덴트를 탭으로 변경 완료

#### 5단계: 폴더당 파일 5개 제한 ⏳ (진행중)

**완료된 작업:**
1. 새 폴더 생성 완료:
   - `src/sim/` - sim_create.c, sim_process.c (tcp에서 이동)
   - `src/state/` - state_load.c, state_save.c (ledger에서 이동)
   - `src/chain/` - chain_ops.c (block에서 이동)
   - `src/sync/` - sync.c, sync_final.c, sync_receive.c, reorg_util.c (consensus에서 이동)
   - `src/main/` - main_sim.c, main_tx.c (utils에서 이동)

2. Makefile 수정 완료 - 새 폴더들 빌드 규칙 추가됨

3. 빌드 성공 확인됨

**남은 작업:**
1. test.sh 수정 필요 - 새 폴더의 obj 파일들 포함하도록:
   - Test 2.2: `obj/chain/*.o` 추가됨 ✅
   - Test 2.3: `obj/sim/*.o` 추가 필요
   - Test 3.1: `obj/chain/*.o`, `obj/sim/*.o` 추가 필요

2. **trivial wrapper 함수 제거 필요** (free만 호출하는 함수들):
   - `src/block/block_core.c:26` - `block_destroy()` → 호출부에서 직접 `free(block)` 사용
   - `src/ledger/history.c:15` - `history_destroy()` → 호출부에서 직접 `free(history)` 사용
   - `src/tcp/packet.c:26` - `packet_destroy()` → 호출부에서 직접 `free(packet)` 사용

   **제거 방법:**
   1. 각 함수의 호출부 찾기: `grep -rn "block_destroy\|history_destroy\|packet_destroy" src/`
   2. 호출부를 직접 `free()` 호출로 변경
   3. 함수 정의 삭제
   4. 헤더 파일에서 함수 선언 삭제

3. **불필요한 {} 제거 필요** (한 줄 문장에 {} 사용한 경우):
   ```c
   // 현재 (잘못됨)
   if (!block)
   {
       return false;
   }

   // 수정 후 (올바름)
   if (!block)
       return false;
   ```

   **수정 필요한 파일:**
   - `src/block/chain.c`
   - `src/block/genesis.c`
   - `src/block/validation.c`
   - `src/consensus/fork.c`
   - `src/ledger/ledger_core.c`

4. 테스트 실행하여 18개 모두 통과 확인

---

## 현재 프로젝트 구조

### 폴더별 파일 수 (모두 5개 이하)
```
src/tcp/       (5개) - network.c, packet.c, queue.c, simulator.c, tcp_node.c
src/ledger/    (5개) - account.c, history.c, ledger_core.c, state.c, transaction.c
src/block/     (5개) - block_core.c, chain.c, genesis.c, hash.c, validation.c
src/consensus/ (5개) - consensus_core.c, consensus_node.c, fork.c, reorg.c, cnode_util.c
src/utils/     (5개) - error.c, logger.c, memory.c, random.c, time.c
src/sim/       (2개) - sim_create.c, sim_process.c
src/state/     (2개) - state_load.c, state_save.c
src/chain/     (1개) - chain_ops.c
src/sync/      (4개) - sync.c, sync_final.c, sync_receive.c, reorg_util.c
src/main/      (2개) - main_sim.c, main_tx.c
```

### 헤더 파일
```
include/ - tcp_simulator.h, ledger.h, block.h, consensus.h, utils.h
```

---

## 다음 세션에서 할 일

1. test.sh 수정 완료:
```bash
# Test 2.3 수정 (line 255-264):
# obj/tcp/*.o obj/utils/random.o 를
# obj/tcp/*.o obj/sim/*.o obj/utils/random.o 로 변경

# Test 3.1 수정 (line 395-405):
# obj/ledger/*.o obj/block/*.o obj/tcp/*.o obj/utils/random.o 를
# obj/ledger/*.o obj/state/*.o obj/block/*.o obj/chain/*.o obj/tcp/*.o obj/sim/*.o obj/utils/random.o 로 변경
```

2. 테스트 실행:
```bash
./test.sh
```

3. 18개 테스트 모두 통과 확인

---

## 빌드 및 테스트

```bash
make clean && make    # 빌드
./test.sh             # 테스트 (18개 모두 통과해야 함)
```

---

## 주의사항

- **static 함수도 5개 제한에 포함됨!**
- **폴더당 파일도 5개 제한!**
- 함수 분리 시 파일당 5개 넘으면 새 파일로 분리 필요
- 파일 분리 시 폴더당 5개 넘으면 새 폴더로 분리 필요
- 모르면 사용자에게 물어볼 것
