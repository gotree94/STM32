# STM32F411 문서 학습 로드맵 및 파일별 요약

> 대상: STM32F411 계열(특히 STM32F411xC/xE)  
> 목적: 데이터시트와 레퍼런스 매뉴얼을 바탕으로 하드웨어 설계, 펌웨어 개발, 디버깅, 프로그래밍/부트로더, 신호 무결성 및 응용 기능까지 단계적으로 학습  
> 작성 기준: 제공된 폴더 트리의 파일명과 문서 제목을 기준으로 정리했습니다. 실제 세부 내용·적용 조건·레지스터 설명은 원문 PDF에서 확인해야 합니다.

---

## 1. 먼저 읽을 문서: 핵심 학습 순서

STM32F411을 처음 제대로 다루거나 보드 설계와 펌웨어를 함께 학습한다면 아래 순서를 권장합니다.

| 순서 | 파일 | 우선순위 | 읽는 목적 |
|---:|---|---|---|
| 1 | `411-Product Specification/DS10314-Datasheet - production data.pdf` | 필수 | 제품군, 메모리, 클록, 전기적 특성, <br>핀 배치 및 패키지 확인 |
| 2 | `411-Reference Manuals/RM0383-STM32F411xCE advanced Arm-based 32-bit MCUs.pdf` | 필수 | 주변장치 레지스터, 버스, <br>클록 트리, GPIO, 타이머, <br>ADC, 통신장치의 동작 이해 |
| 3 | `411-Programming Manuals/PM0214-STM32 Cortex-M4 MCUs and MPUs programming manual.pdf` | 필수 | Cortex-M4 명령어, 예외/인터럽트, <br>메모리 모델, FPU 및 코어 프로그래밍 이해 |
| 4 | `411-Errata Sheets/ES0287-STM32F411xCxE device errata.pdf` | 필수 | 실리콘 리비전별 제한사항과 우회 방법 확인 |
| 5 | `411-Application Notes/AN4488-Getting started with STM32F4xxxx MCU hardware development.pdf` | 매우 높음 | 전원, 리셋, 클록, <br>디커플링 등 MCU 하드웨어 설계의 출발점 |
| 6 | `411-Application Notes/AN4899-Guidelines for GPIO hardware settings and low-power consumption on STM32 MCUs.pdf` | 매우 높음 | GPIO 전기적 설정, <br>입력/출력 구성, 풀업·풀다운 및 저전력 설계 |
| 7 | `411-Application Notes/AN4013-Introduction to timers for STM32 MCUs.pdf` | 매우 높음 | 타이머의 기본 구조와 입력 캡처, <br>출력 비교, PWM 등의 개념 |
| 8 | `411-Application Notes/AN4776-How to use general-purpose timer peripheral on STM32 MCUs.pdf` | 매우 높음 | 범용 타이머의 실제<br>설정과 응용 방법 |
| 9 | `411-Application Notes/AN4031-Using the STM32F2, STM32F4 and STM32F7 Series DMA controller.pdf` | 매우 높음 | DMA 스트림·채널·요청 매핑과 <br>CPU 부하를 줄이는 데이터 전송 |
| 10 | `411-Application Notes/AN2834-How to optimize the ADC accuracy in the STM32 MCUs.pdf` | 매우 높음 | ADC 정확도, 샘플링 시간, 기준전압, <br>아날로그 회로 및 보정 고려 |
| 11 | `411-Application Notes/AN4646-Peripheral interconnections on STM32F401 and STM32F411 lines.pdf` | 높음 | 주변장치 간 내부 연결 및 <br>타이머·ADC 등 연동 설계 |
| 12 | `411-Application Notes/AN4739-STM32Cube firmware examples for STM32F4 Series.pdf` | 높음 | STM32Cube 펌웨어 예제의 구성과 <br>주변장치별 실습 방향 파악 |
| 13 | `411-Application Notes/AN4989-Introduction to debug toolbox for STM32 MCUs.pdf` | 높음 | 디버깅 도구와 문제 분석 방법 이해 |
| 14 | `411-Application Notes/AN2606-Introduction to system memory boot mode on STM32 MCUs.pdf` | 높음 | System Memory 부트 모드와 <br>내장 부트로더 진입 조건 이해 |
| 15 | `411-Application Notes/AN3155-How to use the USART protocol in the bootloader.pdf` | 높음 | USART를 이용한 ROM <br>부트로더 다운로드 절차 |
| 16 | `411-Application Notes/AN3154-How to use CAN protocol in bootloader on STM32 MCUs.pdf` | 선택 | CAN 부트로더 프로토콜. F411의 해당 인터페이스/부트로더 <br>지원 여부를 AN2606에서 먼저 확인 |
| 17 | `411-Application Notes/AN3156-How to use the USB DFU protocol in the bootloader on STM32 MCUs.pdf` | 선택 | USB DFU 부트로더 개념. F411의 USB 및<br> ROM 부트로더 지원 조건 확인 필요 |
| 18 | `411-Application Notes/AN4221-How to use I2C protocol in bootloader on STM32 MCUs.pdf` | 선택 | I2C 부트로더 프로토콜. <br>F411의 실제 지원 여부 확인 필요 |
| 19 | `411-Application Notes/AN4286-How to use SPI protocol in bootloader on STM32 MCUs.pdf` | 선택 | SPI 부트로더 프로토콜. <br>F411의 실제 지원 여부 확인 필요 |

