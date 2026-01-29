# STM32 영상 스캐너 장비 - TWAIN/WIA 호환 시스템

STM32 기반 영상 스캐너/카메라 장비를 PC의 표준 스캐너 드라이버(TWAIN, WIA) 및 영상 소프트웨어와 호환되도록 개발하는 가이드입니다.

## 📋 개요

| 항목 | 내용 |
|------|------|
| 목표 | Photoshop, 문서 스캔 프로그램에서 사용 가능한 스캐너 |
| 프로토콜 | **TWAIN**, **WIA**, 또는 **UVC** |
| 드라이버 | TWAIN DS, WIA 드라이버, 또는 OS 내장 UVC |

## 🎯 영상 전송 방식 비교

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    영상 스캐너/카메라 PC 전송 방식                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐  │
│   │  Option 1: UVC (USB Video Class) - 드라이버 불필요 ⭐ 권장          │  │
│   ├─────────────────────────────────────────────────────────────────────┤  │
│   │  • OS 내장 드라이버 사용                                            │  │
│   │  • 카메라 앱, Zoom, Skype 등에서 자동 인식                         │  │
│   │  • 실시간 영상 스트리밍                                             │  │
│   │  • 개발 난이도: ⭐⭐⭐⭐                                             │  │
│   └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐  │
│   │  Option 2: TWAIN - 드라이버 개발 필요                               │  │
│   ├─────────────────────────────────────────────────────────────────────┤  │
│   │  • 스캐너 표준 인터페이스                                           │  │
│   │  • Photoshop, GIMP 등에서 "Import" 메뉴로 접근                     │  │
│   │  • TWAIN Data Source (DS) 드라이버 개발 필요                       │  │
│   │  • 개발 난이도: ⭐⭐⭐⭐⭐                                           │  │
│   └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐  │
│   │  Option 3: WIA (Windows Image Acquisition) - 드라이버 개발 필요    │  │
│   ├─────────────────────────────────────────────────────────────────────┤  │
│   │  • Windows 전용                                                     │  │
│   │  • Windows 팩스 및 스캔, Paint 등에서 인식                         │  │
│   │  • WIA 미니드라이버 개발 필요 (WDK 사용)                           │  │
│   │  • 개발 난이도: ⭐⭐⭐⭐⭐                                           │  │
│   └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐  │
│   │  Option 4: USB MSC + 이미지 파일 - 드라이버 불필요                 │  │
│   ├─────────────────────────────────────────────────────────────────────┤  │
│   │  • 스캔 결과를 파일로 저장 후 USB 드라이브로 접근                  │  │
│   │  • 가장 간단한 방식                                                 │  │
│   │  • 실시간 스트리밍 불가                                             │  │
│   │  • 개발 난이도: ⭐                                                   │  │
│   └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 📊 방식별 상세 비교

| 방식 | PC 드라이버 | PC 소프트웨어 | 실시간 | 개발 난이도 | 권장 용도 |
|------|------------|--------------|--------|------------|----------|
| **UVC** | ❌ 불필요 | ❌ 불필요 | ✅ | ⭐⭐⭐⭐ | 실시간 카메라 |
| **TWAIN** | ✅ 필요 | ❌ (표준 앱) | △ | ⭐⭐⭐⭐⭐ | 문서 스캐너 |
| **WIA** | ✅ 필요 | ❌ (Windows) | △ | ⭐⭐⭐⭐⭐ | Windows 스캐너 |
| **MSC + 파일** | ❌ 불필요 | ❌ 불필요 | ❌ | ⭐ | 단순 이미지 전송 |

---

## 📹 Option 1: UVC (USB Video Class) - 권장

### 장점

```
✅ 드라이버 설치 불필요
✅ Windows/Mac/Linux 모두 지원
✅ 실시간 영상 스트리밍
✅ 모든 카메라/영상 앱에서 인식
✅ Zoom, Teams, Skype 등에서 바로 사용
```

