# 05. 준수 및 문서화

[← 이전: 정적 분석 도구](04-static-analysis.md) | [메인으로](../README.md) | [다음: 실습 예제 →](06-practical-examples.md)

---

## 📚 학습 목표

이 장을 완료하면 다음을 수행할 수 있습니다:
- MISRA Compliance:2020 요구사항 이해
- Guideline Enforcement Plan (GEP) 작성
- Guideline Compliance Summary (GCS) 작성
- Deviation Record 문서화

---

## 5.1 MISRA Compliance 개요

### 컴플라이언스란?

**MISRA Compliance**는 단순히 "모든 규칙을 지켰다"가 아니라, **체계적인 프로세스**를 통해 **문서화된 증거**를 제공하는 것입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                 MISRA Compliance 구성 요소                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌────────────────────┐                                         │
│  │ Guideline          │  규칙 적용 계획                          │
│  │ Enforcement Plan   │  (어떤 규칙을, 어떻게 검사)               │
│  │ (GEP)              │                                         │
│  └─────────┬──────────┘                                         │
│            │                                                    │
│            ▼                                                    │
│  ┌────────────────────┐                                         │
│  │ Static Analysis    │  규칙 위반 검출                          │
│  │ Tool Reports       │  (도구 분석 결과)                         │
│  └─────────┬──────────┘                                         │
│            │                                                    │
│            ▼                                                    │
│  ┌────────────────────┐                                         │
│  │ Deviation          │  위반 사항 문서화                        │
│  │ Records            │  (허용된 위반의 근거)                     │
│  └─────────┬──────────┘                                         │
│            │                                                    │
│            ▼                                                    │
│  ┌────────────────────┐                                         │
│  │ Guideline          │  최종 준수 요약                          │
│  │ Compliance Summary │  (전체 현황)                             │
│  │ (GCS)              │                                         │
│  └────────────────────┘                                         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 컴플라이언스 조건

| 카테고리 | 컴플라이언스 조건 |
|----------|------------------|
| **Mandatory** | 위반 없음 (Deviation 불가) |
| **Required** | 위반 없음 OR 승인된 Deviation 존재 |
| **Advisory** | 위반 추적 권장 (공식 Deviation 선택적) |

---

## 5.2 Guideline Enforcement Plan (GEP)

### GEP란?

**GEP**는 프로젝트에서 MISRA 가이드라인을 **어떻게 적용하고 검증**할 것인지를 정의하는 문서입니다.

### GEP 필수 구성 요소

| 항목 | 설명 |
|------|------|
| 적용 가이드라인 버전 | MISRA C:2023 등 |
| 규칙 재분류 | Advisory → Required 등 |
| 검증 도구 | Cppcheck, PC-lint 등 |
| 검증 방법 | 자동 분석, 코드 리뷰 |
| Deviation 승인 프로세스 | 승인 권한자, 절차 |

### GEP 템플릿

```markdown
# Guideline Enforcement Plan (GEP)

## 1. 문서 정보
- 프로젝트명: [프로젝트명]
- 버전: 1.0
- 작성일: 2025-02-01
- 승인자: [기술 책임자]

## 2. 적용 가이드라인
- MISRA C:2023

## 3. 규칙 재분류

| 규칙 | 기본 | 재분류 | 사유 |
|------|------|--------|------|
| Rule 15.7 | Advisory | Required | 방어적 코딩 강화 |
| Rule 21.6 | Required | Disapplied | 디버그 빌드 전용 |

## 4. 검증 도구
- Cppcheck 2.12 (MISRA addon)
- 수동 코드 리뷰 (Directive 검증)

## 5. 검증 프로세스
- CI/CD: 매 커밋 시 자동 분석
- 코드 리뷰: PR 시 체크리스트 확인

## 6. Deviation 승인
- 요청: 개발자 → 기술 리더
- 승인: 기술 리더 검토 후 승인
```

---

## 5.3 Deviation (예외 처리)

### Deviation이란?

**Deviation**은 규칙 위반이 **불가피하거나 정당화**될 때 이를 공식적으로 문서화하는 것입니다.