### 핵심 원칙

- **Datasheet는 무엇이 가능한지**, **Reference Manual은 어떻게 동작하는지**, **Programming Manual은 Cortex-M4 코어가 어떻게 동작하는지**를 설명합니다.
- Errata는 기능을 구현하기 전과 검증 중에 반드시 확인합니다.
- Application Note는 모든 문서를 처음부터 읽기보다 현재 구현할 주변장치와 기능에 맞춰 읽는 편이 효율적입니다.
- 부트로더 관련 AN은 STM32 제품군 전체를 대상으로 작성된 경우가 있으므로, 문서에 프로토콜이 나온다는 사실만으로 F411에서 지원된다고 판단하면 안 됩니다. `AN2606`의 STM32F411 항목과 해당 실리콘/부트로더 버전의 지원 표를 확인하세요.

---

## 2. 기능별 실습에 따른 추천 읽기 순서

| 실습 주제 | 먼저 읽을 문서 | 이어서 읽을 문서 |
|---|---|---|
| 보드 전원·클록·리셋 설계 | DS10314, RM0383, AN4488 | AN2867, AN4899, AN5612, AN5036 |
| GPIO·외부 인터럽트 | RM0383, PM0214 | AN4899, AN4989 |
| 타이머·PWM·서보모터 | RM0383, AN4013 | AN4776, AN4277, AN4646 |
| ADC·센서 입력 | RM0383 | AN2834, AN5537, AN4646 |
| DMA 기반 ADC/UART/SPI | RM0383, AN4031 | AN4739, AN4655 |
| USART·RS-485 | RM0383 | AN3070, AN4908, AN4655 |
| I2C·SPI 주변장치 | RM0383 | AN4739, AN4655, 부트로더 목적이면 AN4221/AN4286 |
| USB 장치·PCB | DS10314, RM0383 | AN4879, AN5225, AN3156(부트로더 목적) |
| 부트 모드·펌웨어 다운로드 | AN2606, DS10314 | AN3155, AN3154 등 해당 인터페이스 문서 |
| 저전력·RTC | RM0383 | AN4899, AN4759 |
| 보안·메모리 보호 | PM0214, RM0383 | AN5156, AN4838, TN1489 |
| PCB 신호 무결성·EMC | AN4488 | AN1709, AN4803, AN4850, AN5612 |
| 생산·실장·패키지 | DS10314 | AN2639, TN1163, TN1204~TN1208, TN1433 |

---

## 3. 파일별 요약

아래 설명은 파일명을 기준으로 한 빠른 안내입니다. 문서의 적용 제품, 개정판, 세부 조건은 PDF 본문을 기준으로 확인하세요.

## 3.1 제품 사양·핵심 아키텍처

| 파일 | 중요도 | 간단한 요약 / 활용 |
|---|---|---|
| `DS10314-Datasheet - production data.pdf` | 필수 | STM32F411의 제품 사양서. 주문 코드, 메모리 용량, CPU 성능, 동작 전압·온도, 클록, 주변장치 수, 핀 기능, 패키지 및 전기적 특성을 확인하는 기준 문서. 회로도와 부품 선정 전에 우선 확인합니다. |
| `RM0383-STM32F411xCE advanced Arm-based 32-bit MCUs.pdf` | 필수 | STM32F411의 Reference Manual. RCC, GPIO, DMA, ADC, 타이머, USART, SPI, I2C, USB 등 주변장치의 레지스터와 동작 절차를 설명합니다. 레지스터 직접 제어 펌웨어의 핵심 문서입니다. |
| `PM0214-STM32 Cortex-M4 MCUs and MPUs programming manual.pdf` | 필수 | Cortex-M4 코어 프로그래밍 안내. 명령어 집합, 레지스터, 예외와 인터럽트, 메모리 모델, FPU 및 코어 기능을 이해할 때 사용합니다. STM32 주변장치 설명서인 RM0383과 함께 읽습니다. |
| `ES0287-STM32F411xCxE device errata.pdf` | 필수 | STM32F411xC/xE의 실리콘별 알려진 오류·제한사항과 영향, 조건 및 대응책을 정리한 Errata. 설계 검토와 양산 전 검증에서 반드시 확인합니다. |
| `UM3461.pdf` | 확인 필요 | 파일명만으로 문서 주제와 STM32F411 직접 관련성을 특정하기 어렵습니다. PDF 표지에서 문서 제목, 대상 제품, 개정판을 먼저 확인한 뒤 관련성이 있는 경우 학습 목록에 편입합니다. |

## 3.2 Application Notes