### 시스템 구성

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         UVC 카메라 시스템 구성                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   STM32F767 + 카메라 모듈                  PC                               │
│   ┌─────────────────────────┐             ┌─────────────────────────┐      │
│   │                         │             │                         │      │
│   │  ┌─────────────────┐    │             │    OS 내장 UVC 드라이버 │      │
│   │  │ Camera Sensor   │    │   USB HS    │    (자동)               │      │
│   │  │ (OV2640/OV5640) │    │◀──────────▶│                         │      │
│   │  └────────┬────────┘    │   MJPEG/    │    ┌─────────────────┐  │      │
│   │           │             │   YUV       │    │ 📷 카메라 앱    │  │      │
│   │  ┌────────▼────────┐    │             │    │ 📹 OBS Studio   │  │      │
│   │  │ DCMI Interface  │    │             │    │ 🎥 Zoom/Teams   │  │      │
│   │  └────────┬────────┘    │             │    │ 🖼️ OpenCV       │  │      │
│   │           │             │             │    └─────────────────┘  │      │
│   │  ┌────────▼────────┐    │             │                         │      │
│   │  │ USB UVC Stack   │    │             │                         │      │
│   │  │ (MJPEG Encoder) │    │             │                         │      │
│   │  └─────────────────┘    │             │                         │      │
│   │                         │             │                         │      │
│   └─────────────────────────┘             └─────────────────────────┘      │
│                                                                             │
│   지원 포맷:                                                                │
│   • MJPEG: 압축, 고해상도 가능, CPU 부하 있음                              │
│   • YUV (Uncompressed): 비압축, 대역폭 많이 필요                           │
│                                                                             │
│   해상도 예시 (USB HS 기준):                                                │
│   • MJPEG: 1920×1080 @ 30fps 가능                                          │
│   • YUV: 640×480 @ 30fps (대역폭 제한)                                     │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### UVC 구현 핵심 요소

#### USB Descriptor 구조

```c
/* UVC Descriptor 구조 (개략) */

// Video Control Interface
USB_INTERFACE_DESCRIPTOR     // bInterfaceClass = 0x0E (Video)
                            // bInterfaceSubClass = 0x01 (Video Control)
VC_HEADER_DESCRIPTOR
VC_INPUT_TERMINAL_DESCRIPTOR  // Camera Terminal
VC_OUTPUT_TERMINAL_DESCRIPTOR // USB Streaming Terminal
VC_PROCESSING_UNIT_DESCRIPTOR // 밝기, 대비 등 조절

// Video Streaming Interface
USB_INTERFACE_DESCRIPTOR     // bInterfaceClass = 0x0E (Video)
                            // bInterfaceSubClass = 0x02 (Video Streaming)
VS_INPUT_HEADER_DESCRIPTOR
VS_FORMAT_MJPEG_DESCRIPTOR   // MJPEG 포맷 정의
VS_FRAME_MJPEG_DESCRIPTOR    // 해상도, 프레임레이트 정의
```

#### MJPEG 프레임 전송

```c
/* UVC 프레임 전송 구조 */

typedef struct {
    uint8_t  bHeaderLength;      // 헤더 길이 (보통 2 또는 12)
    uint8_t  bmHeaderInfo;       // 프레임 정보 비트
    uint32_t dwPresentationTime; // 타임스탬프 (선택)
    uint8_t  scrSourceClock[6];  // 소스 클럭 (선택)
} UVC_PayloadHeader_t;

// bmHeaderInfo 비트 정의
#define UVC_HEADER_FID    (1 << 0)  // Frame ID (토글)
#define UVC_HEADER_EOF    (1 << 1)  // End of Frame
#define UVC_HEADER_PTS    (1 << 2)  // Presentation Time Stamp
#define UVC_HEADER_SCR    (1 << 3)  // Source Clock Reference
#define UVC_HEADER_STI    (1 << 5)  // Still Image
#define UVC_HEADER_ERR    (1 << 6)  // Error
#define UVC_HEADER_EOH    (1 << 7)  // End of Header

/**
  * @brief  MJPEG 프레임 전송
  */
void UVC_SendFrame(uint8_t *jpeg_data, uint32_t jpeg_size)
{
    static uint8_t frame_id = 0;
    uint32_t offset = 0;
    uint16_t packet_size;
    uint8_t tx_buffer[1024];
    
    while (offset < jpeg_size)
    {
        // 헤더 설정
        UVC_PayloadHeader_t *header = (UVC_PayloadHeader_t *)tx_buffer;
        header->bHeaderLength = 2;
        header->bmHeaderInfo = frame_id ? UVC_HEADER_FID : 0;
        
        // 마지막 패킷이면 EOF 설정
        uint32_t remaining = jpeg_size - offset;
        if (remaining <= (sizeof(tx_buffer) - 2))
        {
            packet_size = remaining;
            header->bmHeaderInfo |= UVC_HEADER_EOF;
        }
        else
        {
            packet_size = sizeof(tx_buffer) - 2;
        }
        
        // 데이터 복사
        memcpy(tx_buffer + 2, jpeg_data + offset, packet_size);
        
        // USB 전송
        USBD_LL_Transmit(&hUsbDevice, UVC_EP_IN, tx_buffer, packet_size + 2);
        
        offset += packet_size;
    }
    
    // 다음 프레임을 위해 ID 토글
    frame_id = !frame_id;
}
```

