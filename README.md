# LedgerTCP-Chain
### Distributed Ledger over Unreliable TCP

---

## I. Overview

LedgerTCP-Chain은 **신뢰할 수 없는 네트워크 환경**을 전제로,
모든 노드가 **동일한 원장 상태에 수렴**하도록 만드는 분산 원장 시스템을 구현하는 과제입니다.

이 과제는 TCP의 재전송, 중복, 순서 뒤바뀜과 같은 **비결정성**을 하위 레이어에서 그대로 노출한 상태로,
상위 레이어(Ledger / Block / Consensus)가 이를 어떻게 극복하는지를 구현하는 것을 목표로 합니다.

본 과제는 블록체인을 다루지만,
채굴, 코인 보상, 난이도 조절과 같은 요소는 **명시적으로 포함하지 않습니다**.

---

## II. Goals

- 신뢰할 수 없는 전송 계층 위에서 결정적 상태 수렴 구현
- 중복 트랜잭션에 대한 idempotency 보장
- 블록 포크 발생 및 체인 재조정(reorganization) 처리
- 모든 노드의 최종 ledger 상태 일치

---

## III. General Instructions

- Language: **C**
- Global variables are **forbidden**
- Memory leaks are **forbidden** (valgrind 기준)
- Undefined behavior 발생 시 Fail
- 외부 네트워크 사용 금지  
  (모든 통신은 내부 TCP 시뮬레이터를 통해 이루어져야 함)
- Makefile 필수

---

## IV. Mandatory Part

### 1. TCP Simulator Layer

TCP Simulator는 실제 소켓을 사용하지 않습니다.

다음 이벤트들이 **랜덤하게 발생**해야 합니다:

- 패킷 지연 (delay)
- 패킷 유실 (drop)
- 패킷 중복 (duplicate)
- 패킷 순서 뒤바뀜 (reorder)

TCP Simulator는 **신뢰성을 보장해서는 안 됩니다**.
신뢰성은 상위 레이어의 책임입니다.

---

### 2. Ledger Layer

Ledger Layer는 트랜잭션을 처리하는 결정적 상태 머신입니다.

#### Requirements
- 각 트랜잭션은 고유한 `tx_id`를 가짐
- 동일한 `tx_id`는 한 번만 적용되어야 함 (idempotent)
- 트랜잭션 적용 순서와 무관하게 최종 결과가 동일해야 함
- 음수 잔액은 허용되지 않음

---

### 3. Block Layer

Block Layer는 여러 트랜잭션을 하나의 블록으로 묶습니다.

#### Requirements
- 블록은 `prev_hash`를 통해 이전 블록을 참조
- `prev_hash` 불일치 시 포크 허용
- 포크 발생 시 **longest-chain rule** 적용
- 체인 재조정(reorganization) 시 ledger 상태 복구 필수

---

### 4. Consensus Rule

본 과제는 완전한 합의 알고리즘을 요구하지 않습니다.

다음 규칙만을 만족하면 됩니다:

- 모든 노드는 가장 긴 체인을 기준으로 상태를 결정
- 네트워크 분할 후 재합류 시 체인 재조정 수행
- 최종적으로 모든 노드는 동일한 ledger 상태에 도달해야 함

---

## V. Bonus Part

- 악의적 노드 시뮬레이션
- 네트워크 partition 자동 테스트
- 상태 변화 타임라인 로그 출력
- fork / reorg 시각화

---

## VI. Submission

- 하나의 Git repository로 제출
- `README.md` 필수
  - 설계 개요
  - 레이어 구조 설명
  - 실행 방법
- `make` 명령으로 빌드 가능해야 함

---

## VII. Evaluation

다음 항목을 기준으로 평가합니다:

- 패킷 유실 / 중복 / 지연 상황에서의 정상 수렴 여부
- fork 발생 및 reorganization 처리 정확성
- 메모리 관리 (leak / double free)
- 코드 구조 및 가독성

---

**주의:**  
모든 노드의 최종 ledger 상태가 다를 경우,
출력 로그와 관계없이 Fail 처리됩니다.