| 파일 | 중요도 | 간단한 요약 / 활용 |
|---|---|---|
| `AN1709-EMC design guide for STM8, STM32 and legacy MCUs.pdf` | 중 | MCU 보드의 전자파 적합성(EMC) 설계 지침. 클록·전원·PCB 배치·배선·접지·필터링 등 방사 및 내성 개선 관점에서 참고합니다. |
| `AN2606-Introduction to system memory boot mode on STM32 MCUs.pdf` | 높음 | STM32의 System Memory 부트 모드와 내장 부트로더 개요. 부트 핀, 진입 조건, 지원 인터페이스와 제품별 차이를 확인합니다. F411 ROM 부트로더를 사용할 때 가장 먼저 볼 문서입니다. |
| `AN2639-Soldering recommendations and package information for lead-free ECOPACK2 MCUs and MPUs.pdf` | 낮음~중 | 무연 패키지의 납땜 및 실장 권고사항. PCB 조립 공정, 리플로우, 패키지 취급과 생산 품질 검토에 참고합니다. |
| `AN2834-How to optimize the ADC accuracy in the STM32 MCUs.pdf` | 높음 | STM32 ADC 측정 정확도 개선 가이드. 입력 임피던스, 샘플링 시간, 아날로그 전원·기준전압, 노이즈, 레이아웃과 보정 요소를 검토할 때 유용합니다. |
| `AN2867-Guidelines for oscillator design on STM8AFALS and STM32 MCUsMPUs.pdf` | 높음 | 외부 크리스털/오실레이터 회로 설계 지침. 부하 커패시터, 발진 조건, PCB 배치 및 기동 안정성을 검토합니다. 실제 F411 클록 구성과 데이터시트 조건을 함께 확인합니다. |
| `AN2945-STM8S and STM32 MCUs a consistent 832-bit product line for painless migration.pdf` | 낮음 | STM8S와 STM32 제품 간 제품군·이행 관점의 문서. 제목의 “832-bit” 표기는 원본 파일명에 있는 표현이며, 실제 문서의 제목과 범위를 PDF에서 확인하는 것이 좋습니다. F411 주변장치 학습의 우선 문서는 아닙니다. |
| `AN3070-Managing the Driver Enable signal for RS-485 and IO-Link communications with the STM32's USART.pdf` | 중~높음 | USART의 DE(Driver Enable) 신호를 이용한 RS-485/IO-Link 송수신 제어 방법. 반이중 RS-485 트랜시버의 송수신 방향 전환과 타이밍 설계에 참고합니다. |
| `AN3126-How to use DAC for audio and waveform generation on STM32 MCUs and MPUs.pdf` | 낮음 | DAC를 이용한 오디오 및 파형 생성 방법. STM32F411에 구현하려는 DAC 기능이 실제로 제공되는지는 DS10314/RM0383에서 먼저 확인해야 합니다. 지원되지 않는 기능을 F411 실습에 그대로 적용하면 안 됩니다. |
| `AN3154-How to use CAN protocol in bootloader on STM32 MCUs.pdf` | 선택 | CAN을 이용하는 STM32 부트로더 프로토콜 설명. F411의 내장 부트로더에서 CAN을 지원한다고 가정하지 말고 AN2606의 제품별 지원표를 확인합니다. |
| `AN3155-How to use the USART protocol in the bootloader.pdf` | 높음 | USART 기반 ROM 부트로더 통신 프로토콜. PC 툴과 STM32 간 명령·주소·데이터 전송, 다운로드 절차를 이해할 때 사용합니다. |
| `AN3156-How to use the USB DFU protocol in the bootloader on STM32 MCUs.pdf` | 선택 | USB DFU 기반 부트로더 프로토콜 설명. F411의 USB 하드웨어 및 내장 부트로더에서 해당 경로를 지원하는지 제품별 문서로 검증해야 합니다. |
| `AN3364-Migration and compatibility guidelines for STM32 microcontroller applications.pdf` | 중 | STM32 MCU 간 애플리케이션 이식 및 호환성 검토 지침. 다른 STM32 계열에서 F411로 이동하거나 반대로 포팅할 때 핀, 주변장치, 클록, 메모리 차이를 점검합니다. |
| `AN3997-Audio playback and recording using the STM32F4DISCOVERY.pdf` | 중 | STM32F4DISCOVERY 보드를 이용한 오디오 재생·녹음 예제. 보드의 오디오 코덱, I2S/SAI 등 주변 회로와 소프트웨어 흐름을 이해하는 교육 자료로 활용합니다. 보드 구성과 F411 단독 칩의 차이를 구분해야 합니다. |
| `AN3998-PDM audio software decoding on STM32 microcontrollers.pdf` | 중 | PDM 디지털 마이크 데이터를 PCM 오디오로 변환하는 소프트웨어 디코딩 기법. 디지털 마이크 입력 및 오디오 처리 프로젝트에 참고합니다. |
| `AN4013-Introduction to timers for STM32 MCUs.pdf` | 높음 | STM32 타이머의 종류와 기본 동작을 소개합니다. 시간 측정, 주기 생성, 입력 캡처, 출력 비교, PWM 등 타이머 실습의 입문 문서입니다. |
| `AN4031-Using the STM32F2, STM32F4 and STM32F7 Series DMA controller.pdf` | 높음 | F2/F4/F7 DMA 컨트롤러 사용법. 스트림·채널 선택, 메모리/주변장치 전송, 순환 모드, 인터럽트 및 데이터 처리 흐름을 학습합니다. ADC·UART·SPI 실습과 직접 연결됩니다. |
| `AN4221-How to use I2C protocol in bootloader on STM32 MCUs.pdf` | 선택 | I2C 기반 STM32 부트로더 프로토콜 설명. F411 ROM 부트로더에서 지원되는지는 AN2606의 제품별 표를 먼저 확인합니다. |
| `AN4230-Introduction to random number generation validation using the NIST statistical test suite for STM32 MCUs and MPUs.pdf` | 낮음~중 | 난수 생성 결과를 NIST 통계 테스트로 검증하는 방법. 난수 품질·보안 기능 평가가 필요한 프로젝트에서 참고합니다. |
| `AN4277-How to use PWM shutdown for motor control and digital power conversion on STM32 MCUs.pdf` | 중~높음 | 모터 제어 및 디지털 전력 변환에서 PWM 출력을 안전하게 차단하는 방법. 타이머의 Break 기능 등 보호 동작과 고장 대응을 공부할 때 유용합니다. |
| `AN4286-How to use SPI protocol in bootloader on STM32 MCUs.pdf` | 선택 | SPI 기반 STM32 부트로더 프로토콜 설명. F411의 내장 부트로더 지원 여부를 AN2606에서 확인한 뒤 참고합니다. |
| `AN4488-Getting started with STM32F4xxxx MCU hardware development.pdf` | 매우 높음 | STM32F4 하드웨어 개발 입문. 전원, 디커플링, 리셋, 클록, 부트 설정 및 기본 회로를 검토하는 보드 설계 출발점입니다. |
| `AN4566-How to extend the DAC performance on STM32 MCUs.pdf` | 낮음 | STM32 DAC 출력 성능을 개선하는 방법. F411에서 DAC 주변장치가 제공되는지 먼저 확인하고, 다른 STM32 계열용 참고자료로 사용할지 판단합니다. |
| `AN4616-Migrating from STM32F401 and STM32F411 lines to STM32L4 Series and STM32L4+ Series microcontrollers.pdf` | 중 | F401/F411에서 저전력 STM32L4/L4+로 이행할 때의 차이와 호환성 검토 자료. F411 자체 기능 학습보다는 제품 교체·포팅에 적합합니다. |
| `AN4646-Peripheral interconnections on STM32F401 and STM32F411 lines.pdf` | 높음 | F401/F411 내부 주변장치 연결 기능을 설명합니다. 타이머 이벤트로 ADC를 트리거하거나 주변장치 간 하드웨어 연동을 구성하는 고급 실습에 유용합니다. |
| `AN4655-Virtually increasing the number of serial communication peripherals in STM32 applications.pdf` | 중 | 소프트웨어 방식 등을 활용해 직렬 통신 포트 수를 확장하는 접근을 다룹니다. UART 등 하드웨어 포트가 부족한 설계에서 대안과 제약을 검토합니다. |
| `AN4739-STM32Cube firmware examples for STM32F4 Series.pdf` | 높음 | STM32F4용 STM32Cube 펌웨어 예제 안내. 예제 프로젝트의 구성과 주변장치별 샘플을 찾고 실습을 확장하는 데 활용합니다. |
| `AN4750-Handling of soft errors in STM32 applications.pdf` | 중 | 소프트 오류 및 일시적 데이터 오류의 영향과 대응 방법. 메모리·데이터 무결성, 오류 감지 및 신뢰성 설계 관점에서 참고합니다. |
| `AN4759-Introduction to using the hardware real-time clock (RTC) and the tamper management unit (TAMP) for STM32 MCUs.pdf` | 중 | RTC와 탬퍼 감지 기능의 개요 및 사용법. 시간 유지, 백업 도메인, 외부 이벤트 감지 응용에 참고합니다. 실제 F411 지원 기능은 RM0383에서 확인합니다. |
| `AN4776-How to use general-purpose timer peripheral on STM32 MCUs.pdf` | 높음 | 범용 타이머의 상세 사용 가이드. 타이머 설정, 입력 캡처, 출력 비교, PWM 및 인터럽트 기반 시간 제어 실습에 활용합니다. |
| `AN4803-High-speed SI simulations using IBIS and board-level simulations using HyperLynx SI on STM32 MCUs and MPUs.pdf` | 중 | IBIS 모델과 HyperLynx를 이용한 고속 신호 무결성(SI) 시뮬레이션 소개. 고속 인터페이스의 신호 품질 및 보드 레벨 검토에 참고합니다. |
| `AN4838-Introduction to memory protection unit management on STM32 MCUs.pdf` | 중 | MPU(Memory Protection Unit)의 메모리 영역 보호와 접근 권한 설정 개요. RTOS 및 메모리 오류 격리 설계에 활용합니다. F411의 Cortex-M4 MPU 지원 및 구성 조건을 확인합니다. |
| `AN4850-STM32 MCUs spread-spectrum clock generation principles, properties and implementation.pdf` | 중 | 스프레드 스펙트럼 클록 생성 원리와 EMI 저감 효과·제약을 설명합니다. 클록 변조를 통한 전자파 방출 개선을 검토할 때 참고합니다. |
| `AN4879-Introduction to USB hardware and PCB guidelines using STM32 MCUs.pdf` | 중~높음 | STM32 USB 하드웨어 및 PCB 설계 지침. USB 차동 배선, 커넥터, 보호소자, 전원 및 레이아웃 검토에 활용합니다. |
| `AN4899-Guidelines for GPIO hardware settings and low-power consumption on STM32 MCUs.pdf` | 높음 | GPIO 모드와 전기적 설정, 미사용 핀 처리, 풀업·풀다운 및 저전력 소비 최적화 지침입니다. 보드 설계와 펌웨어 초기화 모두에 중요합니다. |
| `AN4904-Migration of microcontroller applications from STM32F1 Series to STM32F4 Access lines.pdf` | 중 | STM32F1에서 STM32F4 Access Line으로 애플리케이션을 옮길 때의 차이와 주의사항. F103에서 F411로 이전하는 교육·개발에 참고합니다. |
| `AN4908-Getting started with USART automatic baud rate detection for STM32 MCUs.pdf` | 중 | USART 자동 보레이트 검출 기능의 동작과 설정 방법. 통신 속도를 사전에 고정하기 어려운 시스템에서 참고합니다. |
| `AN4989-Introduction to debug toolbox for STM32 MCUs.pdf` | 높음 | STM32 디버깅 도구와 문제 분석 기법 소개. 디버거, 추적 및 진단 기능을 활용한 펌웨어 디버깅 학습에 적합합니다. |
| `AN4995-Using an electromyogram technique to detect muscle activity.pdf` | 낮음~중 | 근전도(EMG) 신호로 근육 활동을 검출하는 응용 사례. 아날로그 프런트엔드, 신호 취득·처리 교육의 참고자료로 활용할 수 있습니다. |
| `AN5027-Interfacing PDM digital microphones using STM32 MCUs and MPUs.pdf` | 중 | PDM 디지털 마이크를 STM32에 연결하는 방법. 마이크 클록·데이터 인터페이스 및 PDM 데이터 취득 실습에 참고합니다. |
| `AN5036-Guidelines for thermal management on STM32 MCUs and MPUs.pdf` | 중 | MCU/MPU의 열 관리 지침. 주변 온도, 전력 소모, 패키지 열 특성 및 PCB 열 설계를 검토합니다. |
| `AN5073-Receiving SPDIF audio stream with the STM32F4F7H7 Series.pdf` | 낮음~중 | S/PDIF 디지털 오디오 스트림 수신 방법. 오디오 인터페이스 실습에서 관련 주변장치와 제품 지원 여부를 확인한 뒤 참고합니다. |
| `AN5156-Introduction to security for STM32 MCUs.pdf` | 중~높음 | STM32 보안 기능과 보안 설계의 기본 개념. 디버그 접근, 펌웨어 보호, 보안 부팅·키 관리 등은 제품별 지원 범위를 확인하며 학습합니다. |
| `AN5225-Introduction to USB Type-C Power Delivery for STM32 MCUs and MPUs.pdf` | 낮음~중 | USB Type-C 및 Power Delivery의 기본 구조와 STM32 활용 방법. F411 보드에서 USB-C 전원 협상을 구현하려면 별도 PD 컨트롤러 및 지원 구성을 확인해야 합니다. |
| `AN5537-How to use ADC Oversampling techniques to improve signal-to-noise ratio on STM32 MCUs.pdf` | 중~높음 | ADC 오버샘플링으로 노이즈와 유효 분해능을 개선하는 기법. F411에서 하드웨어 지원 여부와 소프트웨어 누산 방식의 적용 가능성을 구분합니다. |
| `AN5612-ESD protection of STM32 MCUs and MPUs.pdf` | 중~높음 | 정전기 방전(ESD)으로부터 MCU와 외부 인터페이스를 보호하는 설계 지침. 커넥터, 보호소자, 접지 및 PCB 배치 검토에 활용합니다. |
| `AN5647-The Goertzel algorithm to compute individual terms of the discrete Fourier transform (DFT) in STM32 products.pdf` | 중 | Goertzel 알고리즘으로 특정 주파수 성분을 계산하는 방법. DTMF, 톤 검출, 센서 진동 주파수 분석 등 제한된 주파수 분석 실습에 적합합니다. |
| `AN5852-Migrating from STM32F401, STM32F410, and STM32F411 to STM32H503 MCUs.pdf` | 중 | F401/F410/F411에서 STM32H503으로 이행할 때의 호환성과 차이 검토 자료. 제품 세대 교체 및 포팅 프로젝트에 활용합니다. |

