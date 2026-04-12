# P05 · GPS 트래커 + 실시간 지도 디스플레이

> STM32F103 + FreeRTOS 기반 GPS NMEA 파싱 + 경로 기록 + SPI TFT 맵 렌더링  
> **핵심 RTOS 개념 : `MessageBuffer` · `UART IDLE ISR` · `EventGroup` · `tickless idle`**

---

## 📌 프로젝트 개요

GPS 모듈에서 NMEA 문자열을 파싱하여 **위경도·속도·고도**를 실시간 추출합니다.  
ILI9341 / 2.4인치 TFT에 **도트 맵과 이동 경로**를 그리고,  
로터리 인코더로 **줌·패닝**을 제어합니다.  
경로 데이터는 FATFS로 **GPX 형식 저장**, 이동 통계는 EEPROM에 누적합니다.  
UART 파싱부터 그래픽 렌더링까지 **복합 시스템 설계 통합**을 학습합니다.

### 학습 목표

- **UART IDLE 인터럽트** 로 가변 길이 NMEA 문자열을 정확하게 수신한다
- **`xMessageBufferSend`** 로 가변 길이 메시지를 태스크 간 안전하게 전달한다
- **`xEventGroupSetBits`** 로 GPS Fix 획득 · 경로 저장 · 배터리 경고 다중 이벤트를 관리한다
- **tickless idle** 모드를 적용하여 GPS 대기 중 저전력 동작을 실험한다
- 위경도 좌표 → 픽셀 좌표 변환 및 **실시간 경로 렌더링** 알고리즘을 구현한다

---

## 🔧 하드웨어 구성

### 부품 목록 (BOM)

| 부품 | 모델 | 인터페이스 | 수량 |
|------|------|-----------|------|
| MCU 보드 | STM32F103C8T6 (Blue Pill) | - | 1 |
| GPS 모듈 | GY-GPS6MV2 (u-blox NEO-6M) | UART | 1 |
| TFT LCD | ILI9341 2.4인치 (240×320) | SPI | 1 |
| RTC 모듈 | DS1307 | I2C | 1 |
| EEPROM | AT24C256 (32KB) | I2C | 1 |
| microSD | NS-SD04 어댑터 + Class10 카드 | SPI / FATFS | 1 |
| 로터리 인코더 | EC11 (클릭 스위치 포함) | TIM Encoder | 1 |
| USB-UART | CH340 / CP2102 | UART | 1 |
| 배터리 (선택) | 18650 + TP4056 충전 모듈 | ADC (전압 감지) | 1 |

### 핀 배치

```
STM32F103C8T6
┌───────────────────────────────────────────────┐
│  PA2  ── USART2 TX → GPS Module (선택)        │
│  PA3  ── USART2 RX ← GPS Module (NMEA 수신)  │
│                                               │
│  PA9  ── USART1 TX → PC (디버그 출력)         │
│  PA10 ── USART1 RX ← PC                      │
│                                               │
│  PA6  ── Rotary Encoder A (TIM3_CH1)          │
│  PA7  ── Rotary Encoder B (TIM3_CH2)          │
│  PB0  ── Rotary Encoder SW (GPIO, EXTI0)      │
│                                               │
│  PB3  ── SPI1 SCK  (ILI9341 + SD 공유)       │
│  PB4  ── SPI1 MISO (ILI9341 + SD 공유)       │
│  PB5  ── SPI1 MOSI (ILI9341 + SD 공유)       │
│  PB6  ── ILI9341 CS                          │
│  PB7  ── ILI9341 DC                          │
│  PB8  ── ILI9341 RST                         │
│  PB9  ── SD Card CS                          │
│                                               │
│  PB10 ── I2C2 SCL (DS1307 + AT24C256)        │
│  PB11 ── I2C2 SDA (DS1307 + AT24C256)        │
│                                               │
│  PA0  ── Battery ADC (분압 저항, ADC1_CH0)   │
└───────────────────────────────────────────────┘
```

### GPS 모듈 연결

```
GY-GPS6MV2
  VCC  ── 3.3V
  GND  ── GND
  TX   ── PA3 (USART2 RX)   ← NMEA 데이터 수신
  RX   ── PA2 (USART2 TX)   → 설정 명령 송신 (선택)

기본 설정: 9600 bps, 8N1, 1Hz 업데이트
권장 설정: 9600 bps → 38400 bps 변경 (u-blox CFG 명령)
```

### 회로 연결 요약

```
SPI1 버스 ──┬── ILI9341 (CS: PB6)
            └── SD Card  (CS: PB9)

I2C2 버스 ──┬── DS1307  (0x68)
            └── AT24C256 (0x50)

배터리 전압 감지 (선택):
  18650 (4.2V max) ─── 10kΩ ─── PA0 ─── 10kΩ ─── GND
  → ADC 측정값 × 2 = 실제 전압 (3.3V 기준)
```

---

## 🗂️ 프로젝트 구조

```
P05_GPS_Tracker/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gps_parse_task.h     ← UART IDLE + NMEA 파서
│   │   ├── map_render_task.h    ← SPI TFT 경로 렌더링
│   │   ├── record_task.h        ← FATFS GPX 로깅
│   │   ├── ui_task.h            ← 로터리 인코더 줌/패닝
│   │   ├── stats_task.h         ← EEPROM 누적 통계
│   │   ├── nmea_parser.h        ← NMEA 문자열 파서 라이브러리
│   │   └── rtos_objects.h       ← 전역 RTOS 핸들
│   └── Src/
│       ├── main.c
│       ├── gps_parse_task.c
│       ├── map_render_task.c
│       ├── record_task.c
│       ├── ui_task.c
│       ├── stats_task.c
│       ├── nmea_parser.c
│       └── rtos_objects.c
├── Middlewares/
│   └── Third_Party/
│       └── FatFs/
├── Drivers/
│   ├── ILI9341/
│   └── AT24C256/
├── .ioc
└── README.md
```

