# GY-GPS6MV2 GPS 모듈 테스트 프로그램

<img width="403" height="217" alt="112" src="https://github.com/user-attachments/assets/7589c4d0-1193-4820-ae89-863db5318aaa" />

* gps_monitor.py

<img width="993" height="506" alt="001" src="https://github.com/user-attachments/assets/5330c876-3261-44c7-a196-3b1569d15505" />
<br>

* gps_monitor_gui.py

<img width="1202" height="832" alt="002" src="https://github.com/user-attachments/assets/96ba5705-9273-4b5d-b8ff-585cff169abe" />
<br>

## 📋 프로그램 기능

이 프로그램은 GY-GPS6MV2 GPS 모듈을 테스트하고 모니터링할 수 있습니다:

- ✅ 실시간 GPS 좌표 표시 (위도, 경도, 고도)
- ✅ 위성 개수 및 신호 상태 표시
- ✅ 각 위성의 고도각, 방위각, 신호강도(SNR) 표시
- ✅ GPS 고정 품질 정보
- ✅ 이동 속도 및 방향 표시
- ✅ 현재 위치를 지도에 표시 (HTML 지도 생성)
- ✅ 이동 경로 추적 및 시각화

## 🔧 하드웨어 연결

### GY-GPS6MV2 모듈 핀 배치
```
VCC  → 3.3V 또는 5V
GND  → GND
TX   → RX (시리얼 수신)
RX   → TX (시리얼 송신)
```

### USB-시리얼 변환기 사용 (권장)
- GPS 모듈의 TX → USB-시리얼 변환기의 RX
- GPS 모듈의 RX → USB-시리얼 변환기의 TX
- VCC, GND 연결

## 📦 필수 패키지 설치

### 1. Python 3가 설치되어 있는지 확인
```bash
python3 --version
```

### 2. 필요한 라이브러리 설치

**Windows:**
```bash
pip install pyserial pynmea2 folium
```

**Linux/Mac:**
```bash
pip3 install pyserial pynmea2 folium
```

또는

```bash
sudo apt-get install python3-serial  # Linux에서 시리얼 권한 문제 방지
pip3 install pyserial pynmea2 folium
```

## 🚀 사용 방법

### 1. 프로그램 실행

**Windows:**
```bash
python gps_monitor.py
```

**Linux/Mac:**
```bash
python3 gps_monitor.py
```

### 2. 시리얼 포트 입력

프로그램 실행 시 시리얼 포트를 입력해야 합니다:

**Windows:**
- `COM3`, `COM4`, `COM5` 등
- 장치 관리자에서 확인 가능

**Linux:**
- `/dev/ttyUSB0`, `/dev/ttyUSB1` 등
- 다음 명령으로 확인: `ls /dev/ttyUSB*`

**Raspberry Pi:**
- `/dev/ttyAMA0` 또는 `/dev/serial0`

### 3. GPS 신호 수신

- GPS 모듈을 **야외나 창가**에 배치하세요
- 실내에서는 신호 수신이 어려울 수 있습니다
- 첫 고정까지 2-5분 정도 소요될 수 있습니다 (Cold Start)

## 📊 화면 표시 정보

프로그램 실행 중 다음 정보가 표시됩니다:

```
==================================================================
              GY-GPS6MV2 GPS 모듈 모니터
==================================================================

📡 GPS 상태: GPS 고정
🛰️  위성 개수: 8개

📍 위치 정보:
   위도: 37.123456°
   경도: 127.123456°
   고도: 45.2m

🚗 속도: 0.0 km/h
🧭 방향: 123.4°

🕐 GPS 시각: 12:34:56 UTC

🛰️  위성 상세 정보 (총 8개):
----------------------------------------------------------------------
위성 번호    고도각      방위각      신호강도(SNR)  
----------------------------------------------------------------------
12         45°        120°       35 dB          
25         30°        250°       32 dB          
...
```

## 🗺️ 지도 생성

GPS 신호가 고정되면 자동으로 HTML 지도가 생성됩니다:

- `gps_map.html` - 실시간 업데이트 (10초마다)
- `gps_map_final.html` - 프로그램 종료 시 최종 지도

생성된 HTML 파일을 웹 브라우저로 열어서 확인할 수 있습니다.

## ⚠️ 문제 해결

### 1. 시리얼 포트를 찾을 수 없음

**Windows:**
```
장치 관리자 → 포트(COM & LPT) 에서 COM 포트 번호 확인
```

**Linux:**
```bash
# USB 시리얼 장치 확인
ls /dev/ttyUSB*
# 또는
dmesg | grep tty

# 권한 문제 해결
sudo chmod 666 /dev/ttyUSB0
# 또는
sudo usermod -a -G dialout $USER  # 재로그인 필요
```

### 2. GPS 신호를 받을 수 없음

- GPS 모듈을 야외나 창가에 배치
- 안테나가 위를 향하도록 배치
- 처음 사용 시 Cold Start로 5분 정도 소요
- 전원 공급이 충분한지 확인 (5V 사용 권장)

### 3. 데이터 파싱 오류

- Baudrate 확인 (기본값: 9600)
- 시리얼 연결 확인 (TX-RX 교차 연결)
- GPS 모듈 전원 확인

### 4. 지도가 생성되지 않음

- GPS 고정이 완료되었는지 확인
- 인터넷 연결 확인 (OpenStreetMap 타일 다운로드)

## 🔍 GPS 고정 품질 상태

- **0**: GPS 신호 없음
- **1**: GPS 고정 (일반 GPS)
- **2**: DGPS 고정 (차등 GPS)
- **3**: PPS 고정
- **4**: RTK 고정 (Real-Time Kinematic)
- **5**: Float RTK
- **6**: 추측 항법

## 📝 코드 커스터마이징

### 시리얼 포트 기본값 변경

`gps_monitor.py` 파일에서:

```python
# Windows
default_port = 'COM3'

# Linux
default_port = '/dev/ttyUSB0'
```

### Baudrate 변경

```python
gps = GPSMonitor(port=port, baudrate=9600)  # 9600을 원하는 값으로 변경
```

### 지도 업데이트 주기 변경

```python
# 10초마다 업데이트 → 5초로 변경
if gps.latitude and gps.longitude and (current_time % 5 < 0.1):
```

## 📚 NMEA 문장 종류

이 프로그램이 처리하는 NMEA 문장:

- **GPGGA / GNGGA**: GPS 고정 데이터 (위치, 고도, 위성 개수)
- **GPRMC / GNRMC**: 권장 최소 GPS 데이터 (속도, 방향)
- **GPGSV / GNGSV**: 위성 정보 (고도각, 방위각, 신호강도)

## 🛠️ 기술 스택

- **Python 3**: 메인 프로그래밍 언어
- **pyserial**: 시리얼 통신
- **pynmea2**: NMEA 문장 파싱
- **folium**: 지도 시각화 (Leaflet.js 기반)

## 📄 라이센스

이 코드는 교육 및 개인 프로젝트 용도로 자유롭게 사용할 수 있습니다.

## 💡 팁

1. GPS 모듈을 처음 사용할 때는 Cold Start 시간이 걸립니다 (최대 5분)
2. 실내에서는 창가에 배치하여 테스트하세요
3. 이동하면서 테스트하면 경로 추적 기능을 확인할 수 있습니다
4. 위성이 8개 이상 잡히면 정확도가 높아집니다

## 📞 문의

문제가 발생하거나 개선 사항이 있으면 알려주세요!