## 3.3 BSDL·IBIS·SVD 및 기타 설계 데이터

| 파일 / 폴더 | 중요도 | 간단한 요약 / 활용 |
|---|---|---|
| `411-BSDL/19098/STM32F411_D_E_LQFP100.bsd` | 낮음~중 | LQFP100 패키지용 Boundary-Scan Description Language 파일. JTAG 경계 스캔 테스트 장비에서 핀과 셀 동작을 기술할 때 사용합니다. |
| `411-BSDL/19098/STM32F411_D_E_LQFP64.bsd` | 낮음~중 | LQFP64 패키지용 BSDL 파일. 해당 패키지의 제조 테스트 및 JTAG 경계 스캔에 사용합니다. |
| `411-BSDL/19098/STM32F411_D_E_UFBGA100.bsd` | 낮음~중 | UFBGA100 패키지용 BSDL 파일. 패키지별 경계 스캔 테스트 구성에 사용합니다. |
| `411-BSDL/19098/STM32F411_D_E_UQFN48.bsd` | 낮음~중 | UQFN48 패키지용 BSDL 파일. 제조 테스트에서 핀 연결 및 경계 스캔 셀 정의를 참조합니다. |
| `411-BSDL/19098/STM32F411_D_E_WLCSP49.bsd` | 낮음~중 | WLCSP49 패키지용 BSDL 파일. 웨이퍼 레벨 패키지의 경계 스캔 테스트에 사용합니다. |
| `411-BSDL/19098/CortexMx.bsd` | 낮음 | Cortex-M 계열 관련 BSDL 보조 파일. 패키지별 STM32 BSDL과 함께 테스트 환경에서 참조할 수 있습니다. |
| `411-BSDL/19098/readme.txt` | 중 | BSDL 패키지의 구성, 적용 범위 및 사용상 주의사항을 확인하는 안내 파일. BSDL을 사용하기 전에 읽습니다. |
| `411-BSDL/19098.zip` | 낮음 | 위 BSDL 파일이 포함된 압축 패키지. 압축을 풀어 패키지별 파일을 사용합니다. |
| `411-IBIS Models/stm32f411_lqfp100.ibs` | 중 | LQFP100 패키지의 IBIS I/O 버퍼 모델. PCB 신호 무결성 시뮬레이션에서 출력 드라이버·입력 버퍼 특성을 모델링합니다. |
| `411-IBIS Models/stm32f411_lqfp64.ibs` | 중 | LQFP64 패키지용 IBIS 모델. 해당 패키지의 보드 레벨 신호 시뮬레이션에 사용합니다. |
| `411-IBIS Models/stm32f411_ufbga100.ibs` | 중 | UFBGA100 패키지용 IBIS 모델. 고속 신호의 패키지별 SI 검토에 사용합니다. |
| `411-IBIS Models/stm32f411_uqfn48.ibs` | 중 | UQFN48 패키지용 IBIS 모델. 해당 패키지의 I/O 신호 시뮬레이션에 사용합니다. |
| `411-IBIS Models/stm32f411_wlcsp49.ibs` | 중 | WLCSP49 패키지용 IBIS 모델. 패키지에 따른 신호 특성을 반영한 시뮬레이션에 사용합니다. |
| `411-IBIS Models/19124.zip` | 낮음 | STM32F411 IBIS 모델 압축 패키지. 필요한 패키지 모델을 추출해 SI 도구에 등록합니다. |
| `411-System View Description/STM32F4_svd/STM32F4_svd_V2.0/STM32F411.svd` | 중~높음 | STM32F411의 CMSIS-SVD 파일. 주변장치 레지스터 주소·필드 정보를 디버거에 제공해 레지스터 뷰와 디버깅 자동화에 활용합니다. |
| `411-System View Description/STM32F4_svd/Release_Notes.htm` | 낮음~중 | SVD 패키지의 버전 및 변경 사항 안내. SVD 파일의 정확성과 개정 이력을 확인합니다. |
| `411-System View Description/20390.zip` | 낮음 | STM32F4 SVD 파일 패키지 압축본. 필요한 SVD를 추출합니다. |
| `411-Device Option Lists/opl_stm32f411_512k.op` | 낮음~중 | STM32F411 512KB 옵션 목록 데이터로 보입니다. ST 도구에서 디바이스 옵션/선택 정보를 제공하는 용도일 수 있으므로 함께 제공된 설명과 도구 호환성을 확인합니다. |
| `411-Device Option Lists/13928.zip` | 낮음 | Device Option List 압축 패키지. 압축 내 README 또는 메타데이터에서 사용 도구와 적용 범위를 확인합니다. |