### 카테고리별 Deviation 가능 여부

| 카테고리 | Deviation | 비고 |
|----------|-----------|------|
| Mandatory | ❌ 불가 | Undefined Behavior 유발 |
| Required | ✅ 가능 | 문서화 필수 |
| Advisory | ✅ 가능 | 문서화 권장 |

### Deviation 유형

| 유형 | 적용 범위 | 예시 |
|------|----------|------|
| Project | 프로젝트 전체 | "디버그 빌드에서 printf 허용" |
| File | 특정 파일 | "legacy.c 전체 Rule 10.3 예외" |
| Instance | 특정 위치 | "main.c:42 Rule 17.7 예외" |

---

## 5.4 Deviation Record 작성

### Deviation Record 템플릿

```markdown
# Deviation Record

## DEV-2025-001

| 항목 | 내용 |
|------|------|
| **ID** | DEV-2025-001 |
| **규칙** | Rule 21.6 (Required) |
| **규칙 설명** | Standard Library I/O functions shall not be used |
| **위치** | src/debug.c, Line 45-67 |
| **사유** | 개발 중 디버깅을 위해 printf 필요 |
| **위험 평가** | 릴리스 빌드에서 #ifdef로 제거됨, 위험 없음 |
| **대안 검토** | UART 출력 검토했으나 개발 효율 저하 |
| **승인자** | [기술 리더] |
| **승인일** | 2025-02-01 |

### 위반 코드
```c
#ifdef DEBUG
// cppcheck-suppress misra-c2012-21.6
printf("Debug: value = %d\n", value);  /* DEV-2025-001 */
#endif
```

### 완화 조치
- 릴리스 빌드에서 DEBUG 매크로 미정의
- CI에서 릴리스 빌드 시 printf 포함 여부 검사
```

---

## 5.5 Guideline Compliance Summary (GCS)

### GCS란?

**GCS**는 프로젝트의 **전체 MISRA 준수 현황**을 요약하는 문서입니다.

### GCS 템플릿

```markdown
# Guideline Compliance Summary (GCS)

## 1. 프로젝트 정보
- 프로젝트명: STM32 Motor Controller
- 버전: v1.0.0
- 분석일: 2025-02-01
- 분석 도구: Cppcheck 2.12

## 2. 준수 요약

| 카테고리 | 규칙 수 | 준수 | 위반 | Deviation | 준수율 |
|----------|--------|------|------|-----------|--------|
| Mandatory | 16 | 16 | 0 | - | 100% |
| Required | 143 | 140 | 0 | 3 | 100% |
| Advisory | 42 | 38 | 4 | - | 90% |
| **합계** | **201** | **194** | **4** | **3** | **98%** |

## 3. Deviation 목록

| ID | 규칙 | 위치 | 사유 요약 |
|----|------|------|----------|
| DEV-2025-001 | Rule 21.6 | debug.c | 디버그 출력 |
| DEV-2025-002 | Rule 21.3 | memory.c | 동적 메모리 풀 |
| DEV-2025-003 | Rule 17.2 | parser.c | 재귀 파서 필요 |

## 4. Advisory 위반 목록 (미처리)

| 규칙 | 위치 | 설명 |
|------|------|------|
| Rule 2.5 | config.h:15 | 미사용 매크로 (향후 사용 예정) |
| Rule 15.7 | main.c:102 | else 미종료 (의도적) |

## 5. 결론
본 프로젝트는 MISRA C:2023 Required 이상 규칙을 100% 준수합니다.
모든 위반 사항은 승인된 Deviation으로 처리되었습니다.

## 6. 승인

| 역할 | 이름 | 서명 | 일자 |
|------|------|------|------|
| 작성자 | | | |
| 검토자 | | | |
| 승인자 | | | |
```

---

## 5.6 Adopted Code (외부 코드) 처리

### Adopted Code란?

**Adopted Code**는 프로젝트 외부에서 가져온 코드입니다:
- 표준 라이브러리
- 서드파티 라이브러리
- 레거시 코드

