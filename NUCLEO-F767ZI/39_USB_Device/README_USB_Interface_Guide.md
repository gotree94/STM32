# 📄 USB Class / DAQ / 영상 스캐너 인터페이스 가이드

본 문서는 USB 기반 장비 개발 시  
**USB Class 선택, PC 드라이버 요구사항, 소프트웨어 연동 방식**을 한눈에 비교·정리한 가이드입니다.

---

## 1️⃣ USB Class 드라이버 가이드

| USB Class | PC 드라이버 | PC 소프트웨어 | 비고 |
|---|---|---|---|
| MSC (Mass Storage) | ❌ 불필요 | ❌ 불필요 | USB 메모리처럼 인식 |
| HID (키보드/마우스) | ❌ 불필요 | ❌ 불필요 | 입력 장치 |
| CDC (Virtual COM) | △ 일부 필요 | 터미널 필요 | 가상 시리얼 |
| UVC (Camera) | ❌ 불필요 | ❌ 불필요 | 웹캠 호환 |
| Audio | ❌ 불필요 | ❌ 불필요 | USB 오디오 |
| USBTMC (계측기) | ✅ 필요 | LabVIEW, MATLAB 등 | NI-VISA |
| Custom (Vendor) | ✅ 필요 | 직접 개발 | WinUSB 기반 |

---

## 2️⃣ DAQ 장비 – LabVIEW 호환 구조  
### (USBTMC + SCPI)

```
┌───────────────┐         ┌───────────────┐
│   STM32 DAQ   │  SCPI   │    LabVIEW    │
│               │ ◀─────▶ │    MATLAB     │
│  USBTMC Class │  명령어  │    Python     │
└───────────────┘         └───────────────┘
```

### 📌 PC 요구사항
- **드라이버**: NI-VISA (필수)
- **소프트웨어**
  - LabVIEW
  - MATLAB
  - Python (`pyvisa`)

### 📜 SCPI 명령어 예시
```
*IDN?            → 장치 정보 요청
MEAS:VOLT? 0     → 채널 0 전압 측정
DIG:OUTP 0xFF    → 디지털 출력 제어
```

---

## 3️⃣ 영상 스캐너 – TWAIN / WIA / UVC 비교

| 방식 | PC 드라이버 | 개발 난이도 | 권장 용도 |
|---|---|---|---|
| UVC | ❌ 불필요 | ⭐⭐⭐⭐ | 실시간 카메라 (권장) |
| TWAIN | ✅ DS 개발 | ⭐⭐⭐⭐⭐ | 문서 스캐너 |
| WIA | ✅ WDK 필요 | ⭐⭐⭐⭐⭐ | Windows 전용 |
| MSC + 파일 | ❌ 불필요 | ⭐ | 단순 이미지 전송 |

### 📌 권장 선택
- **실시간 영상** → `UVC` (드라이버 불필요)
- **단순 이미지 전송** → `MSC + 파일`
- **전문 스캐너 / Photoshop 연동** → `TWAIN`

---

## 💡 핵심 결론 요약

| 장비 유형 | 권장 방식 | PC 드라이버 |
|---|---|---|
| 파일 전송 장비 | USB MSC | ❌ 불필요 |
| DAQ / 계측기 | USBTMC + SCPI | NI-VISA |
| 카메라 / 영상 장치 | UVC | ❌ 불필요 |
| 고급 문서 스캐너 | TWAIN | 직접 개발 |

---

## 🧭 설계 가이드 한 줄 요약

- **단순·안정성 우선** → 표준 USB Class 사용
- **계측·자동화** → USBTMC + SCPI
- **영상 장비** → UVC
- **전문 스캐너** → TWAIN

---

📎 본 문서는 GitHub `README.md` 용도로 작성되었습니다.