---

## 🔄 FreeRTOS 태스크 설계

### 태스크 구성표

| 태스크 이름 | 우선순위 | 스택 (words) | 주기 / 트리거 | 역할 |
|------------|---------|-------------|-------------|------|
| `GPSParseTask` | 4 (최고) | 512 | UART IDLE ISR | NMEA 수신 + 파싱 |
| `MapRenderTask` | 3 | 512 | 500ms + 이벤트 | TFT 경로 드로잉 |
| `RecordTask` | 2 | 512 | 1Hz 이벤트 | FATFS GPX 로깅 |
| `UITask` | 3 | 256 | 인코더 이벤트 | 줌 / 패닝 제어 |
| `StatsTask` | 1 | 256 | 30s 주기 | EEPROM 누적 통계 |

### 태스크 간 통신 구조

```
USART2 IDLE ISR
    │
    │ xMessageBufferSendFromISR()
    ▼
┌──────────────────┐
│  GPSParseTask    │── NMEA 파싱 → GPSData_t 구조체
│  (Priority 4)    │
└────────┬─────────┘
         │
         │ xQueueOverwrite(xQueue_GPS)
         │ xEventGroupSetBits(xEvt_GPS)
         ▼
    ┌────┴──────────────────────────────────────┐
    │           xEvt_GPS EventGroup             │
    │  EVT_GPS_FIX    EVT_GPS_UPDATE            │
    │  EVT_LOG_SAVE   EVT_BAT_LOW               │
    └────┬──────────────┬────────────┬──────────┘
         │              │            │
         ▼              ▼            ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│ MapRenderTask│ │  RecordTask  │ │  StatsTask   │
│ (P3) TFT 맵  │ │ (P2) GPX 저장│ │ (P1) EEPROM  │
└──────────────┘ └──────────────┘ └──────────────┘

┌──────────────┐
│   UITask     │── 로터리 인코더 → 줌/패닝 파라미터
│  (P3) 인코더  │── xQueue → MapRenderTask
└──────────────┘
```

### RTOS 오브젝트 목록

```c
/* rtos_objects.h */

/* MessageBuffer - 가변 길이 NMEA 문자열 전달 */
extern MessageBufferHandle_t xMsgBuf_NMEA;   // ISR → GPSParseTask

/* Queue */
extern QueueHandle_t xQueue_GPS;     // GPSParseTask → 모든 소비 태스크
extern QueueHandle_t xQueue_MapCtrl; // UITask → MapRenderTask (줌/패닝)

/* EventGroup */
extern EventGroupHandle_t xEvt_GPS;

/* Mutex */
extern SemaphoreHandle_t xMutex_SPI;   // SPI1 (ILI9341 + SD)
extern SemaphoreHandle_t xMutex_I2C;   // I2C2 (DS1307 + EEPROM)

/* EventGroup 비트 정의 */
#define EVT_GPS_FIX      ( 1 << 0 )   // GPS Fix 획득
#define EVT_GPS_UPDATE   ( 1 << 1 )   // 새 GPS 데이터
#define EVT_LOG_SAVE     ( 1 << 2 )   // GPX 저장 요청
#define EVT_BAT_LOW      ( 1 << 3 )   // 배터리 부족 경고
#define EVT_ROUTE_CLEAR  ( 1 << 4 )   // 경로 초기화

/* GPS 데이터 구조체 */
typedef struct {
    double   latitude;        // 위도 (소수점 도)
    double   longitude;       // 경도 (소수점 도)
    float    altitude_m;      // 고도 (m)
    float    speed_kmh;       // 속도 (km/h)
    float    heading_deg;     // 방향 (0~360도)
    uint8_t  fix_quality;     // 0=없음, 1=GPS, 2=DGPS
    uint8_t  satellites;      // 수신 위성 수
    uint8_t  hour, min, sec;  // UTC 시각
    uint8_t  day, month;
    uint16_t year;
} GPSData_t;

/* 지도 제어 구조체 */
typedef struct {
    int8_t  zoom_delta;    // +1 줌인, -1 줌아웃
    int16_t pan_x;         // 픽셀 단위 패닝
    int16_t pan_y;
    uint8_t center_reset;  // 현재 위치로 센터 이동
} MapCtrl_t;

/* 경로 포인트 저장 (SRAM 링버퍼) */
#define MAX_ROUTE_POINTS   200

typedef struct {
    int16_t x;   // 화면 픽셀 좌표 (변환 후)
    int16_t y;
} RoutePoint_t;

extern RoutePoint_t  gRoutePoints[MAX_ROUTE_POINTS];
extern uint16_t      gRouteHead;
extern uint16_t      gRouteTail;
extern uint16_t      gRouteCount;
```

---

## 💻 핵심 코드

### 1. RTOS 오브젝트 생성 (`main.c`)