## 3.4 Design Notes & Tips

| 파일 | 중요도 | 간단한 요약 / 활용 |
|---|---|---|
| `DT0088-FIR filter design by sampling, windowing and modulating the sinc() function.pdf` | 중 | 샘플링·윈도잉·sinc 함수 변조를 이용한 FIR 필터 설계 노트. ADC 센서 데이터의 디지털 필터 설계와 주파수 응답 이해에 활용합니다. |
| `DT0089-The Goertzel algorithm to compute individual terms of the discrete Fourier transform (DFT).pdf` | 중 | Goertzel 알고리즘으로 DFT의 특정 주파수 성분을 계산하는 설계 노트. AN5647과 함께 읽으면 이론과 구현 관점을 연결할 수 있습니다. |
| `DT0091-Lattice wave digital filter design and automatic C code generation.pdf` | 낮음~중 | Lattice Wave Digital Filter 설계 및 C 코드 자동 생성 방법. 고급 디지털 필터 알고리즘과 임베디드 코드 생성에 관심이 있을 때 읽습니다. |
| `DT0092-Lattice wave digital filter test and performance verification.pdf` | 낮음~중 | Lattice Wave Digital Filter의 시험 및 성능 검증 방법. DT0091에서 설계한 필터의 검증 절차를 이해할 때 이어서 읽습니다. |

