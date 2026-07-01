# NUCLEO 보드에서 ST-LINK가 갑자기 동작하지 않는 경우

## 증상

-   ST-LINK LED가 전혀 켜지지 않음
-   Windows에서 ST-LINK 장치가 나타나지 않음
-   Virtual COM Port도 사라짐
-   STM32CubeProgrammer에서 ST-LINK가 보이지 않음

이 경우는 일반적인 Target MCU Flash 손상이 아니라 **ST-LINK 부분 자체가
동작하지 않는 상태**일 가능성이 높습니다.

반대로 Target MCU(STM32)가 망가졌더라도 ST-LINK는 보통 정상적으로
동작합니다.

    USB
     │
     ├── ST-LINK MCU (F103/F723 등)
     │      │
     │      ├─ USB
     │      ├─ Virtual COM
     │      └─ SWD
     │
     └── Target STM32

## 자주 보고되는 원인

1.  ST-LINK Firmware Update 실패
2.  ST-LINK Firmware 영역 손상
3.  외부 회로의 역전압 또는 ESD
4.  3.3V 전원 쇼트
5.  USB 커넥터 손상
6.  ST-LINK MCU 자체 손상

## 다운로드 중 발생했다면

### 1. Firmware Update 실패

다운로드 또는 펌웨어 업그레이드 도중 USB가 끊기면서 Firmware 일부만
기록되어 USB 인식이 되지 않는 경우입니다.

### 2. ST-LINK MCU Flash 손상

Boot ROM은 살아 있지만 Firmware 영역이 손상되어 USB Enumeration이 되지
않을 수 있습니다.

### 3. 전원 문제

외부 보드에서 역전류가 유입되어 LDO나 보호회로가 손상되는 사례가
있습니다.

### 4. ST-LINK MCU 자체 손상

과전압, ESD 등으로 MCU가 손상되면 USB 자체가 동작하지 않습니다.

## LED가 완전히 꺼진 경우

LED가 전혀 켜지지 않고 USB도 인식되지 않는다면 Firmware 문제보다는
하드웨어 문제 가능성이 높습니다.

가능성이 높은 순서

-   3.3V 전원 불량
-   LDO 손상
-   ST-LINK MCU 손상
-   USB 회로 손상

## COM Port도 사라진 경우

Virtual COM은 ST-LINK Firmware에서 제공되므로 COM Port가 사라졌다면
Target MCU보다 ST-LINK 문제일 가능성이 높습니다.

## 권장 점검 순서

1.  USB 데이터 케이블 교체
2.  다른 USB 포트 사용
3.  다른 PC에서 테스트
4.  Windows 장치 관리자 확인
5.  STLinkUpgrade로 Firmware Recovery 시도
6.  STM32CubeProgrammer 확인
7.  ST-LINK 3.3V 측정

## 교육 환경에서 권장 구성

    NUCLEO
          │
          ├── 내장 ST-LINK 미사용
          │
          └── 외장 ST-LINK V3 사용

여러 대를 운영하는 교육 환경에서는 외장 ST-LINK를 준비하면 유지보수가
훨씬 쉽습니다.

## 경험상 발생 빈도

  원인                        가능성
  --------------------------- --------
  USB 케이블 문제             ★★★★★
  USB 드라이버 또는 PC 문제   ★★★★☆
  ST-LINK Firmware 손상       ★★★☆☆
  Target MCU 설정 오류        ★★★☆☆
  ST-LINK MCU 하드웨어 손상   ★★☆☆☆
  ST-LINK Flash 완전 손상     ★☆☆☆☆

## 결론

ST-LINK IC 내부 Flash가 완전히 지워지는 경우는 드뭅니다.

실제 현장에서는 다음 순으로 많이 발생합니다.

1.  USB 케이블 문제
2.  USB 드라이버 문제
3.  ST-LINK Firmware 손상
4.  전원 또는 보호회로 문제
5.  ST-LINK MCU 하드웨어 손상

따라서 LED와 COM Port가 모두 사라졌다면 ST-LINK 자체의 전원 또는
하드웨어를 먼저 의심하는 것이 가장 합리적입니다.
