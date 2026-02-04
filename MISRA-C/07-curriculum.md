# 07. 교육 커리큘럼

[← 이전: 실습 예제](06-practical-examples.md) | [메인으로](../README.md)

---

## 📚 교육 개요

### 교육 정보

| 항목 | 내용 |
|------|------|
| **교육명** | MISRA-C 코딩 가이드라인 실무 과정 |
| **대상** | 임베디드 시스템 개발자, 품질 엔지니어 |
| **기간** | 5일 (총 20시간) |
| **선수지식** | C 프로그래밍 기초, 임베디드 시스템 개념 |
| **준비물** | PC, STM32 개발보드, STM32CubeIDE |

### 교육 목표

```
┌─────────────────────────────────────────────────────────────────┐
│                     교육 완료 후 역량                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ✅ MISRA-C 규칙 체계 이해 및 적용                               │
│  ✅ 정적 분석 도구를 활용한 코드 검사                            │
│  ✅ 규칙 위반 코드 식별 및 수정                                  │
│  ✅ 컴플라이언스 문서 작성                                       │
│  ✅ 실제 프로젝트에 MISRA-C 도입                                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📅 일정별 상세 커리큘럼

### Day 1: MISRA-C 기초 (4시간)

| 시간 | 주제 | 내용 | 자료 |
|------|------|------|------|
| 09:00-10:00 | 오리엔테이션 | 교육 소개, 환경 설정 확인 | - |
| 10:00-11:00 | MISRA-C 소개 | 역사, 목적, 적용 분야 | [01-introduction.md](01-introduction.md) |
| 11:00-12:00 | C 언어 위험 요소 | Undefined Behavior, 실제 사고 사례 | [01-introduction.md](01-introduction.md) |
| 13:00-14:00 | 규칙 분류 체계 | Directive/Rule, 카테고리 | [02-rule-categories.md](02-rule-categories.md) |
| 14:00-15:00 | 규칙 분류 실습 | 규칙 분류 연습, 퀴즈 | 실습 자료 |

**Day 1 학습 목표:**
- MISRA-C의 필요성 설명 가능
- Mandatory/Required/Advisory 구분 가능
- Directive와 Rule 차이점 설명 가능

---

### Day 2: 핵심 규칙 학습 Part 1 (4시간)

| 시간 | 주제 | 내용 | 자료 |
|------|------|------|------|
| 09:00-10:00 | Essential Type Model | 타입 분류, 변환 규칙 | [03-common-rules.md](03-common-rules.md) |
| 10:00-11:00 | 타입 관련 규칙 | Rule 10.x 시리즈 실습 | 실습 코드 |
| 11:00-12:00 | 포인터 규칙 | Rule 11.x, 18.x 시리즈 | [03-common-rules.md](03-common-rules.md) |
| 13:00-14:00 | 포인터 실습 | 안전한 포인터 사용법 | 실습 코드 |
| 14:00-15:00 | 코드 리뷰 실습 | 위반 코드 찾기 연습 | 연습 문제 |

**Day 2 학습 목표:**
- Essential Type 6가지 분류 가능
- Narrowing 변환 식별 및 수정 가능
- 포인터 관련 위험 패턴 식별 가능

---

### Day 3: 핵심 규칙 학습 Part 2 (4시간)

| 시간 | 주제 | 내용 | 자료 |
|------|------|------|------|
| 09:00-10:00 | 제어문 규칙 | Rule 14.x, 15.x, 16.x | [03-common-rules.md](03-common-rules.md) |
| 10:00-11:00 | 함수 규칙 | Rule 17.x, 재귀 금지 | [03-common-rules.md](03-common-rules.md) |
| 11:00-12:00 | 전처리기 규칙 | Rule 20.x, 매크로 안전 사용 | [03-common-rules.md](03-common-rules.md) |
| 13:00-14:00 | 표준 라이브러리 규칙 | Rule 21.x, 금지 함수 | [03-common-rules.md](03-common-rules.md) |
| 14:00-15:00 | 종합 실습 | 다양한 규칙 위반 수정 | 연습 문제 |

**Day 3 학습 목표:**
- switch/if/for 문 올바른 작성
- 매크로 안전하게 정의
- 금지된 표준 함수의 대안 구현

---

### Day 4: 정적 분석 도구 실습 (4시간)

| 시간 | 주제 | 내용 | 자료 |
|------|------|------|------|
| 09:00-10:00 | Cppcheck 설치 | 설치, MISRA addon 설정 | [04-static-analysis.md](04-static-analysis.md) |
| 10:00-11:00 | Cppcheck 실습 | 명령줄 사용, 옵션 | [04-static-analysis.md](04-static-analysis.md) |
| 11:00-12:00 | IDE 통합 | VS Code, STM32CubeIDE | [04-static-analysis.md](04-static-analysis.md) |
| 13:00-14:00 | 컴플라이언스 문서 | GEP, GCS, Deviation | [05-compliance.md](05-compliance.md) |
| 14:00-15:00 | 문서 작성 실습 | Deviation Record 작성 | 템플릿 |

**Day 4 학습 목표:**
- Cppcheck MISRA 검사 실행 가능
- 위반 보고서 해석 가능
- Deviation Record 작성 가능

---

### Day 5: 종합 실습 및 평가 (4시간)

| 시간 | 주제 | 내용 | 자료 |
|------|------|------|------|
| 09:00-10:30 | 종합 프로젝트 | STM32 MISRA 준수 프로젝트 | [06-practical-examples.md](06-practical-examples.md) |
| 10:30-12:00 | 코드 리뷰 | 팀별 코드 상호 리뷰 | - |
| 13:00-14:00 | 필기 평가 | 이론 시험 (30문항) | 평가지 |
| 14:00-15:00 | 실습 평가 | 코드 수정 실습 시험 | 평가지 |
| 15:00-16:00 | 마무리 | 평가 리뷰, Q&A, 수료 | - |

**Day 5 학습 목표:**
- MISRA 준수 코드 직접 작성
- 동료 코드 리뷰 수행
- 평가 합격 (70점 이상)

---

## 📝 평가 방법

### 평가 구성

| 항목 | 배점 | 합격 기준 |
|------|------|----------|
| 필기 시험 | 50점 | 35점 이상 |
| 실습 시험 | 50점 | 35점 이상 |
| **총점** | **100점** | **70점 이상** |

### 필기 시험 (50점, 30문항)

**문제 유형:**
- 객관식 (4지선다): 20문항 × 2점 = 40점
- 단답형: 5문항 × 2점 = 10점

**출제 범위:**

| 영역 | 문항 수 | 배점 |
|------|--------|------|
| MISRA-C 개요 | 5 | 10점 |
| 규칙 분류 체계 | 5 | 10점 |
| 주요 규칙 | 10 | 20점 |
| 정적 분석/컴플라이언스 | 5 | 10점 |

### 실습 시험 (50점)

**문제 유형:**
- 위반 코드 식별 및 수정: 30점
- MISRA 준수 코드 작성: 20점

**평가 기준:**

| 기준 | 배점 |
|------|------|
| 위반 규칙 정확히 식별 | 10점 |
| 올바른 수정 방법 적용 | 10점 |
| 수정 코드 컴파일 성공 | 5점 |
| 정적 분석 통과 | 5점 |
| 신규 코드 MISRA 준수 | 15점 |
| 코드 품질 및 가독성 | 5점 |

---

## 📋 예시 평가 문제

### 필기 시험 예시

**[객관식] 다음 중 Mandatory 규칙에 대한 설명으로 옳은 것은?**

1. Deviation 문서화 시 위반 가능
2. 어떤 경우에도 위반 불가
3. Advisory보다 우선순위 낮음
4. 정적 분석 도구로 검출 불가

<details>
<summary>정답</summary>

**2번**: Mandatory 규칙은 Undefined Behavior를 직접 유발하므로 어떤 경우에도 위반이 허용되지 않습니다.

</details>

---

**[객관식] Rule 15.6 "복합문 사용"을 위반하는 코드는?**

```c
// A
if (x > 0) {
    y = 1;
}