### Adopted Code 처리 방법

```
┌─────────────────────────────────────────────────────────────────┐
│                Adopted Code 처리 전략                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. 완전 검증 (권장)                                             │
│     - 외부 코드도 MISRA 검사 수행                                │
│     - Deviation 문서화                                          │
│                                                                 │
│  2. 신뢰 기반 (제한적)                                           │
│     - 신뢰할 수 있는 소스의 코드                                  │
│     - 검증 범위에서 제외하되 문서화                               │
│                                                                 │
│  3. 래퍼 격리                                                   │
│     - 외부 코드를 래퍼 함수로 격리                                │
│     - 인터페이스만 MISRA 준수                                    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 래퍼 함수 예시

```c
/* hal_wrapper.c - HAL 라이브러리 래퍼 */

#include "stm32f1xx_hal.h"

/* 
 * HAL 함수 래퍼 - MISRA 준수 인터페이스 제공
 * Adopted Code: STM32 HAL Library
 */

/**
 * @brief UART 송신 래퍼
 * @param data 송신 데이터 포인터
 * @param size 데이터 크기
 * @return 0: 성공, -1: 실패
 */
int32_t uart_transmit(const uint8_t *data, uint16_t size) {
    int32_t result = -1;
    
    if (data != NULL) {
        HAL_StatusTypeDef hal_status;
        
        /* HAL 함수 호출 - Adopted Code */
        hal_status = HAL_UART_Transmit(&huart2, (uint8_t *)data, size, 1000U);
        
        if (hal_status == HAL_OK) {
            result = 0;
        }
    }
    
    return result;
}
```

---

## 5.7 컴플라이언스 체크리스트

### 프로젝트 시작 시

- [ ] GEP 문서 작성
- [ ] 규칙 재분류 결정
- [ ] 정적 분석 도구 설정
- [ ] CI/CD 파이프라인 구성
- [ ] Deviation 승인 프로세스 정의

### 개발 중

- [ ] 매 커밋 시 정적 분석 실행
- [ ] 위반 발생 시 즉시 수정 또는 Deviation 요청
- [ ] 코드 리뷰 시 MISRA 체크리스트 확인
- [ ] Directive 관련 설계 문서 유지

### 릴리스 전

- [ ] 전체 코드베이스 정적 분석
- [ ] 모든 Deviation Record 검토
- [ ] GCS 문서 작성
- [ ] 최종 승인 획득

---

## 📝 학습 확인 문제

### Q1. Mandatory 규칙에 Deviation이 불가능한 이유는?

<details>
<summary>정답 보기</summary>

Mandatory 규칙은 **Undefined Behavior**를 직접적으로 유발하는 코드를 금지합니다. Undefined Behavior가 발생하면 프로그램의 동작을 예측할 수 없으므로, 안전성을 보장할 수 없습니다.

</details>

### Q2. GEP와 GCS의 차이점은?

<details>
<summary>정답 보기</summary>

- **GEP (Guideline Enforcement Plan)**: 프로젝트 **시작 시** 작성, 어떻게 규칙을 적용할지 **계획**
- **GCS (Guideline Compliance Summary)**: 프로젝트 **완료 시** 작성, 규칙 준수 **결과** 요약

</details>

### Q3. Deviation Record의 필수 항목 5가지는?

<details>
<summary>정답 보기</summary>

1. 식별자 (ID)
2. 위반 규칙
3. 위반 위치
4. Deviation 사유
5. 승인 정보 (승인자, 승인일)

추가 권장: 위험 평가, 대안 검토

</details>

---

## 📚 다음 학습

다음 장에서는 STM32 기반 실습 예제를 통해 MISRA-C를 실제 프로젝트에 적용합니다.

[다음: 06. 실습 예제 →](06-practical-examples.md)

---

## 🔗 참고 자료

- [MISRA Compliance:2020 (무료 PDF)](https://misra.org.uk/app/uploads/2021/06/MISRA-Compliance-2020.pdf)
- [MISRA 공식 사이트](https://misra.org.uk/)