### OpenCV에서 사용 (Python)

```python
import cv2

# UVC 카메라 열기 (STM32가 카메라 장치로 인식됨)
cap = cv2.VideoCapture(0)  # 또는 1, 2... (카메라 인덱스)

# 해상도 설정
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    # 영상 처리
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    
    # 화면 표시
    cv2.imshow('STM32 Camera', frame)
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 🖨️ Option 2: TWAIN 드라이버 개발

### TWAIN 아키텍처

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         TWAIN 아키텍처                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ┌─────────────────┐                                                      │
│   │   Application   │  (Photoshop, GIMP, 스캔 프로그램)                    │
│   │                 │                                                      │
│   └────────┬────────┘                                                      │
│            │ TWAIN API                                                     │
│            ▼                                                               │
│   ┌─────────────────┐                                                      │
│   │  TWAIN Source   │  (TWAIN Manager - twain_32.dll)                     │
│   │    Manager      │                                                      │
│   └────────┬────────┘                                                      │
│            │ DS Entry Point                                                │
│            ▼                                                               │
│   ┌─────────────────┐                                                      │
│   │   Data Source   │  ← 🔧 개발 필요! (DLL 파일)                         │
│   │      (DS)       │     myScanner.ds                                     │
│   └────────┬────────┘                                                      │
│            │ Device I/O                                                    │
│            ▼                                                               │
│   ┌─────────────────┐                                                      │
│   │  STM32 Device   │  (USB 통신)                                         │
│   │                 │                                                      │
│   └─────────────────┘                                                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### TWAIN Data Source 개발

#### DS Entry Point

```c
/* twain_ds.c - TWAIN Data Source DLL */

#include <windows.h>
#include "twain.h"

// TWAIN 상태
static TW_UINT16 g_CurrentState = 1;  // 1-7 상태 머신
static TW_IDENTITY g_AppIdentity;
static TW_IDENTITY g_DSIdentity = {
    .Id = 0,
    .Version = { .MajorNum = 1, .MinorNum = 0 },
    .ProtocolMajor = 2,
    .ProtocolMinor = 3,
    .SupportedGroups = DG_IMAGE | DG_CONTROL,
    .Manufacturer = "MyCompany",
    .ProductFamily = "STM32 Scanner",
    .ProductName = "STM32-SCAN"
};

/**
  * @brief  TWAIN DS Entry Point (DLL Export)
  */
TW_UINT16 FAR PASCAL DS_Entry(
    pTW_IDENTITY pOrigin,      // 응용 프로그램 정보
    TW_UINT32    DG,           // Data Group
    TW_UINT16    DAT,          // Data Argument Type
    TW_UINT16    MSG,          // Message
    TW_MEMREF    pData)        // 데이터 포인터
{
    TW_UINT16 rc = TWRC_SUCCESS;
    
    switch (DG)
    {
        case DG_CONTROL:
            rc = ProcessControlGroup(pOrigin, DAT, MSG, pData);
            break;
            
        case DG_IMAGE:
            rc = ProcessImageGroup(pOrigin, DAT, MSG, pData);
            break;
            
        default:
            rc = TWRC_FAILURE;
            break;
    }
    
    return rc;
}

/**
  * @brief  Control Group 처리
  */
TW_UINT16 ProcessControlGroup(pTW_IDENTITY pOrigin, TW_UINT16 DAT, 
                               TW_UINT16 MSG, TW_MEMREF pData)
{
    switch (DAT)
    {
        case DAT_IDENTITY:
            return ProcessIdentity(pOrigin, MSG, (pTW_IDENTITY)pData);
            
        case DAT_CAPABILITY:
            return ProcessCapability(pOrigin, MSG, (pTW_CAPABILITY)pData);
            
        case DAT_USERINTERFACE:
            return ProcessUserInterface(pOrigin, MSG, (pTW_USERINTERFACE)pData);
            
        case DAT_PENDINGXFERS:
            return ProcessPendingXfers(pOrigin, MSG, (pTW_PENDINGXFERS)pData);
            
        default:
            return TWRC_FAILURE;
    }
}

/**
  * @brief  Image Group 처리 (실제 스캔)
  */
