# Car Control — Serial + Voice + DroidCam

Node.js 서버로 자동차를 시리얼 포트로 제어하고, 웹 브라우저에서 음성인식 및 DroidCam 영상을 함께 사용합니다.

![](27.png)

---

## 시스템 구성

```
스마트폰/PC 브라우저 (4블록 UI)
├── 🔌 Control  — 포트 연결 + D-Pad (키보드)
├── 🎤 Voice    — Web Speech API 음성인식
├── 📷 DroidCam — 실시간 영상 스트리밍
└── 📋 Log      — 명령/이벤트 로그 콘솔
        ↓ HTTP / HTTPS (REST API)
    Node.js 서버 (Express)
        ├── /api/command       — 명령 전송
        ├── /api/ports         — 포트 목록
        ├── /api/connect       — 포트 연결
        ├── /api/disconnect    — 포트 해제
        ├── /api/status        — 연결 상태
        └── /api/camera/stream — DroidCam MJPEG 프록시
            ↓ USB Serial
        Arduino (HC-05/Serial)
            ↓
        자동차 (모터 드라이버 / 릴레이)
```

---

## 화면 구성 (4블록)

| # | 블록 | 설명 |
|---|------|------|
| 1 | **🔌 Control** | COM 포트 선택/연결, D-Pad 방향키 (W/A/S/D/X) |
| 2 | **🎤 Voice** | 마이크 버튼, 음성인식 상태 표시, 지원 명령어 힌트 |
| 3 | **📷 DroidCam** | IP 입력(기록), Connect/Disconnect, 실시간 MJPEG 영상 |
| 4 | **📋 Log** | 모든 이벤트 시간순 로그, Clear/Auto Scroll, 엔트리 카운터 |

### 반응형 레이아웃

| 화면 너비 | 배치 | 적용 |
|-----------|------|------|
| **< 820px** (스마트폰) | **세로 1열** — 4블록 순서대로 적층 | `flex-direction: column` |
| **>= 820px** (태블릿/PC) | **2 × 2 Grid** — 4블록 격자 배치 | `grid-template-columns: 1fr 1fr` |
| **>= 1100px** (와이드) | 2×2 유지, 카메라 16:9, 로그 확대 | 컨테이너 최대폭 1100px |

---

## 지원 명령어

| 기능 | 키보드 | 음성 (한국어) | 음성 (영어) |
|------|--------|-------------|------------|
| 전진 | W | "전진", "앞으로", "포워드" | "forward", "front" |
| 좌회전 | A | "좌회전", "왼쪽", "레프트" | "left", "turn left" |
| 후진 | S | "후진", "뒤로", "백워드" | "backward", "back" |
| 우회전 | D | "우회전", "오른쪽", "라이트" | "right", "turn right" |
| 정지 | X | "정지", "멈춰", "스톱", "브레이크" | "stop", "halt", "brake" |

---

## 주요 기능

### 🎤 음성 제어 (Web Speech API)
- Chrome/Edge/Safari(iOS 14.5+) 브라우저 내장 음성인식 사용
- `SpeechRecognition` + `webkitSpeechRecognition` 접두사 자동 감지
- 한국어(`ko-KR`) 지원, 한글/영어 명령어 동시 매핑
- Push-to-talk 방식 (🎤 버튼 누르고 말하기)
- **iOS 대응**: 마이크 워밍업(`getUserMedia`), 첫 인식 실패 방지, `no-speech` 에러 무시 처리

### 📷 DroidCam 영상
- DroidCam 앱(안드로이드) → `http://<ip>:4747/video` MJPEG 스트림
- 서버 프록시(`/api/camera/stream?ip=...`)로 중계 → mixed content 문제 해결
- **IP 기록**: 연결 성공한 IP를 `localStorage`에 저장 (최대 10개)
- **칩(Chip)** 클릭 → IP 입력 + 자동 연결
- **datalist** 자동완성 드롭다운
- 개별 삭제(✕), 전체 삭제(🗑), 입력 초기화(✕)

### 📋 로그 콘솔
- 모든 이벤트 시간순 기록
- 태그별 색상 구분: `[SENT]` 파랑, `[RECOG]` 초록, `[CAM]` 주황, `[INFO]` 회색, `[ERROR]` 빨강
- **Auto ▼** 버튼 — 자동 스크롤 on/off
- **Clear** 버튼 — 로그 초기화
- 우측 `N entries` 카운터

