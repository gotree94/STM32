# Web Control (node.js)

## 1. Node.js install

   * https://nodejs.org/ko
   * 다운로드 : https://nodejs.org/dist/v24.13.0/node-v24.13.0-x64.msi

## 2. 서버 파일 설치

---


# STM32 RC Car 웹 컨트롤러 (Node.js)

PC에서 Node.js 웹서버를 실행하여 브라우저로 RC카를 제어합니다.

## 시스템 구성

```
[스마트폰/PC 브라우저] --WiFi--> [PC Node.js서버] --USB/Serial--> [STM32] --> [모터]
```

## 설치 및 실행

### 1. Node.js 설치
- https://nodejs.org 에서 LTS 버전 다운로드 및 설치

### 2. 프로젝트 설치
```bash
cd rc_car_nodejs
npm install
```

### 3. 하드웨어 연결
- STM32 UART를 USB-Serial 변환기로 PC에 연결
  - STM32 TX (PA9) → USB-Serial RX
  - STM32 RX (PA10) → USB-Serial TX
  - GND 공통 연결

### 4. 서버 실행
```bash
npm start
# 또는
node server.js
```

### 5. 브라우저 접속
- **PC**: http://localhost:3000
- **스마트폰**: http://<PC의 IP>:3000 (같은 WiFi)

## 조작 방법

### 버튼/터치
| 버튼 | 동작 |
|------|------|
| ▲ | 전진 (누르는 동안) |
| ▼ | 후진 (누르는 동안) |
| ◀ | 좌회전 (누르는 동안) |
| ▶ | 우회전 (누르는 동안) |
| ✕ | 정지 |

### 키보드 (PC)
| 키 | 동작 |
|----|------|
| W / ↑ | 전진 |
| S / ↓ | 후진 |
| A / ← | 좌회전 |
| D / → | 우회전 |
| X / Space | 정지 |
| 0-9 | 속도 조절 |

## API 엔드포인트

| URL | 설명 |
|-----|------|
| GET / | 웹 컨트롤러 페이지 |
| GET /cmd?c=W | 명령 전송 (W/S/A/D/X/0-9) |
| GET /status | 현재 상태 조회 |
| GET /ports | 시리얼 포트 목록 |
| GET /connect?port=COM3 | 특정 포트 연결 |

## 문제 해결

### 시리얼 포트 연결 안됨
1. USB 케이블 확인
2. http://localhost:3000/ports 에서 포트 목록 확인
3. Windows: 장치관리자에서 COM 포트 확인
4. Linux/Mac: `ls /dev/tty*` 로 포트 확인

### 모바일에서 접속 안됨
1. PC와 같은 WiFi 네트워크인지 확인
2. PC 방화벽에서 3000번 포트 허용
3. http:// 프로토콜 사용 (https 아님)

### serialport 설치 오류 (Windows)
```bash
npm install --global windows-build-tools
npm install serialport
```

## STM32 명령 프로토콜

| 명령 | 동작 |
|------|------|
| W | 전진 |
| S | 후진 |
| A | 좌회전 |
| D | 우회전 |
| X | 정지 |
| 0-9 | 속도 (0=정지, 9=최대) |