TW_UINT16 ProcessImageGroup(pTW_IDENTITY pOrigin, TW_UINT16 DAT,
                             TW_UINT16 MSG, TW_MEMREF pData)
{
    switch (DAT)
    {
        case DAT_IMAGEINFO:
            return GetImageInfo((pTW_IMAGEINFO)pData);
            
        case DAT_IMAGENATIVEXFER:
            return TransferImageNative((TW_HANDLE*)pData);
            
        case DAT_IMAGEMEMXFER:
            return TransferImageMemory((pTW_IMAGEMEMXFER)pData);
            
        default:
            return TWRC_FAILURE;
    }
}

/**
  * @brief  Native 이미지 전송 (DIB 형식)
  */
TW_UINT16 TransferImageNative(TW_HANDLE *pHandle)
{
    // STM32에서 이미지 데이터 수신
    uint8_t *image_data;
    uint32_t image_size;
    
    if (!USB_ReceiveImage(&image_data, &image_size))
    {
        return TWRC_FAILURE;
    }
    
    // DIB (Device Independent Bitmap) 생성
    BITMAPINFOHEADER bih = {
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = g_ImageWidth,
        .biHeight = -g_ImageHeight,  // Top-down
        .biPlanes = 1,
        .biBitCount = 24,
        .biCompression = BI_RGB
    };
    
    uint32_t dib_size = sizeof(BITMAPINFOHEADER) + image_size;
    HGLOBAL hDib = GlobalAlloc(GHND, dib_size);
    
    if (hDib)
    {
        uint8_t *dib = (uint8_t *)GlobalLock(hDib);
        memcpy(dib, &bih, sizeof(BITMAPINFOHEADER));
        memcpy(dib + sizeof(BITMAPINFOHEADER), image_data, image_size);
        GlobalUnlock(hDib);
        
        *pHandle = hDib;
        return TWRC_XFERDONE;
    }
    
    return TWRC_FAILURE;
}
```

#### TWAIN 상태 머신

```
TWAIN 상태 (1-7):

┌─────┐ OpenDSM ┌─────┐ OpenDS ┌─────┐ EnableDS ┌─────┐
│  1  │────────▶│  2  │───────▶│  3  │─────────▶│  4  │
│     │         │     │        │     │          │     │
│Pre- │         │Source│       │Source│         │Source│
│Session│       │Manager│      │Open  │         │Enabled│
│     │         │Open  │        │     │          │     │
└─────┘         └─────┘        └─────┘          └──┬──┘
                                                   │
                                    Image Ready    │
                                                   ▼
┌─────┐ Reset  ┌─────┐ EndXfer ┌─────┐ Transfer ┌─────┐
│  7  │◀───────│  6  │◀────────│  5  │◀─────────│  4  │
│     │        │     │         │     │          │     │
│Xfer │        │Xfer │         │Xfer │          │     │
│Done │        │Ready │        │Active│         │     │
└─────┘        └─────┘         └─────┘          └─────┘
```

### TWAIN DS 배포

```
설치 위치:
Windows 32-bit: C:\Windows\twain_32\MyScanner\
Windows 64-bit: C:\Windows\twain_64\MyScanner\

필요 파일:
myScanner.ds     - TWAIN Data Source DLL
myScanner.ini    - 설정 파일 (선택)
```

---

## 🪟 Option 3: WIA (Windows Image Acquisition)

### WIA 아키텍처

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         WIA 아키텍처 (Windows 전용)                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ┌─────────────────┐                                                      │
│   │   Application   │  (Windows 팩스 및 스캔, Paint)                       │
│   └────────┬────────┘                                                      │
│            │ WIA API (IWiaDevMgr, IWiaItem)                                │
│            ▼                                                               │
│   ┌─────────────────┐                                                      │
│   │   WIA Service   │  (Windows 서비스)                                    │
│   │   (stisvc.dll)  │                                                      │
│   └────────┬────────┘                                                      │
│            │                                                               │
│            ▼                                                               │
│   ┌─────────────────┐                                                      │
│   │  WIA Minidriver │  ← 🔧 개발 필요! (WDK 사용)                         │
│   │     (DLL)       │     myScanner.dll                                    │
│   └────────┬────────┘                                                      │
│            │ USB I/O                                                       │
│            ▼                                                               │
│   ┌─────────────────┐                                                      │
│   │  STM32 Device   │                                                      │
│   └─────────────────┘                                                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### WIA Minidriver 개발 (개요)

WIA 드라이버 개발에는 **Windows Driver Kit (WDK)**가 필요합니다.

```c
/* WIA Minidriver 인터페이스 (개략) */

// 필수 구현 인터페이스
class CWIAMinidriverSample : public IWiaMiniDrv
{
public:
    // IUnknown
    HRESULT QueryInterface(REFIID riid, void **ppv);
    ULONG AddRef();
    ULONG Release();
    