### 🔐 HTTPS 지원
- `selfsigned` 패키지로 자체서명 인증서 서버 첫 실행 시 자동 생성
- HTTP(`localhost:3000`) + HTTPS(`:3443`) 동시 서빙
- iPhone에서 Web Speech API 동작에 필수 (secure context 요구사항)

### ⌨️ 키보드 제어
- W/A/S/D/X 키를 누르면 즉시 명령 전송 (keydown 이벤트)
- D-Pad 버튼 클릭과 동일한 동작

---

## 설치 및 실행

### 필요 파일

```
├── package.json
├── package-lock.json
├── server.js
└── public/
    └── index.html
```

`node_modules/`와 `.cert/` 폴더는 각각 `npm install`과 서버 최초 실행 시 자동 생성됩니다.

### 실행

```bash
cd C:\Users\Administrator\Desktop\Service2
npm install
npm start
```

### 접속 주소

| 기기 | 주소 | 용도 | 비고 |
|------|------|------|------|
| PC | `http://localhost:3000` | 전체 기능 | Chrome 권장 |
| iPhone | `https://192.168.x.x:3443` | 음성 + DroidCam | 자체서명 인증서 주의 |

서버 실행 시 콘솔에 표시되는 IP와 포트를 확인하세요.

---

## 상세 사용법

### 1. 포트 연결
1. 드롭다운에서 Arduino COM 포트 선택
2. **Connect** 클릭 → 상태가 `Connected: COMx`로 변경
3. 로그에 `[INFO] Connected to COMx` 기록

### 2. 방향 제어
- **D-Pad 버튼** 클릭 또는 **키보드 W/A/S/D/X** 입력
- 명령이 시리얼로 전송되고 로그에 `[SENT] → W` 기록
- 연결되지 않은 상태에서 명령 시도 시 로그에 `[ERROR]` 기록

### 3. 음성 제어
1. **🎤 마이크 버튼** 탭 (빨간색 펄스 = 듣는 중)
2. 명령어 말하기 (예: "전진", "정지", "좌회전", "left", "stop")
3. 인식 성공 시 로그에 `[RECOG] "전진" → W` 기록 후 명령 자동 전송
4. 다시 🎤 버튼 탭하여 종료

### 4. DroidCam 연결
1. 자동차 안드로이드 폰에서 **DroidCam 앱** 실행 (Wi-Fi 같은 네트워크)
2. 앱에 표시된 IP 주소 입력 후 **Connect**
3. 실시간 영상 확인, 연결 성공 시 IP가 칩 형태로 저장
4. 다음 접속: IP 칩 클릭 → 자동 연결

### 5. 로그 활용
- 모든 동작이 실시간 로그로 기록됨
- **Auto ▼** 켜짐 상태면 새 로그가 추가될 때 자동 스크롤
- 필요 시 **Clear**로 초기화

---

## API 엔드포인트

| Method | Path | 설명 |
|--------|------|------|
| GET | `/api/ports` | 사용 가능한 시리얼 포트 목록 |
| POST | `/api/connect` | 포트 연결 (`{ path: "COM3" }`) |
| POST | `/api/disconnect` | 포트 해제 |
| GET | `/api/status` | 연결 상태 확인 |
| POST | `/api/command` | 명령 전송 (`{ cmd: "w" }`) |
| GET | `/api/camera/stream?ip=...` | DroidCam MJPEG 스트림 프록시 |

---

## 개발 스택

| 계층 | 기술 |
|------|------|
| Frontend | HTML5, CSS3 (Grid/Flexbox), Web Speech API, localStorage |
| Backend | Node.js, Express |
| 하드웨어 통신 | SerialPort (USB) |
| 영상 | MJPEG over HTTP, 서버 프록시 |
| 인증서 | selfsigned (자체서명) |

---

## 주의사항

- **음성인식**: Chrome/Edge/Safari(iOS 14.5+) 필요, **HTTPS 환경에서만 동작**
- **iPhone 접속**: 자체서명 인증서 사용 → Safari "고급 → [IP]로 이동", Chrome "고급 → 계속 진행"
- **DroidCam**: 안드로이드 폰과 제어기가 **같은 Wi-Fi 네트워크**에 있어야 함
- **포트**: HTTP 3000, HTTPS 3443 사용
- **iOS Chrome**: iOS의 모든 브라우저는 WebKit 기반 → Web Speech API는 Safari와 동일하게 동작
