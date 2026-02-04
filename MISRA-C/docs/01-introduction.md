# 01. MISRA-C 소개

[← 메인으로 돌아가기](../README.md) | [다음: 규칙 분류 체계 →](02-rule-categories.md)

---

## 📚 학습 목표

이 장을 완료하면 다음을 이해할 수 있습니다:
- MISRA-C의 정의와 목적
- MISRA-C 발전 역사
- C 언어의 위험 요소
- MISRA-C 적용이 필요한 산업 분야

---

## 1.1 MISRA-C란 무엇인가?

### 정의

**MISRA-C**는 **Motor Industry Software Reliability Association**에서 개발한 C 프로그래밍 언어 코딩 가이드라인입니다.

> "Guidelines for the use of the C language in critical systems"
> — MISRA C:2023 공식 명칭

### 핵심 목적

```
┌────────────────────────────────────────────────────────────────┐
│                     MISRA-C의 4대 목표                          │
├────────────────────────────────────────────────────────────────┤
│  1. 안전성 (Safety)                                            │
│     - Undefined Behavior 방지                                  │
│     - Runtime Error 최소화                                     │
│                                                                │
│  2. 보안성 (Security)                                          │
│     - Buffer Overflow 방지                                     │
│     - Memory Corruption 예방                                   │
│                                                                │
│  3. 이식성 (Portability)                                       │
│     - Implementation-defined Behavior 제어                     │
│     - 플랫폼 독립적 코드 작성                                   │
│                                                                │
│  4. 유지보수성 (Maintainability)                               │
│     - 코드 가독성 향상                                         │
│     - 일관된 코딩 스타일                                       │
└────────────────────────────────────────────────────────────────┘
```

---

## 1.2 왜 MISRA-C가 필요한가?

### C 언어의 문제점

C 언어는 강력하지만, 다음과 같은 위험 요소가 있습니다:

#### 1. 정의되지 않은 동작 (Undefined Behavior)

```c
/* ❌ 위험한 코드 - Undefined Behavior */
int arr[10];
int value = arr[15];    // 배열 범위 초과 접근

int x = INT_MAX;
x = x + 1;              // 부호 있는 정수 오버플로우

int *ptr = NULL;
*ptr = 10;              // NULL 포인터 역참조
```

**ISO C 표준**에는 200개 이상의 Undefined Behavior가 정의되어 있습니다. 컴파일러는 이러한 코드에 대해 어떤 동작도 할 수 있으며, 이는 예측 불가능한 결과를 초래합니다.

#### 2. 구현 정의 동작 (Implementation-defined Behavior)

```c
/* ⚠️ 결과가 컴파일러에 따라 달라지는 코드 */
int result = -5 >> 1;   // 음수 우측 시프트 - 컴파일러마다 다름

char c = 128;           // char가 signed인지 unsigned인지 컴파일러 의존

sizeof(int);            // 2바이트? 4바이트? 플랫폼 의존
```

#### 3. 미지정 동작 (Unspecified Behavior)

```c
/* ⚠️ 실행 순서가 정의되지 않은 코드 */
int i = 0;
int result = i++ + i++;     // 평가 순서 미지정

void foo(int a, int b);
foo(i++, i++);              // 인자 평가 순서 미지정
```

### 실제 사고 사례

| 사고 | 원인 | 결과 |
|------|------|------|
| 도요타 급발진 (2009) | 스택 오버플로우, MISRA 위반 | 89명 사망, $12억 배상 |
| Therac-25 방사선 사고 (1985-1987) | Race Condition | 6명 사망/부상 |
| 아리안 5 폭발 (1996) | 정수 오버플로우 | $3.7억 손실 |

---

## 1.3 MISRA-C 발전 역사

### 타임라인

```
1994 ──────────────────────────────────────────────────────────────▶
      │
      ▼
  MISRA 프로젝트 시작
  (영국 정부 지원, 자동차 산업)
      │
1998 ─┼──────────────────────────────────────────────────────────▶
      │
      ▼
  MISRA-C:1998 출시
  - 127개 규칙 (93 Required, 34 Advisory)
  - C90 지원
      │
2004 ─┼──────────────────────────────────────────────────────────▶
      │
      ▼
  MISRA-C:2004 출시
  - 141개 규칙 (121 Required, 20 Advisory)
  - 규칙 재구성 및 명확화
      │
2013 ─┼──────────────────────────────────────────────────────────▶
      │
      ▼
  MISRA-C:2012 출시
  - 143 Rules + 16 Directives
  - C99 지원 추가
  - Mandatory 카테고리 도입
      │
2023 ─┼──────────────────────────────────────────────────────────▶
      │
      ▼
  MISRA-C:2023 출시
  - 201 Rules + 21 Directives (총 222개)
  - C11, C18 지원
  - 멀티스레딩, Atomic 규칙 추가
      │
2025 ─┼──────────────────────────────────────────────────────────▶
      │
      ▼
  MISRA-C:2025 출시
  - 225개 가이드라인
  - C23 호환성 기반 마련
```

### 주요 변화 내용

| 버전 | 주요 변화 |
|------|----------|
| 1998 → 2004 | 규칙 명확화, 도구 검증 의무화 |
| 2004 → 2012 | Directive 개념 도입, Mandatory 카테고리 추가 |
| 2012 → 2023 | C11/C18 지원, 멀티스레딩 규칙 (AMD4) |
| 2023 → 2025 | C23 호환성, 동시성 규칙 개선 |

---

## 1.4 MISRA-C 적용 산업

### 산업별 표준 매핑