    // IWiaMiniDrv
    HRESULT drvInitializeWia(BYTE *pWiasContext, ...);
    HRESULT drvAcquireItemData(BYTE *pWiasContext, ...);
    HRESULT drvGetCapabilities(BYTE *pWiasContext, ...);
    HRESULT drvGetDeviceErrorStr(LONG lFlags, ...);
    // ... 기타 메서드
};
```

### WIA vs TWAIN

| 항목 | TWAIN | WIA |
|------|-------|-----|
| 플랫폼 | Windows, Mac, Linux | **Windows 전용** |
| 개발 도구 | 일반 C/C++ | **WDK 필요** |
| 복잡도 | 중간 | 높음 |
| 앱 호환성 | Photoshop, GIMP 등 | Windows 앱 |
| 권장 | 범용 스캐너 | Windows 전용 |

---

## 📁 Option 4: USB MSC + 이미지 파일 (가장 간단)

### 동작 방식

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    MSC + 파일 방식 (가장 간단)                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   1. 스캔 버튼 누름                                                         │
│      │                                                                      │
│      ▼                                                                      │
│   2. STM32가 이미지 캡처                                                    │
│      │                                                                      │
│      ▼                                                                      │
│   3. JPEG/BMP로 저장 (SD카드 또는 내부 Flash)                               │
│      │    scan_001.jpg                                                      │
│      │    scan_002.jpg                                                      │
│      ▼                                                                      │
│   4. USB MSC로 PC에 연결                                                    │
│      │                                                                      │
│      ▼                                                                      │
│   5. PC 탐색기에서 파일 접근                                                │
│      │    📁 E:\scan_001.jpg                                                │
│      │    📁 E:\scan_002.jpg                                                │
│      ▼                                                                      │
│   6. 사용자가 파일 복사                                                     │
│                                                                             │
│   장점: 드라이버 불필요, 개발 매우 간단                                     │
│   단점: 실시간 전송 불가, 수동 파일 복사 필요                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 🎯 권장 솔루션

### 용도별 권장

| 용도 | 권장 방식 | 이유 |
|------|----------|------|
| **실시간 카메라** | UVC | 드라이버 불필요, 범용 |
| **문서 스캐너 (상용)** | TWAIN | 업계 표준, Photoshop 지원 |
| **Windows 전용 스캐너** | WIA | Windows 통합 |
| **단순 이미지 전송** | MSC + 파일 | 개발 가장 간단 |
| **프로토타입/학습** | UVC 또는 MSC | 드라이버 개발 불필요 |

### 개발 난이도 vs 기능

```
                    ┌─────────────────────────────────────────┐
                    │              기능/호환성                 │
                    │                 높음                     │
                    │                   │                      │
                    │         TWAIN ●   │   ● WIA             │
                    │                   │                      │
                    │                   │                      │
                    │             UVC ● │                      │
                    │                   │                      │
                    │                   │                      │
                    │                   │                      │
                    │     MSC+파일 ●    │                      │
                    │                   │                      │
                    │                 낮음                     │
                    └───────────────────┴─────────────────────┘
                          낮음       개발 난이도        높음
```

---

## 🔍 결론

### 스캐너/카메라 장비 개발 시 선택 가이드

```
Q: 드라이버 개발 없이 하고 싶다?
├── 실시간 영상 필요 → UVC (권장)
└── 파일 전송만 필요 → USB MSC + 이미지 파일

Q: 전문 스캔 소프트웨어 호환 필요?
├── Photoshop/GIMP → TWAIN
└── Windows 앱만 → WIA

Q: 프로토타입/학습 목적?
└── USB MSC + 파일 (가장 간단)
```

### 최종 권장

| 순위 | 방식 | 이유 |
|------|------|------|
| 1️⃣ | **UVC** | 드라이버 불필요, 실시간, 범용 |
| 2️⃣ | **MSC + 파일** | 가장 간단, 드라이버 불필요 |
| 3️⃣ | TWAIN | 전문 스캐너 (개발 복잡) |
| 4️⃣ | WIA | Windows 전용 (WDK 필요) |

---

## 📚 참고 자료

- [USB Video Class Specification](https://www.usb.org/document-library/video-class-v15-document-set)
- [TWAIN Specification](https://twain.org/specification/)
- [WIA Driver Development](https://docs.microsoft.com/en-us/windows-hardware/drivers/image/)
- [libUVC Documentation](https://github.com/libuvc/libuvc)

## 📝 라이선스

This document is licensed under the MIT License.

## ✍️ Author

Created for STM32 imaging device development.