// B
if (x > 0)
    y = 1;

// C
for (int i = 0; i < 10; i++) {
    arr[i] = 0;
}

// D
while (flag) {
    process();
}
```

<details>
<summary>정답</summary>

**B**: if 문 본문이 복합문(중괄호)으로 감싸지 않았습니다.

</details>

---

**[단답형] Essential Type 6가지를 나열하시오.**

<details>
<summary>정답</summary>

1. Boolean
2. Signed
3. Unsigned
4. Floating
5. Character
6. Enum

</details>

---

### 실습 시험 예시

**다음 코드의 MISRA 위반을 모두 찾고 수정하시오. (30점)**

```c
#include <stdio.h>
#include <stdlib.h>

#define ABS(x) x < 0 ? -x : x

int data[100];

int sum_array(int *arr, int len)
{
    int sum = 0;
    
    for (int i = 0; i <= len; i++)
        sum += arr[i];
    
    if (len > 50)
        printf("Large array: %d elements\n", len);
    
    return sum;
}

int main()
{
    int *dynamic = malloc(100 * sizeof(int));
    int result;
    
    for (int i = 0; i < 100; i++)
        data[i] = ABS(i - 50);
    
    sum_array(data, 100);
    
    free(dynamic);
}
```

<details>
<summary>채점 기준</summary>

| 위반 | 규칙 | 배점 |
|------|------|------|
| stdio.h 사용 | Rule 21.6 | 3점 |
| stdlib.h malloc/free | Rule 21.3 | 3점 |
| 매크로 괄호 누락 | Rule 20.7 | 3점 |
| for 복합문 미사용 (2개) | Rule 15.6 | 4점 |
| if 복합문 미사용 | Rule 15.6 | 2점 |
| 배열 범위 초과 | Rule 18.1 | 5점 |
| main 프로토타입 | Rule 8.4 | 2점 |
| 반환값 미사용 | Rule 17.7 | 3점 |
| 올바른 수정 | - | 5점 |

</details>

---

## 📚 참고 도서 및 자료

### 필수 자료
- MISRA C:2023 (유료, [misra.org.uk](https://misra.org.uk/))
- MISRA Compliance:2020 (무료 PDF)

### 권장 도서
- "Embedded C Coding Standard" - Michael Barr
- "Writing Solid Code" - Steve Maguire
- "C Programming FAQs" - Steve Summit

### 온라인 자료
- [Cppcheck 공식 문서](https://cppcheck.sourceforge.io/)
- [LDRA MISRA 블로그](https://ldra.com/blog/)
- [Barr Group 코딩 표준](https://barrgroup.com/embedded-systems/books/embedded-c-coding-standard)

---

## 🎓 수료증 발급

### 수료 조건
- 출석률 80% 이상
- 평가 점수 70점 이상
- 모든 실습 과제 제출

### 수료증 내용
```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                         수 료 증                                │
│                                                                 │
│                                                                 │
│   성명: _______________                                         │
│                                                                 │
│   위 사람은 MISRA-C 코딩 가이드라인 실무 과정을                  │
│   성실히 이수하였기에 이 증서를 수여합니다.                      │
│                                                                 │
│   교육 기간: 2025년 _월 _일 ~ 2025년 _월 _일                    │
│   교육 시간: 20시간                                              │
│   평가 점수: ___점                                               │
│                                                                 │
│                              2025년  월  일                     │
│                                                                 │
│                              [교육 기관명]                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📞 교육 문의

### 교육 담당자
- 이메일: training@example.com
- 전화: 02-XXX-XXXX

### FAQ

**Q: MISRA 가이드라인 문서를 꼭 구매해야 하나요?**
A: 교육 중에는 교육 자료로 학습하지만, 실무 적용 시에는 공식 문서 구매를 권장합니다.

**Q: Cppcheck 외에 다른 도구도 배우나요?**
A: 교육에서는 오픈소스인 Cppcheck를 주로 다루지만, 상용 도구 소개도 포함됩니다.

**Q: 교육 후 지원이 있나요?**
A: 수료 후 3개월간 이메일 Q&A 지원을 제공합니다.

---

## 📎 부록: 교육 자료 체크리스트

### 강사 준비물
- [ ] 교육 자료 프린트
- [ ] 실습 코드 USB
- [ ] 평가지
- [ ] 수료증 양식

### 수강생 준비물
- [ ] 노트북 (STM32CubeIDE 설치)
- [ ] STM32 개발보드
- [ ] USB 케이블
- [ ] 필기구

---

[메인으로 돌아가기 →](../README.md)
