# 04. 정적 분석 도구

[← 이전: 주요 규칙 상세](03-common-rules.md) | [메인으로](../README.md) | [다음: 준수 및 문서화 →](05-compliance.md)

---

## 📚 학습 목표

이 장을 완료하면 다음을 수행할 수 있습니다:
- 정적 분석의 원리와 한계 이해
- Cppcheck MISRA addon 설치 및 설정
- 명령줄 및 IDE에서 MISRA 검사 실행
- CI/CD 파이프라인에 정적 분석 통합

---

## 4.1 정적 분석 개요

### 정적 분석이란?

**정적 분석(Static Analysis)**은 프로그램을 **실행하지 않고** 소스 코드를 분석하여 잠재적 결함을 찾는 기법입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                    소프트웨어 검증 방법                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌───────────────┐     ┌───────────────┐     ┌─────────────┐  │
│   │  정적 분석     │     │  동적 분석     │     │  코드 리뷰   │  │
│   │ Static        │     │ Dynamic       │     │ Code Review │  │
│   └───────┬───────┘     └───────┬───────┘     └──────┬──────┘  │
│           │                     │                    │         │
│           ▼                     ▼                    ▼         │
│   • 코드 실행 없음        • 코드 실행 필요       • 수동 검토     │
│   • 자동화 가능          • 테스트 케이스 필요    • 전문성 필요   │
│   • 빠른 피드백          • 런타임 오류 검출      • 깊은 분석     │
│   • 모든 경로 검사 가능   • 실제 동작 확인       • 설계 문제 발견 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### MISRA 정적 분석 도구 비교

| 도구 | 유형 | MISRA 지원 | 장점 | 단점 |
|------|------|-----------|------|------|
| **Cppcheck** | 오픈소스 | Addon | 무료, 쉬운 사용 | 제한된 규칙 |
| **PC-lint Plus** | 상용 | 전체 | 깊은 분석 | 유료 |
| **Polyspace** | 상용 | 전체 | 런타임 오류 증명 | 고가 |
| **Helix QAC** | 상용 | 전체 | 업계 표준 | 고가 |
| **LDRA** | 상용 | 전체 | 인증 지원 | 고가 |

---

## 4.2 Cppcheck 설치

### Linux (Ubuntu/Debian)

```bash
# 패키지 관리자로 설치
sudo apt update
sudo apt install cppcheck

# 버전 확인
cppcheck --version
```

### Windows

```powershell
# Chocolatey로 설치
choco install cppcheck

# 또는 공식 설치 프로그램 다운로드
# https://github.com/danmar/cppcheck/releases
```

### macOS

```bash
brew install cppcheck
```

---

## 4.3 MISRA Addon 설정

### Rule Text 파일 생성

```bash
mkdir -p ~/.config/cppcheck
cat > ~/.config/cppcheck/misra_rules.txt << 'EOF'
Rule 1.3 Required
There shall be no occurrence of undefined or critical unspecified behaviour.

Rule 10.3 Required
The value of an expression shall not be assigned to a narrower essential type.

Rule 15.6 Required
The body of an iteration-statement shall be a compound-statement.

Rule 17.7 Required
The value returned by a function shall be used.

Rule 21.3 Required
Memory allocation functions shall not be used.

Rule 21.6 Required
Standard Library I/O functions shall not be used.
EOF
```

---

## 4.4 Cppcheck 사용법

### 기본 명령어

```bash
# 단일 파일 검사
cppcheck main.c

# MISRA addon 적용
cppcheck --addon=misra src/main.c

# 전체 검사 옵션
cppcheck --addon=misra --enable=all --std=c99 -I include src/

# XML 보고서 출력
cppcheck --addon=misra --xml --xml-version=2 src/ 2> report.xml

# CI용 (에러 시 종료코드 1)
cppcheck --addon=misra --error-exitcode=1 src/
```

---

## 4.5 실습: 예제 코드 검사

### 테스트 코드 작성

```c
/* test_violations.c - MISRA 위반 예제 */
#include <stdio.h>
#include <stdlib.h>

#define SQUARE(x) x * x    /* Rule 20.7 위반 */

int get_value(void) { return 42; }

int main(void) {
    int arr[10];
    
    for (int i = 0; i < 10; i++)   /* Rule 15.6 위반 */
        arr[i] = SQUARE(i + 1);
    
    unsigned char small = arr[0];  /* Rule 10.3 위반 */
    
    int *ptr = malloc(40);         /* Rule 21.3 위반 */
    
    get_value();                   /* Rule 17.7 위반 */
    
    printf("Value: %d\n", small);  /* Rule 21.6 위반 */
    
    free(ptr);
    return 0;
}
```

### 검사 실행

```bash
cppcheck --addon=misra test_violations.c
```

---

## 4.6 IDE 통합

### VS Code 설정

```json
// .vscode/settings.json
{
    "cppcheck.enable": true,
    "cppcheck.miscellaneousArguments": "--addon=misra --std=c99"
}
```

---

## 4.7 CI/CD 통합

### GitHub Actions

```yaml
# .github/workflows/misra.yml
name: MISRA Check

on: [push, pull_request]

jobs:
  analyze:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
    
    - name: Install Cppcheck
      run: sudo apt-get install -y cppcheck
    
    - name: Run MISRA Check
      run: cppcheck --addon=misra --error-exitcode=1 src/
```

---

## 4.8 Suppression 설정

### 인라인 Suppression

```c
// cppcheck-suppress misra-c2012-21.6
printf("Debug message\n");
```

### Suppression 파일

```
# suppressions.txt
misra-c2012-21.6:src/debug.c
misra-*:test/*.c
```

```bash
cppcheck --addon=misra --suppressions-list=suppressions.txt src/
```

---

## 📝 학습 확인 문제

### Q1. MISRA addon 사용 명령은?

<details>
<summary>정답</summary>

```bash
cppcheck --addon=misra src/main.c
```

</details>

### Q2. CI에서 위반 시 빌드 실패하게 하려면?

<details>
<summary>정답</summary>

```bash
cppcheck --addon=misra --error-exitcode=1 src/
```

</details>

---

## 📚 다음 학습

[다음: 05. 준수 및 문서화 →](05-compliance.md)

---

## 🔗 참고 자료

- [Cppcheck 공식](https://cppcheck.sourceforge.io/)
- [Cppcheck MISRA addon](https://github.com/danmar/cppcheck)