```c
void MX_FREERTOS_Init(void)
{
    /* MessageBuffer - NMEA 문자열 최대 128bytes, 버퍼 512bytes */
    xMsgBuf_NMEA = xMessageBufferCreate(512);

    /* Queue */
    xQueue_GPS     = xQueueCreate(4, sizeof(GPSData_t));
    xQueue_MapCtrl = xQueueCreate(8, sizeof(MapCtrl_t));

    /* EventGroup */
    xEvt_GPS = xEventGroupCreate();

    /* Mutex */
    xMutex_SPI = xSemaphoreCreateMutex();
    xMutex_I2C = xSemaphoreCreateMutex();

    /* 태스크 생성 */
    xTaskCreate(GPSParseTask,  "GPS",     512, NULL, 4, NULL);
    xTaskCreate(MapRenderTask, "Map",     512, NULL, 3, NULL);
    xTaskCreate(UITask,        "UI",      256, NULL, 3, NULL);
    xTaskCreate(RecordTask,    "Record",  512, NULL, 2, NULL);
    xTaskCreate(StatsTask,     "Stats",   256, NULL, 1, NULL);
}
```

### 2. GPSParseTask — UART IDLE ISR + MessageBuffer

```c
/*
 * UART IDLE 인터럽트 방식 선택 이유
 *
 * NMEA 문자열은 가변 길이 ($GPGGA 는 약 60~80 bytes)
 * 고정 DMA 수신으로는 문장 경계를 알 수 없음
 *
 * IDLE ISR 방식:
 *   DMA를 큰 버퍼(256 bytes)로 열어두고
 *   UART 라인이 1프레임(약 1ms) 이상 조용해지면 IDLE 인터럽트 발생
 *   → 완전한 NMEA 문장 1개 이상이 버퍼에 수신된 것을 보장
 */

#define UART_DMA_BUF_SIZE   256
static uint8_t uart_dma_buf[UART_DMA_BUF_SIZE];

/* UART2 초기화 - DMA + IDLE 인터럽트 활성화 */
void GPS_UART_Init(void)
{
    HAL_UART_Receive_DMA(&huart2, uart_dma_buf, UART_DMA_BUF_SIZE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

/* UART2 IRQ Handler - IDLE 감지 */
void USART2_IRQHandler(void)
{
    /* IDLE 인터럽트 확인 */
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);

        /* DMA가 수신한 바이트 수 계산 */
        uint16_t dma_remaining = __HAL_DMA_GET_COUNTER(huart2.hdmarx);
        uint16_t received_len  = UART_DMA_BUF_SIZE - dma_remaining;

        if (received_len > 0)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;

            /* MessageBuffer에 가변 길이 메시지 전송 (FromISR 버전) */
            xMessageBufferSendFromISR(
                xMsgBuf_NMEA,
                uart_dma_buf,
                received_len,
                &xHigherPriorityTaskWoken);

            /* DMA 버퍼 초기화 및 재시작 */
            HAL_UART_AbortReceive(&huart2);
            memset(uart_dma_buf, 0, UART_DMA_BUF_SIZE);
            HAL_UART_Receive_DMA(&huart2, uart_dma_buf, UART_DMA_BUF_SIZE);

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    HAL_UART_IRQHandler(&huart2);
}

void GPSParseTask(void *pvParameters)
{
    uint8_t  nmea_buf[256];
    size_t   received;
    GPSData_t gps;
    static GPSData_t prev_gps = {0};

    for (;;)
    {
        /* MessageBuffer에서 NMEA 문자열 수신 (블로킹) */
        received = xMessageBufferReceive(
                        xMsgBuf_NMEA,
                        nmea_buf,
                        sizeof(nmea_buf),
                        portMAX_DELAY);

        if (received > 0)
        {
            nmea_buf[received] = '\0';   // 문자열 종료

            /* NMEA 문장 파싱 */
            NMEA_ParseBuffer((char*)nmea_buf, received, &gps);

            if (gps.fix_quality > 0)
            {
                /* GPS Fix 획득 이벤트 */
                if (prev_gps.fix_quality == 0)
                    xEventGroupSetBits(xEvt_GPS, EVT_GPS_FIX);

                /* GPS 업데이트 이벤트 */
                xEventGroupSetBits(xEvt_GPS, EVT_GPS_UPDATE);

                /* Queue에 최신 GPS 데이터 전송 */
                xQueueOverwrite(xQueue_GPS, &gps);

                /* 이동 거리 계산 → 1초마다 GPX 저장 요청 */
                static uint32_t last_log_tick = 0;
                if ((xTaskGetTickCount() - last_log_tick) >= pdMS_TO_TICKS(1000))
                {
                    xEventGroupSetBits(xEvt_GPS, EVT_LOG_SAVE);
                    last_log_tick = xTaskGetTickCount();
                }

                prev_gps = gps;
            }

            /* 디버그 출력 */
            char dbg[64];
            snprintf(dbg, sizeof(dbg),
                     "[GPS] Lat:%.6f Lon:%.6f Spd:%.1f Fix:%d Sat:%d\r\n",
                     gps.latitude, gps.longitude,
                     gps.speed_kmh, gps.fix_quality, gps.satellites);
            HAL_UART_Transmit(&huart1, (uint8_t*)dbg, strlen(dbg), 50);
        }
    }
}
```

### 3. NMEA 파서 라이브러리 (`nmea_parser.c`)