```
┌─────────────────────────────────────────────────────────────────┐
│                        산업별 기능안전 표준                       │
├─────────────┬───────────────────────────────────────────────────┤
│   자동차     │  ISO 26262 (Road Vehicles - Functional Safety)   │
│             │  ASIL A ~ ASIL D                                  │
├─────────────┼───────────────────────────────────────────────────┤
│   항공우주   │  DO-178C (Software Considerations in Airborne    │
│             │  Systems and Equipment Certification)             │
│             │  Level A ~ Level E                                │
├─────────────┼───────────────────────────────────────────────────┤
│   의료기기   │  IEC 62304 (Medical Device Software)             │
│             │  Class A ~ Class C                                │
├─────────────┼───────────────────────────────────────────────────┤
│   철도      │  EN 50128 (Railway - Software for Railway         │
│             │  Control and Protection Systems)                  │
│             │  SIL 0 ~ SIL 4                                    │
├─────────────┼───────────────────────────────────────────────────┤
│   산업자동화 │  IEC 61508 (Functional Safety of E/E/PE          │
│             │  Safety-related Systems)                          │
│             │  SIL 1 ~ SIL 4                                    │
└─────────────┴───────────────────────────────────────────────────┘
```

### ISO 26262와 MISRA-C 관계

ISO 26262 Part 6 (Software Development)에서는 MISRA-C 사용을 권장합니다:

| ASIL 등급 | 코딩 가이드라인 요구사항 |
|-----------|------------------------|
| ASIL A | 권장 (Recommended) |
| ASIL B | 강력 권장 (Highly Recommended) |
| ASIL C | 강력 권장 (Highly Recommended) |
| ASIL D | 강력 권장 (Highly Recommended) |

---

## 1.5 MISRA-C 문서 구성

### MISRA C:2023 문서 구조

```
MISRA C:2023
├── Part 1: Introduction
│   ├── 목적 및 범위
│   ├── 용어 정의
│   └── 가이드라인 사용 방법
│
├── Part 2: The Guidelines
│   ├── Directives (지시사항) - 21개
│   │   └── 코드만으로 완전히 검증 불가
│   │
│   └── Rules (규칙) - 201개
│       └── 코드 분석으로 검증 가능
│
├── Part 3: Compliance
│   ├── 컴플라이언스 정의
│   ├── Deviation 처리
│   └── 문서화 요구사항
│
└── Appendices
    ├── 규칙 요약표
    ├── Undefined Behavior 목록
    └── 용어집
```

### 가이드라인 식별자

```
Rule 11.3 (Required)
  │    │      │
  │    │      └── 카테고리: Mandatory/Required/Advisory
  │    └── 규칙 번호: 3
  └── 그룹 번호: 11 (Pointer Type Conversions)

Dir 4.1 (Required)
  │   │      │
  │   │      └── 카테고리
  │   └── 지시사항 번호
  └── Directive (지시사항)
```

---

## 1.6 MISRA-C의 장단점

### 장점

| 장점 | 설명 |
|------|------|
| **버그 감소** | 컴파일 전 잠재적 오류 검출 |
| **코드 품질** | 일관된 코딩 스타일 강제 |
| **안전성 인증** | 기능안전 표준 준수 지원 |
| **유지보수성** | 가독성 높은 코드 |
| **팀 협업** | 명확한 코딩 규칙 |

### 단점 및 도전 과제

| 도전 과제 | 대응 방안 |
|----------|----------|
| 학습 곡선 | 단계적 도입, 교육 |
| 개발 속도 저하 | 자동화 도구 활용 |
| 레거시 코드 | Baseline 설정, 점진적 개선 |
| 도구 비용 | 오픈소스 도구 활용 (Cppcheck) |
| False Positive | 도구 설정 튜닝, Deviation 문서화 |

---

## 📝 학습 확인 문제

### Q1. MISRA-C의 주요 목적 4가지를 나열하세요.

<details>
<summary>정답 보기</summary>

1. **안전성 (Safety)** - Undefined Behavior 방지
2. **보안성 (Security)** - Buffer Overflow 등 취약점 예방
3. **이식성 (Portability)** - 플랫폼 독립적 코드
4. **유지보수성 (Maintainability)** - 가독성 높은 코드

</details>

### Q2. 다음 코드의 문제점과 관련된 C 언어 동작 유형은?

```c
int a = 5, b = 0;
int result = a / b;
```

<details>
<summary>정답 보기</summary>

**Undefined Behavior (정의되지 않은 동작)**

0으로 나누기는 C 표준에서 정의되지 않은 동작입니다. 프로그램이 크래시하거나, 예상치 못한 값을 반환하거나, 아무 일도 일어나지 않을 수 있습니다.

</details>

### Q3. MISRA-C:2023에서 총 가이드라인 수는?

<details>
<summary>정답 보기</summary>

**222개** (21 Directives + 201 Rules)

</details>

### Q4. ISO 26262 ASIL D에서 MISRA-C 사용은?

<details>
<summary>정답 보기</summary>

**강력 권장 (Highly Recommended)**

ISO 26262 Part 6에서 ASIL B 이상의 경우 코딩 가이드라인 사용을 강력 권장합니다.

</details>

---

## 📚 다음 학습

다음 장에서는 MISRA-C 규칙의 분류 체계에 대해 자세히 알아봅니다:
- Mandatory / Required / Advisory 카테고리
- Directive vs Rule
- Decidable vs Undecidable

[다음: 02. 규칙 분류 체계 →](02-rule-categories.md)

---

## 🔗 참고 자료

- [MISRA 공식 웹사이트](https://misra.org.uk/)
- [MISRA C:2023 소개](https://misra.org.uk/misra-c2023-released/)
- [ISO 26262 개요](https://www.iso.org/standard/68383.html)
