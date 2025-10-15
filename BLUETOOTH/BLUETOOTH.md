# BLUETOOTH Module

---
## 1. BT06 DM2147
<img width="442" height="173" alt="BT06_DM2147" src="https://github.com/user-attachments/assets/9b92e6a3-71bf-4c2b-843f-63ae17afe8d0" />

- 블루투스 2.1 버전 모듈입니다.
- HC-06 을 대체할 수 있습니다.
 
---

## 2. HC-06

<img width="357" height="150" alt="HC-06" src="https://github.com/user-attachments/assets/8068f470-c33a-4a62-8abf-83f56e78a098" />


- 블루투스 V2.0 프로토콜 기반의 CSR 블루투스 칩 사용
- 동작전압 3.3V
- UART : 1200,2400,4800,9600,19200,38400,57600,115200 bps
- 사이즈 : 28mm * 15mm * 2.35mm
- 동작전류
  - 동작 : 40mA
  - 대기전류 <1mA
* 기본설정 : Slave, 9600 baud rate, N,8,1 / Pincode 1234

### AT 명령어1
- 통신테스트
  - Sent : AT
  - Receive : OK
- Baudrate 변경
  - Sent : AT+UART=9600,0,0
  - Receive : OK
 - 9600 부분은 이하의 값으로 변경할 수 있습니다.
   - 1200,2400,4800,9600,19200,38400,57600,115200 bps
 - Baud rate 설정은 전원을 차단하시더라도 저장되어 있습니다.

- 블루투스 이름 변경
  - Sent : AT+PSWD:"xxxx"
  - Receive : OK
  - xxxx는 설정되는 pin code값입니다.
  - pin code 설정은 전원 OFF시 저장할 수 있습니다.
 
- 초기화
  - Sent : AT+ORGL
  - Receive : OK
  - 모듈은 공장 출고상태로 초기화 합니다.
  - Pin code는 "1234" 또는 "0000" 입니다.

### AT 명령어2
- 통신테스트
  - Sent : AT
  - Receive : OK
- Baudrate 변경
  - Sent : AT+BAUD1
  - Receive : OK1200
  - Sent : AT+BAUD2
  - Receive : OK2400
    - 1200(1),2400(2),4800(3),9600(4),19200(5),38400(6),57600(7),115200(8) bps
 - Baud rate 설정은 전원을 차단하시더라도 저장되어 있습니다.

- 블루투스 이름 변경
  - Sent : AT+NAMEdevicename
  - Receive : OKname

- Pin code 변경
  - Sent : AT+PINxxxx
  - Receive : OKsetpin
  - xxxx는 설정되는 pin code 값입니다.

---


## 3. CSR_BC41C

<img width="395" height="260" alt="CSR_BC41C" src="https://github.com/user-attachments/assets/6faec10a-d41e-4233-a3dd-c0738e9bebb3" />

- HC-06 슬레이브 전용 모듈
- HC-06 마스터 모듈 1개와 1:1 통신 가능
- CSR Bluetooth Chip이 장착된 모듈
- Bluetooth V2.0 표준2 프로토콜 적용
- 3.3V 에서 동작
- UART : 1200,2400,4800,9600,19200,38400,57600,115200 bps
- 사이즈 : 28mm * 15mm * 2.35mm 
- 동작전류
  - 페어링 될때 까지 30~40mA
  - 통신대기시 2~8mA
  - 페어링시 8mA
  - 슬립시 슬립
* GPS 네비게이션 시스템, 각종 응용 검침 시스템, 산업현장 마이닝 제어 시스템 및 기타 장치에 다양하게 응용 가능.
* 블루투스 장착된 노트북, 컴퓨터, PDA, 스마트폰 등 기타 장치와 원활한 여결이 가능.

---