```c
/*
 * 지원 NMEA 문장:
 *   $GPGGA - 위치, 고도, Fix 품질, 위성 수
 *   $GPRMC - 위치, 속도, 방향, 날짜/시각
 *   $GPVTG - 속도 (km/h, knot)
 */

/* NMEA 위도/경도 변환: DDMM.MMMM → 소수점 도 */
static double NMEA_CoordToDeg(const char *coord, char direction)
{
    double raw = atof(coord);
    int    deg  = (int)(raw / 100);
    double min  = raw - (deg * 100.0);
    double result = deg + min / 60.0;

    if (direction == 'S' || direction == 'W')
        result = -result;

    return result;
}

/* NMEA 버퍼에서 $GP 문장 추출 및 파싱 */
void NMEA_ParseBuffer(const char *buf, size_t len, GPSData_t *gps)
{
    char *ptr = (char *)buf;
    char *end = (char *)buf + len;
    char  sentence[128];

    while (ptr < end)
    {
        /* $ 문자 찾기 */
        char *start = memchr(ptr, '$', end - ptr);
        if (!start) break;

        /* \r\n 찾기 (문장 끝) */
        char *nl = memchr(start, '\n', end - start);
        if (!nl) break;

        size_t slen = nl - start;
        if (slen < 6 || slen > 127) { ptr = nl + 1; continue; }

        memcpy(sentence, start, slen);
        sentence[slen] = '\0';

        /* 체크섬 검증 */
        if (!NMEA_ValidateChecksum(sentence)) { ptr = nl + 1; continue; }

        /* 문장 종류별 파싱 */
        if (strncmp(sentence, "$GPGGA", 6) == 0 ||
            strncmp(sentence, "$GNGGA", 6) == 0)
        {
            NMEA_ParseGGA(sentence, gps);
        }
        else if (strncmp(sentence, "$GPRMC", 6) == 0 ||
                 strncmp(sentence, "$GNRMC", 6) == 0)
        {
            NMEA_ParseRMC(sentence, gps);
        }

        ptr = nl + 1;
    }
}

static void NMEA_ParseGGA(const char *sentence, GPSData_t *gps)
{
    /* $GPGGA,HHMMSS.ss,LLLL.LL,a,YYYYY.YY,a,x,xx,x.x,x.x,M,...*hh */
    char tokens[15][16];
    NMEA_Tokenize(sentence, tokens, 15);

    if (strlen(tokens[1]) >= 6) {
        gps->hour   = (tokens[1][0]-'0')*10 + (tokens[1][1]-'0');
        gps->min    = (tokens[1][2]-'0')*10 + (tokens[1][3]-'0');
        gps->sec    = (tokens[1][4]-'0')*10 + (tokens[1][5]-'0');
    }
    if (strlen(tokens[2]) > 0)
        gps->latitude  = NMEA_CoordToDeg(tokens[2], tokens[3][0]);
    if (strlen(tokens[4]) > 0)
        gps->longitude = NMEA_CoordToDeg(tokens[4], tokens[5][0]);

    gps->fix_quality = atoi(tokens[6]);
    gps->satellites  = atoi(tokens[7]);
    gps->altitude_m  = atof(tokens[9]);
}
```

### 4. MapRenderTask — 좌표 변환 + 경로 렌더링

