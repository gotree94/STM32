# AUTOSAR 완벽 가이드 (Complete AUTOSAR Tutorial)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![AUTOSAR](https://img.shields.io/badge/AUTOSAR-Classic%20%26%20Adaptive-blue.svg)](https://www.autosar.org/)
[![Korean](https://img.shields.io/badge/Language-Korean-red.svg)]()

> **AUTomotive Open System ARchitecture** - 차량용 소프트웨어 개발을 위한 표준화된 아키텍처

## 📋 목차

- [개요](#-개요)
- [학습 대상](#-학습-대상)
- [커리큘럼 구성](#-커리큘럼-구성)
- [사전 요구사항](#-사전-요구사항)
- [학습 로드맵](#-학습-로드맵)
- [참고 자료](#-참고-자료)

---

## 📖 개요

이 강의는 **AUTOSAR(AUTomotive Open System ARchitecture)** 를 처음 접하는 분부터 실무에 적용하고자 하는 분까지, 체계적으로 AUTOSAR를 학습할 수 있도록 구성되어 있습니다.

### AUTOSAR란?

AUTOSAR는 2003년에 설립된 전 세계 자동차 제조사(OEM), 부품 공급업체(Tier-1), 반도체 회사들의 컨소시엄으로, 차량용 ECU(Electronic Control Unit) 소프트웨어의 **표준화된 아키텍처**를 정의합니다.

```
┌─────────────────────────────────────────────────────────────────┐
│                     AUTOSAR의 핵심 철학                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   "Cooperate on Standards, Compete on Implementation"           │
│        (표준에서 협력하고, 구현에서 경쟁한다)                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 왜 AUTOSAR인가?

| 문제점 (Before AUTOSAR) | 해결책 (With AUTOSAR) |
|------------------------|----------------------|
| ECU 소프트웨어의 하드웨어 종속성 | 하드웨어 독립적인 소프트웨어 개발 |
| 소프트웨어 재사용 불가 | 컴포넌트 기반의 재사용 가능한 아키텍처 |
| 공급업체 간 호환성 문제 | 표준화된 인터페이스로 상호운용성 확보 |
| 복잡한 통합 과정 | 체계적인 방법론으로 통합 간소화 |
| 개발 비용 증가 | 재사용과 표준화로 개발 비용 절감 |

---

## 🎯 학습 대상

- **임베디드 시스템 개발자**: 차량용 소프트웨어 개발에 관심있는 개발자
- **자동차 공학 전공자**: AUTOSAR 기반 ECU 개발을 배우고자 하는 학생
- **전장 분야 취업 준비생**: 자동차 전장 업계 취업을 준비하는 분
- **현업 엔지니어**: Classic에서 Adaptive로의 전환을 준비하는 엔지니어

---

## 📚 커리큘럼 구성

### Part 1: AUTOSAR 기초 (Foundation)

| Chapter | 제목 | 학습 시간 | 난이도 |
|---------|------|----------|--------|
| [01](./Chapter_01_Introduction/README.md) | AUTOSAR 소개 및 역사 | 2시간 | ⭐ |
| [02](./Chapter_02_Classic_Architecture/README.md) | Classic Platform 아키텍처 | 4시간 | ⭐⭐ |
| [03](./Chapter_03_BSW_Layers/README.md) | Basic Software (BSW) 계층 | 6시간 | ⭐⭐⭐ |

### Part 2: 소프트웨어 개발 (Software Development)

| Chapter | 제목 | 학습 시간 | 난이도 |
|---------|------|----------|--------|
| [04](./Chapter_04_Application_Software/README.md) | Application Software Layer | 4시간 | ⭐⭐ |
| [05](./Chapter_05_RTE/README.md) | Runtime Environment (RTE) | 4시간 | ⭐⭐⭐ |
| [06](./Chapter_06_Communication_Stack/README.md) | Communication Stack | 6시간 | ⭐⭐⭐ |

### Part 3: 고급 주제 (Advanced Topics)

| Chapter | 제목 | 학습 시간 | 난이도 |
|---------|------|----------|--------|
| [07](./Chapter_07_Memory_Stack/README.md) | Memory Stack | 3시간 | ⭐⭐⭐ |
| [08](./Chapter_08_Diagnostic_Stack/README.md) | Diagnostic Stack | 4시간 | ⭐⭐⭐ |
| [09](./Chapter_09_Methodology/README.md) | AUTOSAR 방법론 | 4시간 | ⭐⭐ |

### Part 4: 실무 적용 (Practical Application)

| Chapter | 제목 | 학습 시간 | 난이도 |
|---------|------|----------|--------|
| [10](./Chapter_10_Tools/README.md) | 개발 도구 및 환경 | 4시간 | ⭐⭐ |
| [11](./Chapter_11_Adaptive_AUTOSAR/README.md) | Adaptive AUTOSAR | 6시간 | ⭐⭐⭐⭐ |
| [12](./Chapter_12_Practical_Projects/README.md) | 실습 프로젝트 | 8시간 | ⭐⭐⭐⭐ |

**총 학습 시간: 약 55시간**

---

## 📋 사전 요구사항

### 필수 지식

```
✅ C 프로그래밍 언어 (중급 이상)
✅ 임베디드 시스템 기초 (마이크로컨트롤러, 메모리 구조)
✅ 기본적인 통신 프로토콜 이해 (CAN, SPI, I2C 등)
```

### 권장 지식

```
📌 RTOS (Real-Time Operating System) 개념
📌 차량 네트워크 통신 (CAN, LIN, FlexRay, Ethernet)
📌 소프트웨어 아키텍처 패턴
📌 UML 다이어그램 해석
```

---

## 🗺️ 학습 로드맵

```
                              ┌─────────────────────┐
                              │   학습 시작 지점     │
                              └──────────┬──────────┘
                                         │
                    ┌────────────────────┼────────────────────┐
                    │                    │                    │
                    ▼                    ▼                    ▼
          ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
          │  완전 초보자     │  │  임베디드 경험   │  │   AUTOSAR 경험   │
          │                 │  │                 │  │                 │
          │  Ch.01 → Ch.02  │  │  Ch.01 → Ch.03  │  │  Ch.06 → Ch.08  │
          │  → Ch.03 순서대로│  │  → Ch.04 → Ch.05│  │  → Ch.11 선택적 │
          └────────┬────────┘  └────────┬────────┘  └────────┬────────┘
                   │                    │                    │
                   └────────────────────┼────────────────────┘
                                        │
                                        ▼
                              ┌─────────────────────┐
                              │    Ch.09 방법론     │
                              │    Ch.10 도구      │
                              └──────────┬──────────┘
                                         │
                                         ▼
                              ┌─────────────────────┐
                              │   Ch.12 실습       │
                              │   프로젝트         │
                              └─────────────────────┘
```

---

## 📂 프로젝트 구조

```
AUTOSAR-Tutorial/
├── README.md                          # 메인 문서 (현재 파일)
├── Chapter_01_Introduction/           # AUTOSAR 소개
│   └── README.md
├── Chapter_02_Classic_Architecture/   # Classic Platform 아키텍처
│   └── README.md
├── Chapter_03_BSW_Layers/            # BSW 계층 상세
│   └── README.md
├── Chapter_04_Application_Software/   # 애플리케이션 소프트웨어
│   └── README.md
├── Chapter_05_RTE/                   # Runtime Environment
│   └── README.md
├── Chapter_06_Communication_Stack/    # 통신 스택
│   └── README.md
├── Chapter_07_Memory_Stack/          # 메모리 스택
│   └── README.md
├── Chapter_08_Diagnostic_Stack/       # 진단 스택
│   └── README.md
├── Chapter_09_Methodology/           # AUTOSAR 방법론
│   └── README.md
├── Chapter_10_Tools/                 # 개발 도구
│   └── README.md
├── Chapter_11_Adaptive_AUTOSAR/      # Adaptive Platform
│   └── README.md
├── Chapter_12_Practical_Projects/    # 실습 프로젝트
│   └── README.md
└── images/                           # 이미지 리소스
```

---

## 🔗 참고 자료

### 공식 문서

- [AUTOSAR 공식 웹사이트](https://www.autosar.org/)
- [AUTOSAR Classic Platform](https://www.autosar.org/standards/classic-platform)
- [AUTOSAR Adaptive Platform](https://www.autosar.org/standards/adaptive-platform)

### 추천 학습 리소스

- Vector E-Learning: AUTOSAR Module
- Udemy: AUTOSAR Architecture Course
- Coursera: Introduction to AUTOSAR

### 개발 도구

| 도구 | 용도 | 제조사 |
|------|------|--------|
| DaVinci Configurator | BSW 설정 | Vector |
| DaVinci Developer | SWC 개발 | Vector |
| EB tresos | AUTOSAR 설정 도구 | Elektrobit |
| MATLAB/Simulink | 모델 기반 개발 | MathWorks |
| Arctic Studio | AUTOSAR 설정 | ARCCORE |

---

## 📝 버전 정보

| 버전 | 날짜 | 내용 |
|------|------|------|
| 1.0.0 | 2024-01 | 초기 커리큘럼 구성 |

---

## 🤝 기여하기

이 강의 자료에 대한 피드백, 수정 제안, 추가 내용 기여를 환영합니다!

1. 이 저장소를 Fork합니다
2. 새로운 Branch를 생성합니다 (`git checkout -b feature/improvement`)
3. 변경사항을 Commit합니다 (`git commit -am 'Add new content'`)
4. Branch에 Push합니다 (`git push origin feature/improvement`)
5. Pull Request를 생성합니다

---

## 📜 라이선스

이 프로젝트는 MIT 라이선스를 따릅니다. 자세한 내용은 [LICENSE](./LICENSE) 파일을 참조하세요.

---

<div align="center">

**[⬆ 맨 위로](#autosar-완벽-가이드-complete-autosar-tutorial)**

Made with ❤️ for Automotive Software Engineers

</div>
