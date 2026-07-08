# EAI S4 LiDAR 분석 노트

## 하드웨어 정보

| 항목 | 내용 |
|---|---|
| 제품명 | EAI LiDAR S4 Mainboard BL V1.4 TX JAN |
| 제조사 | YDLIDAR (EAIBOT) |
| 통신 방식 | UART (3.3V) |
| 연결 포트 | Prolific USB-to-Serial Comm Port (COM6) |
| 측정 방식 | 삼각측량 (Triangle) |
| 측정 범위 | 0.10 ~ 8.0 m |
| 측정 주파수 | 4 KHz (samples/sec) |
| 스캔 주파수 | ~7.4 Hz (모터 PWM 제어) |

## 시리얼 설정

| 파라미터 | 값 |
|---|---|
| Baud rate | **115200** |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |

## 프로토콜

### 명령어

| 명령어 | 바이트 | 설명 |
|---|---|---|
| 스캔 시작 | `A5 60` | 지속 스트리밍 모드 시작 |
| 스캔 중지 | `A5 65` | 스캔 종료 |
| 장치 정보 | `A5 90` | 모델, 펌웨어 버전 등 조회 |
| 상태 확인 | `A5 91` | 상태 정보 조회 |

### 스캔 데이터 패킷 구조

`A5 60` 전송 후, LiDAR는 바로 `AA 55` 헤더의 패킷을 연속 스트리밍한다. (`A5 5A` 응답 헤더 없음)

```
Byte: [0-1] [2] [3] [4-5] [6-7] [8-9] [10...]
      PH    CT  LSN  FSA   LSA   CS    Si (2B * LSN)
```

| 필드 | 크기 | 설명 |
|---|---|---|
| **PH** | 2B | 패킷 헤더, 고정 `0x55AA` (LE: `AA 55`) |
| **CT** | 1B | 패킷 타입. bit0=1: 영점(zero/rotation start) 패킷, bit0=0: 일반 데이터 |
| **LSN** | 1B | 이 패킷에 포함된 샘플 개수 (보통 1~40) |
| **FSA** | 2B | 시작 각도 (Little-Endian) |
| **LSA** | 2B | 종료 각도 (Little-Endian) |
| **CS** | 2B | 체크섬 (아래 알고리즘 참조) |
| **Si** | 2B*LSN | 샘플 데이터 (개별 거리값) |

### 각도 계산

```python
# 원시 16비트 값에서 각도 추출 (bit0 = check bit, 항상 1)
angle_deg = (raw_angle >> 1) / 64.0

# 샘플별 각도는 FSA ~ LSA 사이 선형 보간
if LSN == 1:
    sample_angle = start_angle
else:
    step = (end_angle - start_angle) / (LSN - 1)
    sample_angle = start_angle + step * i   # i = 0..LSN-1

# 360도 랩어라운드 처리
if end_angle < start_angle:
    end_angle += 360.0
if sample_angle >= 360.0:
    sample_angle -= 360.0
```

### 거리 계산

```python
dist_mm = si >> 2                           # mm 단위
dist_m  = dist_mm / 4000.0                  # 미터 단위 (삼각측량 방식)
```

### 삼각측량 각도 보정

실제 각도는 거리에 따라 아래 공식으로 보정한다:

```python
import math
if dist_mm > 0:
    correction = math.atan(21.8 * (155.3 - dist_mm) / (155.3 * dist_mm))
    correction_deg = correction * 180.0 / math.pi
    angle_corrected = sample_angle + correction_deg
```

### 체크섬 (YDLIDAR SDK 알고리즘)

16비트 워드 단위 XOR. 순서는 다음과 같다:

```
CS = PH ^ FSA ^ Si[0] ^ Si[1] ^ ... ^ Si[N-1] ^ (LSN<<8 | CT) ^ LSA
```

```python
import struct

def calc_checksum(packet, lsn):
    ph  = struct.unpack_from('<H', packet, 0)[0]
    fsa = struct.unpack_from('<H', packet, 4)[0]
    lsa = struct.unpack_from('<H', packet, 6)[0]
    ct  = packet[2]

    cs = ph ^ fsa
    for i in range(lsn):
        si = struct.unpack_from('<H', packet, 10 + i*2)[0]
        cs ^= si
    cs ^= (lsn << 8) | ct
    cs ^= lsa
    return cs
```

## 스크립트 사용법

### 1. 실시간 로깅 (`s4_lidar_logger.py`)

```bash
python s4_lidar_logger.py
```

- COM6 포트 자동 연결 (스크립트 내 `COM_PORT` 수정 가능)
- 실행 즉시 스캔 시작
- Ctrl+C 로 중지
- 출력: `lidar_logs/s4_raw_*.log` (hex dump) + `s4_parsed_*.csv` (파싱 데이터)

### 2. 로그 분석 (`s4_analyze_log.py`)

```bash
# raw hex dump 로그 분석
python s4_analyze_log.py lidar_logs/s4_raw_20260708_*.log

# CSV 분석
python s4_analyze_log.py lidar_logs/s4_parsed_20260708_*.csv
```

### 3. CSV 출력 컬럼

| 컬럼 | 설명 |
|---|---|
| `timestamp` | 수신 시각 |
| `session_time_ms` | 세션 시작 후 경과 시간 (ms) |
| `packet_type` | ZERO (회전 시작) / DATA |
| `lsn` | 해당 패킷의 샘플 개수 |
| `idx` | 패킷 내 샘플 인덱스 |
| `angle_raw_deg` | 보간 원시 각도 (도) |
| `angle_corr_deg` | 삼각측량 보정 각도 (도) |
| `dist_mm` | 거리 (mm) |
| `dist_m` | 거리 (m) |
| `intensity` | 간섭 플래그 (0/1, bit1 of Si) |
| `checksum_ok` | 체크섬 검증 결과 (True/False) |

## 참고 자료

- GitHub: [YDLIDAR/YDLidar-SDK](https://github.com/YDLIDAR/YDLidar-SDK)
- YDLIDAR 공식 문서: [YDLidar-SDK-Communication-Protocol.md](https://github.com/YDLIDAR/YDLidar-SDK/blob/master/doc/YDLidar-SDK-Communication-Protocol.md)
- ROS 드라이버: [ydlidar_ros_driver](https://github.com/YDLIDAR/ydlidar_ros_driver)