```c
/*
 * 위경도 → 픽셀 좌표 변환 (Mercator 근사)
 *
 * 줌 레벨별 픽셀당 도 단위:
 *   zoom 1 : 0.01°/pixel  (~1.1km/pixel)
 *   zoom 2 : 0.005°/pixel (~550m/pixel)
 *   zoom 3 : 0.002°/pixel (~220m/pixel)
 *   zoom 4 : 0.001°/pixel (~110m/pixel)
 *   zoom 5 : 0.0005°/pixel(~55m/pixel)
 */

#define MAP_WIDTH    240
#define MAP_HEIGHT   280   // 상단 40픽셀 = 정보 표시줄

static double  gCenterLat = 0.0;
static double  gCenterLon = 0.0;
static uint8_t gZoomLevel = 3;     // 기본 줌 레벨
static int16_t gPanOffsetX = 0;
static int16_t gPanOffsetY = 0;

static const double ZoomScale[6] = {
    0.0, 0.01, 0.005, 0.002, 0.001, 0.0005
};

/* 위경도 → 화면 픽셀 좌표 변환 */
static void LatLonToPixel(double lat, double lon,
                           int16_t *px, int16_t *py)
{
    double scale = ZoomScale[gZoomLevel];
    *px = (int16_t)((lon - gCenterLon) / scale) + MAP_WIDTH  / 2 + gPanOffsetX;
    *py = (int16_t)((gCenterLat - lat) / scale) + MAP_HEIGHT / 2 + gPanOffsetY;
}

void MapRenderTask(void *pvParameters)
{
    GPSData_t   gps;
    MapCtrl_t   ctrl;
    EventBits_t bits;
    int16_t     px, py;
    int16_t     prev_px = -1, prev_py = -1;
    char        info_buf[48];
    TickType_t  xLastWakeTime = xTaskGetTickCount();

    ILI9341_Init();
    ILI9341_FillScreen(ILI9341_BLACK);
    DrawMapGrid();   // 격자 초기 표시

    for (;;)
    {
        /* EventGroup 대기 - GPS 업데이트 OR UI 제어 (최대 500ms) */
        bits = xEventGroupWaitBits(
                    xEvt_GPS,
                    EVT_GPS_UPDATE | EVT_ROUTE_CLEAR,
                    pdTRUE,    // 처리 후 비트 클리어
                    pdFALSE,   // OR 조건
                    pdMS_TO_TICKS(500));

        /* UI 줌/패닝 명령 처리 (비블로킹) */
        while (xQueueReceive(xQueue_MapCtrl, &ctrl, 0) == pdTRUE)
        {
            if (ctrl.zoom_delta != 0)
            {
                gZoomLevel = (uint8_t)__USAT(
                    (int)gZoomLevel + ctrl.zoom_delta, 3); // 1~5 클램프
                /* 줌 변경 → 전체 화면 재렌더링 */
                if (xSemaphoreTake(xMutex_SPI, pdMS_TO_TICKS(30)) == pdTRUE)
                {
                    ILI9341_FillRect(0, 40, MAP_WIDTH, MAP_HEIGHT, ILI9341_BLACK);
                    DrawMapGrid();
                    RedrawRouteHistory();   // 저장된 경로 전체 재표시
                    xSemaphoreGive(xMutex_SPI);
                }
            }
            gPanOffsetX += ctrl.pan_x;
            gPanOffsetY += ctrl.pan_y;

            if (ctrl.center_reset)
            {
                gPanOffsetX = 0;
                gPanOffsetY = 0;
                xQueuePeek(xQueue_GPS, &gps, 0);
                gCenterLat = gps.latitude;
                gCenterLon = gps.longitude;
            }
        }

        /* 경로 초기화 이벤트 */
        if (bits & EVT_ROUTE_CLEAR)
        {
            gRouteCount = 0;
            gRouteHead  = 0;
            gRouteTail  = 0;
            if (xSemaphoreTake(xMutex_SPI, pdMS_TO_TICKS(30)) == pdTRUE)
            {
                ILI9341_FillRect(0, 40, MAP_WIDTH, MAP_HEIGHT, ILI9341_BLACK);
                DrawMapGrid();
                xSemaphoreGive(xMutex_SPI);
            }
        }

        /* GPS 데이터 업데이트 */
        if ((bits & EVT_GPS_UPDATE) &&
            xQueuePeek(xQueue_GPS, &gps, 0) == pdTRUE)
        {
            /* 첫 Fix 획득 시 센터 설정 */
            if (gCenterLat == 0.0 && gps.fix_quality > 0)
            {
                gCenterLat = gps.latitude;
                gCenterLon = gps.longitude;
            }

            /* 현재 위치 → 픽셀 변환 */
            LatLonToPixel(gps.latitude, gps.longitude, &px, &py);

            if (xSemaphoreTake(xMutex_SPI, pdMS_TO_TICKS(30)) == pdTRUE)
            {
                /* 이전 점 → 현재 점 경로선 그리기 */
                if (prev_px >= 0 && prev_py >= 0 &&
                    px > 0 && px < MAP_WIDTH &&
                    py > 40 && py < MAP_HEIGHT + 40)
                {
                    ILI9341_DrawLine(prev_px, prev_py + 40,
                                     px, py + 40, ILI9341_CYAN);
                }

                /* 현재 위치 마커 (빨간 점) */
                ILI9341_FillCircle(px, py + 40, 4, ILI9341_RED);

                /* 상단 정보 표시줄 갱신 */
                ILI9341_FillRect(0, 0, 240, 38, ILI9341_NAVY);
                snprintf(info_buf, sizeof(info_buf),
                         "%.6f  %.6f", gps.latitude, gps.longitude);
                ILI9341_DrawString(2,  2, info_buf, FONT_12,
                                   ILI9341_WHITE, ILI9341_NAVY);
                snprintf(info_buf, sizeof(info_buf),
                         "Spd:%.1fkm/h  Alt:%.0fm  Sat:%d  Z:%d",
                         gps.speed_kmh, gps.altitude_m,
                         gps.satellites, gZoomLevel);
                ILI9341_DrawString(2, 20, info_buf, FONT_12,
                                   ILI9341_YELLOW, ILI9341_NAVY);

                xSemaphoreGive(xMutex_SPI);
            }

            /* 경로 링버퍼에 포인트 저장 */
            gRoutePoints[gRouteHead] = (RoutePoint_t){ px, py };
            gRouteHead = (gRouteHead + 1) % MAX_ROUTE_POINTS;
            if (gRouteCount < MAX_ROUTE_POINTS) gRouteCount++;

            prev_px = px;
            prev_py = py;
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}
```

### 5. RecordTask — FATFS GPX 형식 저장

