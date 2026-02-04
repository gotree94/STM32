# MISRA-C 교육 자료

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![MISRA C:2023](https://img.shields.io/badge/MISRA_C-2023-blue.svg)](https://misra.org.uk/)

임베디드 시스템 개발을 위한 MISRA-C 코딩 가이드라인 교육 자료입니다.

## 📋 목차

| 문서 | 내용 | 소요시간 |
|------|------|----------|
| [01. MISRA-C 소개](docs/01-introduction.md) | 개요, 역사, 적용 분야 | 1시간 |
| [02. 규칙 분류 체계](docs/02-rule-categories.md) | Mandatory/Required/Advisory, Directives/Rules | 1시간 |
| [03. 주요 규칙 상세](docs/03-common-rules.md) | 핵심 규칙 설명 및 코드 예제 | 3시간 |
| [04. 정적 분석 도구](docs/04-static-analysis.md) | Cppcheck, PC-lint, 도구 활용법 | 2시간 |
| [05. 준수 및 문서화](docs/05-compliance.md) | 컴플라이언스 프로세스, Deviation | 1시간 |
| [06. 실습 예제](docs/06-practical-examples.md) | STM32 기반 실습 프로젝트 | 3시간 |
| [07. 교육 커리큘럼](docs/07-curriculum.md) | 교육 일정 및 평가 방법 | - |
| [08. IDE 툴 가이드](docs/08-ide-tools-guide.md) | IDE 툴 가이드 | - |

## 🎯 교육 목표

이 교육을 완료하면 다음을 수행할 수 있습니다:

1. **MISRA-C의 목적과 필요성** 이해
2. **규칙 분류 체계** (Mandatory/Required/Advisory) 이해
3. **핵심 규칙**을 코드에 적용
4. **정적 분석 도구**를 사용한 규칙 위반 검출
5. **Deviation 문서화** 및 컴플라이언스 증명

## 📚 MISRA-C란?

**MISRA-C**는 Motor Industry Software Reliability Association에서 개발한 C 언어 코딩 가이드라인입니다.

### 주요 특징

- **안전성**: 정의되지 않은 동작(Undefined Behavior) 방지
- **이식성**: 플랫폼 독립적인 코드 작성
- **신뢰성**: 런타임 오류 최소화
- **유지보수성**: 가독성 높은 코드 작성

### 적용 분야

```
┌─────────────────────────────────────────────────────────────┐
│                    MISRA-C 적용 산업                         │
├─────────────┬─────────────┬─────────────┬─────────────────┤
│  자동차      │  항공우주    │  의료기기    │  철도/산업설비   │
│  ISO 26262  │  DO-178C    │  IEC 62304  │  EN 50128      │
└─────────────┴─────────────┴─────────────┴─────────────────┘
```

## 📊 버전 히스토리

| 버전 | 출시년도 | 규칙 수 | 지원 C 표준 |
|------|----------|---------|------------|
| MISRA-C:1998 | 1998 | 127 | C90 |
| MISRA-C:2004 | 2004 | 141 | C90 |
| MISRA-C:2012 | 2013 | 143 Rules + 16 Directives | C90, C99 |
| MISRA-C:2023 | 2023 | 201 Rules + 21 Directives | C90, C99, C11, C18 |
| MISRA-C:2025 | 2025 | 225 Guidelines | C90, C99, C11, C18 |

## 🛠️ 필요 환경

### 하드웨어
- STM32 개발보드 (STM32F103 또는 STM32F411 권장)
- ST-Link 프로그래머

### 소프트웨어
- STM32CubeIDE (최신 버전)
- Cppcheck (MISRA addon 포함)
- Git

### 설치 스크립트

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install cppcheck

# Cppcheck MISRA addon 확인
cppcheck --addon=misra --version
```

## 📁 디렉토리 구조

```
misra-c-training/
├── README.md                    # 메인 문서 (현재 파일)
├── docs/
│   ├── 01-introduction.md       # MISRA-C 소개
│   ├── 02-rule-categories.md    # 규칙 분류 체계
│   ├── 03-common-rules.md       # 주요 규칙 상세
│   ├── 04-static-analysis.md    # 정적 분석 도구
│   ├── 05-compliance.md         # 준수 및 문서화
│   ├── 06-practical-examples.md # 실습 예제
│   └── 07-curriculum.md         # 교육 커리큘럼
└── examples/
    ├── violations/              # 규칙 위반 예제
    ├── compliant/               # 준수 예제
    └── stm32/                   # STM32 실습 프로젝트
```

## 🚀 빠른 시작

### 1. 저장소 클론

```bash
git clone https://github.com/your-username/misra-c-training.git
cd misra-c-training
```

### 2. 첫 번째 검사 실행

```bash
# 예제 코드 MISRA 검사
cppcheck --addon=misra examples/violations/example1.c
```

### 3. 교육 문서 순서대로 학습

1. [01-introduction.md](docs/01-introduction.md) 부터 시작
2. 각 장의 실습 예제 수행
3. [07-curriculum.md](docs/07-curriculum.md)의 평가 문제 풀이

## 📖 참고 자료

### 공식 문서
- [MISRA 공식 웹사이트](https://misra.org.uk/)
- [MISRA C:2023 가이드라인](https://misra.org.uk/product/misra-c2023/) (유료)
- [MISRA Compliance:2020](https://misra.org.uk/app/uploads/2021/06/MISRA-Compliance-2020.pdf) (무료 PDF)

### 도구
- [Cppcheck](https://cppcheck.sourceforge.io/) - 오픈소스 정적 분석 도구
- [PC-lint Plus](https://pclintplus.com/) - 상용 정적 분석 도구
- [Polyspace](https://www.mathworks.com/products/polyspace.html) - MathWorks 정적 분석 도구

### 관련 표준
- ISO 26262 - 자동차 기능안전
- IEC 61508 - 기능안전 일반
- DO-178C - 항공 소프트웨어 안전

## 📝 교육 일정 요약

| 일차 | 주제 | 시간 |
|------|------|------|
| 1일차 | MISRA-C 개요 및 규칙 분류 | 4시간 |
| 2일차 | 핵심 규칙 학습 (Part 1) | 4시간 |
| 3일차 | 핵심 규칙 학습 (Part 2) | 4시간 |
| 4일차 | 정적 분석 도구 실습 | 4시간 |
| 5일차 | 종합 실습 및 평가 | 4시간 |

**총 교육 시간: 20시간 (5일)**

## ⚠️ 주의사항

1. **MISRA-C 가이드라인 문서는 유료**입니다. 공식 문서는 [MISRA 웹사이트](https://misra.org.uk/)에서 구매해야 합니다.
2. 본 교육 자료는 **교육 목적**으로 작성되었으며, 공식 문서를 대체하지 않습니다.
3. 실제 프로젝트 적용 시 **최신 버전의 공식 가이드라인**을 참조하세요.

## 🤝 기여 방법

1. 이 저장소를 Fork합니다
2. 새 브랜치를 생성합니다 (`git checkout -b feature/improvement`)
3. 변경사항을 커밋합니다 (`git commit -am 'Add new content'`)
4. 브랜치에 Push합니다 (`git push origin feature/improvement`)
5. Pull Request를 생성합니다

## 📄 라이선스

이 교육 자료는 MIT 라이선스로 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

---

**작성일**: 2025년 2월  
**버전**: 1.0  
**문의**: [Issues](https://github.com/your-username/misra-c-training/issues)