## 3.5 Technical & Article

| 파일 | 중요도 | 간단한 요약 / 활용 |
|---|---|---|
| `TN0516-Overview of the STM32F0xF100xxF103xx and STM32F2xxF30xF4xx MCUs PMSM singledual FOC SDK V4.0.pdf` | 중 | STM32 제품군용 PMSM 모터 제어 및 FOC SDK 개요. F411을 이용한 모터 제어 교육에서 FOC 소프트웨어 구조와 지원 범위를 조사할 때 참고합니다. |
| `TN1163-Description of WLCSP for microcontrollers and recommendations for its use.pdf` | 낮음 | WLCSP 패키지 구조와 사용 권고사항. 소형 패키지의 PCB 풋프린트, 실장 및 제조 고려사항을 확인합니다. |
| `TN1204-Tape and reel shipping media for STM32 devices in BGA packages.pdf` | 낮음 | BGA 패키지 STM32의 테이프·릴 포장 및 운송 매체 설명. 부품 조달과 SMT 생산 공정에 참고합니다. |
| `TN1205-Tape and reel shipping media for STM8 and STM32 microcontrollers in FPN packages.pdf` | 낮음 | FPN 패키지 STM8/STM32의 테이프·릴 포장 정보. 생산·부품 취급 관련 자료입니다. |
| `TN1206-Tape and reel shipping media for STM8 and STM32 microcontrollers in QFP packages.pdf` | 낮음 | QFP 패키지의 테이프·릴 포장 정보. 부품 실장 공정 및 조달에 참고합니다. |
| `TN1207-Tape and reel shipping media for STM8 and STM32 microcontrollers in SO packages.pdf` | 낮음 | SO 패키지의 테이프·릴 포장 정보. 제조·조달 참고자료입니다. |
| `TN1208-Tape and reel shipping media for STM8 and STM32 microcontrollers in TSSOP and SSOP packages.pdf` | 낮음 | TSSOP/SSOP 패키지의 테이프·릴 포장 정보. 제조·조달 참고자료입니다. |
| `TN1433-Reference device marking schematics for STM32 microcontrollers and microprocessors.pdf` | 낮음~중 | STM32 부품의 마킹 표기 체계 안내. 칩 상면의 제품 식별 정보와 마킹을 확인할 때 참고합니다. |
| `TN1489-Security bulletin TN1489-ST-PSIRT Physical attacks on STM32 and STM32Cube firmware.pdf` | 중 | STM32 및 STM32Cube 펌웨어에 대한 물리적 공격 관련 보안 공지. 보안 위협 모델과 대응 권고를 확인합니다. 최신 보안 상태는 ST의 공지 개정 여부도 확인합니다. |