```c
/*
 * GPX (GPS Exchange Format) 파일 구조
 * XML 기반, Google Earth / QGIS / OpenStreetMap 에서 읽기 가능
 *
 * <?xml version="1.0" encoding="UTF-8"?>
 * <gpx version="1.1">
 *   <trk><name>Track 20250601</name><trkseg>
 *     <trkpt lat="37.123456" lon="127.123456">
 *       <ele>45.2</ele>
 *       <time>2025-06-01T14:30:22Z</time>
 *       <speed>12.5</speed>
 *     </trkpt>
 *     ...
 *   </trkseg></trk>
 * </gpx>
 */

void RecordTask(void *pvParameters)
{
    GPSData_t gps;
    FIL       file;
    FRESULT   fr;
    char      filename[32];
    char      gpx_line[192];
    EventBits_t bits;
    uint32_t  point_count = 0;
    uint8_t   file_open   = 0;

    /* SD 마운트 */
    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"[REC] SD Mount Failed!\r\n", 23, 100);
        vTaskDelete(NULL);
        return;
    }

    for (;;)
    {
        /* EVT_LOG_SAVE 또는 EVT_GPS_FIX 대기 */
        bits = xEventGroupWaitBits(
                    xEvt_GPS,
                    EVT_LOG_SAVE | EVT_GPS_FIX,
                    pdTRUE,
                    pdFALSE,
                    portMAX_DELAY);

        if (!xQueuePeek(xQueue_GPS, &gps, 0)) continue;
        if (gps.fix_quality == 0) continue;

        /* GPS Fix 획득 시 파일 생성 */
        if ((bits & EVT_GPS_FIX) && !file_open)
        {
            snprintf(filename, sizeof(filename),
                     "TRK_%02d%02d%02d_%02d%02d.gpx",
                     gps.year % 100, gps.month, gps.day,
                     gps.hour, gps.min);

            if (xSemaphoreTake(xMutex_SPI, pdMS_TO_TICKS(1000)) == pdTRUE)
            {
                f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
                /* GPX 헤더 */
                f_puts("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n", &file);
                f_puts("<gpx version=\"1.1\" creator=\"STM32F103-GPS\">\r\n", &file);
                snprintf(gpx_line, sizeof(gpx_line),
                         "  <trk><name>Track %04d-%02d-%02d</name><trkseg>\r\n",
                         gps.year, gps.month, gps.day);
                f_puts(gpx_line, &file);
                f_sync(&file);
                xSemaphoreGive(xMutex_SPI);
                file_open = 1;

                char log[48];
                snprintf(log, sizeof(log), "[REC] File: %s\r\n", filename);
                HAL_UART_Transmit(&huart1, (uint8_t*)log, strlen(log), 50);
            }
        }

        /* 1초마다 트랙 포인트 기록 */
        if ((bits & EVT_LOG_SAVE) && file_open)
        {
            if (xSemaphoreTake(xMutex_SPI, pdMS_TO_TICKS(500)) == pdTRUE)
            {
                snprintf(gpx_line, sizeof(gpx_line),
                         "    <trkpt lat=\"%.6f\" lon=\"%.6f\">"
                         "<ele>%.1f</ele>"
                         "<time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>"
                         "<speed>%.2f</speed></trkpt>\r\n",
                         gps.latitude, gps.longitude,
                         gps.altitude_m,
                         gps.year, gps.month, gps.day,
                         gps.hour, gps.min, gps.sec,
                         gps.speed_kmh / 3.6f);   // km/h → m/s
                f_puts(gpx_line, &file);
                f_sync(&file);
                xSemaphoreGive(xMutex_SPI);
                point_count++;
            }
        }
    }
}
```

### 6. UITask — 로터리 인코더 줌/패닝

```c
/*
 * 로터리 인코더 TIM3 Encoder 모드
 *   - CW 회전  : TIM3 카운터 증가 → 줌인
 *   - CCW 회전 : TIM3 카운터 감소 → 줌아웃
 *   - 짧게 누름 : 현재 위치로 센터 이동
 *   - 길게 누름 : 경로 초기화
 */

#define ENCODER_LONG_PRESS_MS   1000

void UITask(void *pvParameters)
{
    MapCtrl_t  ctrl;
    int32_t    enc_prev  = 0;
    int32_t    enc_curr  = 0;
    uint32_t   sw_press_tick = 0;
    uint8_t    sw_pressed    = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* TIM3 Encoder 모드 시작 */
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(&htim3, 0x8000);   // 중간값 초기화

    for (;;)
    {
        enc_curr = (int32_t)__HAL_TIM_GET_COUNTER(&htim3) - 0x8000;
        int32_t enc_delta = enc_curr - enc_prev;

        /* 인코더 회전 처리 */
        if (enc_delta != 0)
        {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.zoom_delta = (enc_delta > 0) ? 1 : -1;
            xQueueSend(xQueue_MapCtrl, &ctrl, 0);
            enc_prev = enc_curr;
        }

        /* 스위치 누름 감지 */
        uint8_t sw_now = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);

        if (sw_now && !sw_pressed)
        {
            sw_press_tick = HAL_GetTick();
            sw_pressed    = 1;
        }
        else if (!sw_now && sw_pressed)
        {
            uint32_t held = HAL_GetTick() - sw_press_tick;
            memset(&ctrl, 0, sizeof(ctrl));

            if (held >= ENCODER_LONG_PRESS_MS)
            {
                /* 길게 누름 → 경로 초기화 */
                xEventGroupSetBits(xEvt_GPS, EVT_ROUTE_CLEAR);
                HAL_UART_Transmit(&huart1,
                    (uint8_t*)"[UI] Route cleared\r\n", 20, 50);
            }
            else
            {
                /* 짧게 누름 → 현재 위치로 센터 */
                ctrl.center_reset = 1;
                xQueueSend(xQueue_MapCtrl, &ctrl, 0);
            }
            sw_pressed = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));   // 50Hz 폴링
    }
}
```

### 7. StatsTask — EEPROM 누적 통계 + tickless idle

