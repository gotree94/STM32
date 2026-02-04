# Chapter 11: Adaptive AUTOSAR

[← 이전 챕터](../Chapter_10_Tools/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_12_Practical_Projects/README.md)

---

## 📋 목차

1. [학습 목표](#1-학습-목표)
2. [Adaptive Platform 개요](#2-adaptive-platform-개요)
3. [Classic vs Adaptive 비교](#3-classic-vs-adaptive-비교)
4. [Adaptive Platform 아키텍처](#4-adaptive-platform-아키텍처)
5. [주요 기능 클러스터](#5-주요-기능-클러스터)
6. [요약 및 연습 문제](#6-요약-및-연습-문제)

---

## 1. 학습 목표

```
✅ Adaptive Platform이 필요한 이유 이해
✅ Classic과 Adaptive의 차이점 파악
✅ Adaptive Platform 아키텍처 이해
✅ 주요 기능 클러스터(FC) 학습
```

---

## 2. Adaptive Platform 개요

### 2.1 Adaptive Platform이란?

**Adaptive AUTOSAR**는 고성능 컴퓨팅과 동적 소프트웨어가 필요한 차세대 ECU를 위한 플랫폼입니다.

```
┌─────────────────────────────────────────────────────────────────┐
│              Adaptive Platform 등장 배경                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  🚗 자동차 트렌드 변화                                           │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  ADAS / 자율주행                                         │   │
│  │  - 카메라, 레이더, 라이다 센서 융합                      │   │
│  │  - 실시간 AI/ML 처리                                     │   │
│  │  - 높은 연산 능력 필요                                   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  V2X / 커넥티비티                                        │   │
│  │  - 클라우드 연결                                         │   │
│  │  - OTA (Over-The-Air) 업데이트                          │   │
│  │  - 동적 서비스 배포                                      │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  인포테인먼트                                            │   │
│  │  - 앱 생태계                                             │   │
│  │  - 멀티미디어 처리                                       │   │
│  │  - 사용자 인터페이스                                     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  → Classic Platform으로는 이러한 요구사항 충족 어려움           │
│  → Adaptive Platform 필요                                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 주요 특징

| 특징 | 설명 |
|------|------|
| **Service-Oriented** | 서비스 기반 통신 (SOME/IP) |
| **동적 구성** | 런타임 서비스 배포/업데이트 |
| **POSIX 기반** | Linux/QNX 등 표준 OS |
| **C++14/17** | 현대적 개발 언어 |
| **고성능 HW** | 멀티코어 MPU (ARM, x86) |

---

## 3. Classic vs Adaptive 비교

```
┌─────────────────────────────────────────────────────────────────┐
│              Classic vs Adaptive Platform 비교                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   특성        │   Classic           │   Adaptive               │
│  ─────────────┼─────────────────────┼──────────────────────── │
│   대상 HW     │   MCU               │   MPU (High-Performance) │
│   OS          │   OSEK/VDX          │   POSIX (Linux, QNX)     │
│   언어        │   C                 │   C++14/17               │
│   통신        │   Signal-based      │   Service-Oriented       │
│   구성        │   정적 (컴파일타임) │   동적 (런타임)          │
│   메모리      │   정적 할당         │   동적 할당 허용         │
│   업데이트    │   전체 플래싱       │   개별 서비스 업데이트   │
│                                                                 │
│   적용 분야   │                     │                          │
│  ─────────────┼─────────────────────┼──────────────────────── │
│   파워트레인  │        ✓            │                          │
│   섀시/제동   │        ✓            │                          │
│   바디 제어   │        ✓            │                          │
│   ADAS        │                     │        ✓                 │
│   자율주행    │                     │        ✓                 │
│   인포테인먼트│                     │        ✓                 │
│   V2X         │                     │        ✓                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 4. Adaptive Platform 아키텍처

### 4.1 계층 구조

```
┌─────────────────────────────────────────────────────────────────┐
│                Adaptive Platform 아키텍처                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                 Adaptive Applications                    │   │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐    │   │
│  │  │  App 1  │  │  App 2  │  │  App 3  │  │  App n  │    │   │
│  │  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘    │   │
│  └───────┼────────────┼────────────┼────────────┼──────────┘   │
│          │            │            │            │              │
│          │         ara::com (Service 통신)      │              │
│          ▼            ▼            ▼            ▼              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              ARA (AUTOSAR Runtime for Adaptive)          │   │
│  │                                                          │   │
│  │  ┌───────────┬───────────┬───────────┬───────────┐      │   │
│  │  │ Comunic-  │Execution  │  State    │  Diag-    │      │   │
│  │  │  ation    │Management │Management │  nostics  │      │   │
│  │  │ (ara::com)│ (ara::exec)│ (ara::sm) │(ara::diag)│      │   │
│  │  └───────────┴───────────┴───────────┴───────────┘      │   │
│  │  ┌───────────┬───────────┬───────────┬───────────┐      │   │
│  │  │  Persist- │   Time    │  Crypto   │  Network  │      │   │
│  │  │   ency    │   Sync    │           │Management │      │   │
│  │  │(ara::per) │(ara::tsync)│(ara::crypto)│ (ara::nm)│      │   │
│  │  └───────────┴───────────┴───────────┴───────────┘      │   │
│  │                                                          │   │
│  └───────────────────────────┬──────────────────────────────┘   │
│                              │                                  │
│  ┌───────────────────────────▼──────────────────────────────┐   │
│  │                    Operating System                       │   │
│  │              (POSIX: Linux, QNX, etc.)                   │   │
│  └───────────────────────────┬──────────────────────────────┘   │
│                              │                                  │
│  ┌───────────────────────────▼──────────────────────────────┐   │
│  │                      Hardware                             │   │
│  │              (High-Performance MPU)                       │   │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 ARA (AUTOSAR Runtime for Adaptive)

ARA는 Adaptive Application에 API를 제공하는 런타임 환경입니다.

| 네임스페이스 | 기능 |
|-------------|------|
| `ara::com` | 서비스 통신 (SOME/IP) |
| `ara::exec` | 애플리케이션 실행 관리 |
| `ara::log` | 로깅 |
| `ara::per` | 영속성 (Persistency) |
| `ara::diag` | 진단 (UDS over IP) |
| `ara::crypto` | 암호화 |

---

## 5. 주요 기능 클러스터

### 5.1 Communication Management (ara::com)

```cpp
// Service 제공 (Server)
class RadarService : public ara::com::skeleton::RadarSkeleton {
public:
    void UpdateDistance(float distance) {
        // Event 전송
        DistanceEvent.Send(distance);
    }
    
    // Method 구현
    ara::core::Future<float> GetDistance() override {
        ara::core::Promise<float> promise;
        promise.set_value(currentDistance);
        return promise.get_future();
    }
};

// Service 사용 (Client)
void UseRadarService() {
    auto proxy = ara::com::proxy::RadarProxy::FindService();
    
    // Event 구독
    proxy->DistanceEvent.Subscribe([](float distance) {
        ProcessDistance(distance);
    });
    
    // Method 호출
    auto future = proxy->GetDistance();
    float dist = future.get();
}
```

### 5.2 Execution Management (ara::exec)

- 애플리케이션 수명 주기 관리
- Machine State 관리
- Function Group 관리

### 5.3 SOME/IP 프로토콜

```
┌─────────────────────────────────────────────────────────────────┐
│                    SOME/IP 메시지 구조                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Header (16 bytes)                                       │   │
│  │  ┌───────────┬───────────┬───────────┬───────────┐      │   │
│  │  │ Service ID│ Method ID │  Length   │ Client ID │      │   │
│  │  │  (2 byte) │  (2 byte) │ (4 byte)  │  (2 byte) │      │   │
│  │  ├───────────┼───────────┼───────────┼───────────┤      │   │
│  │  │Session ID │Proto Ver  │Interface  │Msg Type   │      │   │
│  │  │  (2 byte) │  (1 byte) │Ver(1byte) │ (1 byte)  │      │   │
│  │  └───────────┴───────────┴───────────┴───────────┘      │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Payload (가변 길이)                                     │   │
│  │  - Serialized Data (Service Interface 정의에 따름)       │   │
│  └─────────────────────────────────────────────────────────┘   │   
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 6. 요약 및 연습 문제

### 핵심 정리

```
┌─────────────────────────────────────────────────────────────────┐
│                    Chapter 11 핵심 정리                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  📌 Adaptive Platform 필요성:                                    │
│     - ADAS/자율주행의 높은 연산 요구                           │
│     - 동적 서비스 배포, OTA 업데이트                           │
│     - 클라우드 연결, V2X 통신                                  │
│                                                                 │
│  📌 Classic vs Adaptive:                                        │
│     ├── HW: MCU vs MPU                                         │
│     ├── OS: OSEK vs POSIX                                      │
│     ├── 언어: C vs C++                                         │
│     └── 통신: Signal vs Service (SOME/IP)                      │
│                                                                 │
│  📌 ARA 주요 기능:                                               │
│     ├── ara::com: 서비스 통신                                  │
│     ├── ara::exec: 실행 관리                                   │
│     ├── ara::per: 영속성                                       │
│     └── ara::diag: 진단                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 연습 문제

**문제 1.** Adaptive Platform이 필요한 이유 3가지는?

<details>
<summary>정답</summary>

1. ADAS/자율주행의 높은 연산 요구 (센서 융합, AI/ML)
2. OTA 업데이트, 동적 서비스 배포 필요
3. V2X 통신, 클라우드 연결 요구
</details>

**문제 2.** Classic의 Signal-based 통신과 Adaptive의 Service-Oriented 통신의 차이는?

<details>
<summary>정답</summary>

- **Signal-based**: 미리 정의된 Signal을 주기적/이벤트로 전송. 정적 구성.
- **Service-Oriented**: 서비스 발견/구독 기반. Method 호출, Event 전송. 동적 구성 가능.
</details>

---

[← 이전 챕터](../Chapter_10_Tools/README.md) | [메인으로 돌아가기](../README.md) | [다음 챕터 →](../Chapter_12_Practical_Projects/README.md)