## 3.6 Presentation 및 Product Certification

| 파일 | 중요도 | 간단한 요약 / 활용 |
|---|---|---|
| `13329-STM32 development boards Helping you accelerate your design.pdf` | 낮음~중 | ST 개발 보드 종류와 개발 가속 방안을 소개하는 프레젠테이션. Nucleo/Discovery 등 교육용 보드 선택에 참고합니다. |
| `13332-Functional safety packages for STM32 MCUs and MPUs, and STM8 MCUs.pdf` | 중 | STM32/STM8 기능 안전 관련 패키지와 지원 자료 소개. 안전 관련 개발 프로세스·문서 패키지를 조사할 때 활용합니다. |
| `20056-Releasing your creativity Discover the STM32 family of microcontrollers & microprocessors.pdf` | 낮음 | STM32 MCU/MPU 제품군 전반을 소개하는 자료. F411의 제품군 내 위치를 이해하는 개요 자료입니다. |
| `13327-not open.pdf` | 확인 필요 | 파일명만으로 제목과 내용을 확인할 수 없습니다. PDF가 열리는지 확인하고 표지·문서 번호로 용도를 식별합니다. |
| `15187-not open.pdf` | 확인 필요 | 파일명만으로 제목과 내용을 확인할 수 없습니다. PDF가 열리는지 확인하고 표지·문서 번호로 용도를 식별합니다. |
| `411-Product Certification/13398.pdf` | 확인 필요 | 인증 관련 문서로 보이나 파일명만으로 인증 종류·적용 제품을 특정할 수 없습니다. PDF 표지와 인증 범위를 확인합니다. |

---

## 4. 학습 우선순위별 읽기 계획

### 1단계 — 칩과 보드의 구조 파악
1. DS10314 데이터시트
2. RM0383 레퍼런스 매뉴얼의 전체 구성 및 메모리 맵
3. PM0214 Cortex-M4 프로그래밍 매뉴얼의 코어·인터럽트 부분
4. ES0287 Errata

**학습 결과:** F411의 핀, 메모리, 클록, 버스, 주변장치와 주의해야 할 실리콘 제한사항을 설명할 수 있어야 합니다.

### 2단계 — 최소 동작 회로와 GPIO
1. AN4488 하드웨어 개발
2. AN2867 오실레이터 설계
3. AN4899 GPIO 및 저전력
4. AN1709 EMC 설계
5. AN5612 ESD 보호

**실습:** 전원·디커플링·NRST·BOOT 설정·외부 클록·SWD 커넥터를 포함한 최소 시스템 회로를 설계하고 GPIO LED/스위치를 구동합니다.

### 3단계 — 타이머·인터럽트·PWM
1. RM0383의 TIM 관련 장
2. AN4013 타이머 개요
3. AN4776 범용 타이머
4. AN4277 PWM shutdown
5. AN4646 주변장치 내부 연결

**실습:** 주기 인터럽트, 입력 캡처, PWM LED 밝기 제어, 서보모터 제어, 보호 입력에 의한 PWM 차단을 구현합니다.

### 4단계 — ADC·DMA·센서 데이터
1. RM0383의 ADC/DMA 장
2. AN2834 ADC 정확도
3. AN4031 DMA
4. AN5537 오버샘플링
5. AN4739 Cube 예제

**실습:** 가변저항/아날로그 센서 입력 → ADC 변환 → DMA 순환 버퍼 → 필터링 → UART 출력의 데이터 경로를 구성합니다.

### 5단계 — 통신 및 다운로드
1. RM0383의 USART/SPI/I2C/USB 장
2. AN3070 RS-485 DE
3. AN4908 자동 보레이트
4. AN2606 System Memory Boot
5. AN3155 USART Bootloader
6. 필요 시 AN3154/AN3156/AN4221/AN4286을 추가하되 F411 지원 여부 확인