```c
/*
 * EEPROM 저장 통계 구조체 (AT24C256, 32KB)
 * 주소 0x0000 에 고정 저장 (매직 넘버로 유효성 확인)
 */
#define STATS_MAGIC   0xA55A3CC3

typedef struct {
    uint32_t magic;
    uint32_t total_sessions;      // 총 트래킹 세션 수
    float    total_distance_km;   // 총 이동 거리 (km)
    float    max_speed_kmh;       // 최고 속도
    float    max_altitude_m;      // 최고 고도
    uint32_t total_time_sec;      // 총 이동 시간
    uint32_t total_points;        // 기록된 포인트 수
    uint8_t  last_date[6];        // YYMMDDHHMMSS
} TrackStats_t;

void StatsTask(void *pvParameters)
{
    GPSData_t    gps;
    TrackStats_t stats;
    uint32_t     session_start_tick;
    float        prev_lat = 0.0f, prev_lon = 0.0f;
    char         buf[96];

    /* EEPROM에서 통계 로드 */
    if (xSemaphoreTake(xMutex_I2C, pdMS_TO_TICKS(500)) == pdTRUE)
    {
        EEPROM_Read(0x0000, (uint8_t*)&stats, sizeof(TrackStats_t));
        xSemaphoreGive(xMutex_I2C);
    }

    /* 매직 넘버 확인 - 첫 실행 시 초기화 */
    if (stats.magic != STATS_MAGIC)
    {
        memset(&stats, 0, sizeof(TrackStats_t));
        stats.magic = STATS_MAGIC;
    }

    stats.total_sessions++;
    session_start_tick = xTaskGetTickCount();

    for (;;)
    {
        /* GPS 데이터 수신 */
        if (xQueuePeek(xQueue_GPS, &gps, pdMS_TO_TICKS(100)) == pdTRUE &&
            gps.fix_quality > 0)
        {
            /* 최고 속도 / 최고 고도 갱신 */
            if (gps.speed_kmh > stats.max_speed_kmh)
                stats.max_speed_kmh = gps.speed_kmh;
            if (gps.altitude_m > stats.max_altitude_m)
                stats.max_altitude_m = gps.altitude_m;

            /* 이동 거리 누적 (Haversine 근사) */
            if (prev_lat != 0.0f)
            {
                float dlat = (gps.latitude  - prev_lat) * 111.0f;   // 위도 1도 ≈ 111km
                float dlon = (gps.longitude - prev_lon) * 111.0f
                             * cosf(gps.latitude * 3.14159f / 180.0f);
                stats.total_distance_km += sqrtf(dlat*dlat + dlon*dlon);
                stats.total_points++;
            }
            prev_lat = (float)gps.latitude;
            prev_lon = (float)gps.longitude;
        }

        /* 30초마다 EEPROM 저장 */
        stats.total_time_sec = (xTaskGetTickCount() - session_start_tick)
                                / configTICK_RATE_HZ;

        if (xSemaphoreTake(xMutex_I2C, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            EEPROM_Write(0x0000, (uint8_t*)&stats, sizeof(TrackStats_t));
            xSemaphoreGive(xMutex_I2C);
        }

        /* UART 통계 출력 */
        snprintf(buf, sizeof(buf),
                 "[STAT] Dist:%.2fkm  MaxSpd:%.1f  MaxAlt:%.0fm"
                 "  Pts:%lu  Time:%lus\r\n",
                 stats.total_distance_km, stats.max_speed_kmh,
                 stats.max_altitude_m, stats.total_points,
                 stats.total_time_sec);
        HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);

        /* 30초 대기 - tickless idle 적용 시 저전력 */
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
```

---

## 📊 시스템 동작 흐름

### 전체 시퀀스

```
전원 ON
  │
  ├─ GPS 모듈 초기화 (UART2 DMA + IDLE ISR 시작)
  ├─ SD 마운트, EEPROM 통계 로드
  ├─ ILI9341 초기화, 지도 격자 표시
  └─ FreeRTOS 스케줄러 시작
         │
         ├─ [GPS 신호 없음] GPSParseTask 대기 (MessageBuffer 블로킹)
         │   → tickless idle 진입 (저전력)
         │
         ├─ [GPS Fix 획득]
         │   GPSParseTask → EVT_GPS_FIX 설정
         │   RecordTask   → GPX 파일 생성
         │   MapRenderTask→ 센터 좌표 초기화 + 첫 마커 표시
         │
         └─ [이동 중 1Hz 업데이트]
             GPSParseTask → xQueue_GPS + EVT_GPS_UPDATE + EVT_LOG_SAVE
             MapRenderTask← EVT_GPS_UPDATE → 경로선 + 마커 업데이트
             RecordTask   ← EVT_LOG_SAVE  → GPX trkpt 추가
             StatsTask    ← 30s 주기      → EEPROM 통계 저장
```

### UART 디버그 출력 예시

```
[REC ] SD Mount OK
[STAT] Loaded: Dist:12.34km MaxSpd:45.2 Sessions:7
[GPS ] Lat:0.000000 Lon:0.000000 Spd:0.0 Fix:0 Sat:0   ← 수신 대기
[GPS ] Lat:37.123456 Lon:127.654321 Spd:0.0 Fix:1 Sat:4  ← Fix 획득!
[REC ] File: TRK_250601_1430.gpx
[GPS ] Lat:37.123512 Lon:127.654389 Spd:12.3 Fix:1 Sat:7
[STAT] Dist:0.05km  MaxSpd:12.3  MaxAlt:52m  Pts:5  Time:5s
```

---

## ⚠️ 주요 설계 고려사항

### MessageBuffer vs StreamBuffer 선택 기준

```
StreamBuffer  - 연속 바이트 스트림, 고정 크기 청크
  예) ADC 샘플(P03), 오디오 데이터

MessageBuffer - 가변 길이 메시지, 길이 헤더 자동 포함
  예) NMEA 문자열 (문장마다 60~100bytes로 가변)

NMEA 선택 근거:
  $GPGGA 문장 ≈ 72 bytes
  $GPRMC 문장 ≈ 68 bytes
  $GPGSV 문장 ≈ 40~80 bytes (위성 수에 따라 가변)
  → 메시지 경계 자동 보존이 필수 → MessageBuffer 선택
```

### UART IDLE 인터럽트 vs 고정 DMA

```
고정 크기 DMA:
  HAL_UART_Receive_DMA(&huart2, buf, 72);  // GPGGA만 처리 가능
  → 다른 NMEA 문장이 더 길면 잘림
  → 여러 문장이 연속 수신되면 경계 깨짐

UART IDLE 인터럽트:
  HAL_UART_Receive_DMA(&huart2, buf, 256);  // 큰 버퍼
  IDLE ISR: 수신 완료 감지 → MessageBuffer 전송
  → 가변 길이 문장 자동 처리
  → 여러 문장 동시 수신 후 파서에서 분리
```

### tickless idle 저전력 설정

```c
/* FreeRTOSConfig.h */
#define configUSE_TICKLESS_IDLE    1   // tickless idle 활성화

/* 동작 원리:
 * 모든 태스크가 xTaskDelay() 또는 xMessageBufferReceive()로 블로킹 상태
 * → FreeRTOS가 다음 깨어날 시각까지 남은 틱 계산
 * → SysTick 중단 + STM32 SLEEP 모드 진입
 * → 다음 이벤트(UART IDLE ISR, TIM 인터럽트 등) 발생 시 자동 복귀
 *
 * GPS 신호 대기 중 전류 소비:
 *   tickless 없음 : ~15mA (SysTick 1kHz 지속)
 *   tickless 적용 : ~4mA  (Sleep 모드)
 */
```

### 경도 거리 보정 (위도에 따른 cos 보정)

```c
/* 위도에 따라 경도 1도의 실제 거리가 달라짐 */
/* 서울(37°N): 경도 1도 ≈ 88.7km */
/* 적도(0°)  : 경도 1도 ≈ 111.3km */

/* 픽셀 변환 시 위도 보정 적용 */
double lon_scale = ZoomScale[gZoomLevel] /
                   cos(gCenterLat * M_PI / 180.0);
*px = (int16_t)((lon - gCenterLon) / lon_scale) + MAP_WIDTH / 2;
*py = (int16_t)((gCenterLat - lat) / ZoomScale[gZoomLevel]) + MAP_HEIGHT / 2;
```

---

## 🧪 실습 과제

### 기본 과제

1. `xMessageBufferCreate` 대신 `xStreamBufferCreate` 를 사용하고 NMEA 문장 경계가 깨지는 현상을 확인한다
2. `UART IDLE ISR` 방식 대신 `HAL_UART_Receive_IT` (1바이트씩 수신)로 교체하고 CPU 점유율 차이를 비교한다
3. 로터리 인코더 줌 레벨을 5단계에서 8단계로 확장하고 각 줌의 `ZoomScale` 값을 계산한다

### 심화 과제

4. `configUSE_TICKLESS_IDLE = 1` 로 설정하고 GPS 대기 중 전류를 측정하여 tickless 효과를 수치로 확인한다
5. 경로 포인트를 SRAM 링버퍼(200개)가 아닌 **EEPROM에 직접 저장**하여 전원 재투입 후 이전 경로를 복원하는 기능을 구현한다
6. `vTaskList()` 와 `vTaskGetRunTimeStats()` 를 활성화하여 각 태스크의 CPU 점유율을 측정하고 병목 태스크를 찾아 최적화한다

---

## 🔍 트러블슈팅

| 증상 | 원인 | 해결 방법 |
|------|------|----------|
| GPS 데이터 전혀 없음 | UART2 보드레이트 불일치 | GPS 모듈 기본 9600bps 확인 |
| NMEA 문자열 잘림 | IDLE ISR DMA 버퍼 부족 | `UART_DMA_BUF_SIZE` 256 → 512 증가 |
| 지도 경로선 이상 | 위도 cos 보정 미적용 | `LatLonToPixel` 함수 보정 코드 추가 |
| SD 쓰기 중 LCD 멈춤 | SPI Mutex 대기 타임아웃 | `xMutex_SPI` 대기 시간 500ms로 증가 |
| EEPROM 데이터 깨짐 | I2C Mutex 미적용 | `xMutex_I2C` 적용 확인 |
| 줌 후 경로 사라짐 | 재렌더링 누락 | `RedrawRouteHistory()` 호출 확인 |
| tickless 적용 후 UART 출력 깨짐 | SysTick 중단 타이밍 | `HAL_UART_Transmit` 타임아웃 증가 |

---

## 📚 참고 자료

- [NMEA 0183 Protocol Reference](https://www.nmea.org/content/STANDARDS/NMEA_0183_Standard)
- [u-blox NEO-6M 데이터시트](https://www.u-blox.com/sites/default/files/products/documents/NEO-6_DataSheet_(GPS.G6-HW-09005).pdf)
- [FreeRTOS MessageBuffer API](https://www.freertos.org/RTOS-message-buffer-API.html)
- [FreeRTOS tickless idle](https://www.freertos.org/low-power-tickless-rtos.html)
- [Haversine 공식 (거리 계산)](https://www.movable-type.co.uk/scripts/latlong.html)
- [GPX 파일 형식 사양](https://www.topografix.com/gpx.asp)

---

*P05 · GPS 트래커 + 실시간 지도 디스플레이 | 광주인력개발원 임베디드 시스템 과정*