**실습:** UART 로그, I2C 센서, SPI 디스플레이, RS-485 통신, ROM 부트로더 다운로드를 단계적으로 구현합니다.

### 6단계 — 디버깅·보안·신뢰성
1. AN4989 디버그 도구
2. PM0214 예외·인터럽트
3. AN4838 MPU
4. AN5156 보안
5. AN4750 소프트 오류
6. TN1489 보안 공지

**실습:** SWD 디버깅, HardFault 분석, 메모리 접근 오류 재현, 펌웨어 보호 옵션 검토를 수행합니다.

### 7단계 — 고급 응용 및 보드 검증
- 오디오: AN3997, AN3998, AN5027, AN5073, AN5647
- 디지털 필터: DT0088, DT0089, DT0091, DT0092
- SI/EMC/열: AN4803, AN4850, AN5036
- 제조·패키지: AN2639, TN1163, TN1204~TN1208, TN1433
- 경계 스캔/모델링: BSDL, IBIS, SVD

---

## 5. STM32F411에서 특히 주의할 점

1. **문서 제목에 STM32가 포함되어도 F411에서 모든 기능이 지원되는 것은 아닙니다.** AN은 여러 제품군을 포괄하는 경우가 많습니다.
2. **주변장치 유무와 핀 배치는 데이터시트 및 RM0383으로 확인합니다.** 특히 DAC, CAN, USB, RTC/TAMP 등은 F411의 실제 구현·기능과 핀 매핑을 확인한 뒤 실습 계획에 반영합니다.
3. **부트로더 인터페이스는 AN2606이 기준입니다.** USART/CAN/USB DFU/I2C/SPI용 AN이 각각 존재하더라도 F411 ROM 부트로더가 그 인터페이스를 지원한다는 뜻은 아닙니다.
4. **패키지에 따라 사용 가능한 핀과 대체 기능이 달라집니다.** 보드의 정확한 부품 전체 주문 코드와 패키지를 기준으로 핀맵을 작성합니다.
5. **Errata는 개발 마지막에 한 번 보는 문서가 아닙니다.** 주변장치 선택과 회로/펌웨어 구조를 결정하기 전에 확인하고, 검증 단계에서 다시 확인합니다.
6. **SVD·IBIS·BSDL은 일반적인 첫 학습 문서가 아닙니다.** 각각 디버거 레지스터 뷰, 신호 무결성 시뮬레이션, 제조 경계 스캔 테스트에 필요한 설계 보조 데이터입니다.

---

## 6. 추천 실습 프로젝트와 문서 연결

| 프로젝트 | 핵심 기능 | 연결 문서 |
|---|---|---|
| 프로젝트 A: 최소 시스템 보드 | 전원, 클록, 리셋, SWD, GPIO | DS10314, RM0383, AN4488, AN2867, AN4899 |
| 프로젝트 B: 센서 데이터 수집기 | ADC, DMA, 타이머 트리거, UART 로깅 | RM0383, AN2834, AN4031, AN4013, AN4646 |
| 프로젝트 C: 모터/액추에이터 제어 | PWM, 입력 캡처, 타이머 인터럽트, 비상 차단 | AN4013, AN4776, AN4277, AN4646 |
| 프로젝트 D: 통신 확장 보드 | USART, I2C, SPI, RS-485 | RM0383, AN3070, AN4655, AN4908 |
| 프로젝트 E: 다운로드·디버깅 실습 | SWD, ROM 부트로더, USART 다운로드, Fault 분석 | AN2606, AN3155, AN4989, PM0214, ES0287 |
| 프로젝트 F: 신뢰성 보드 검증 | EMC/ESD, 전원·열, SI, 실장 | AN1709, AN5612, AN5036, AN4803, AN2639 |

---

## 7. PDF를 읽을 때 권장하는 기록 양식

각 문서를 읽을 때 아래 항목을 한 페이지에 정리하면 나중에 회로 설계와 펌웨어 실습으로 연결하기 쉽습니다.

- **문서 번호 / 개정판 / 발행일**
- **대상 MCU 및 적용 조건**
- **핵심 개념 3~5개**
- **F411에서 직접 사용할 수 있는 기능**
- **관련 레지스터 / 핀 / 클록**
- **회로 설계 시 주의사항**
- **펌웨어 구현 절차**
- **실습 및 검증 방법**
- **다른 문서와의 연결**
- **미해결 질문**

### 최종 추천

처음에는 모든 PDF를 순서대로 읽기보다 다음 10개를 중심으로 실습을 진행하는 것이 가장 효율적입니다.

1. DS10314 — Datasheet
2. RM0383 — Reference Manual
3. PM0214 — Cortex-M4 Programming Manual
4. ES0287 — Errata
5. AN4488 — Hardware Development
6. AN4899 — GPIO
7. AN4013 — Timers
8. AN4776 — General-purpose Timer
9. AN4031 — DMA
10. AN2834 — ADC Accuracy

이 10개를 바탕으로 GPIO → Timer/PWM → ADC/DMA → USART/I2C/SPI → Bootloader/Debug 순으로 실습을 확장하고, 나머지 Application Note는 프로젝트 주제에 맞춰 참조하는 방식을 권장합니다.
