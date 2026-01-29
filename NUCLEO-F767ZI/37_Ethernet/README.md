# NUCLEO-F767ZI Ethernet TCP/IP with LwIP

STM32 NUCLEO-F767ZI 보드의 Ethernet을 이용한 TCP/IP 통신 예제입니다.

## 📋 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 보드 | NUCLEO-F767ZI |
| MCU | STM32F767ZIT6 (ARM Cortex-M7, 216MHz) |
| IDE | STM32CubeIDE |
| 기능 | LwIP 스택을 이용한 TCP Echo Server + USART3 디버그 출력 |

## 🔧 하드웨어 구성

### Ethernet PHY

NUCLEO-F767ZI는 **LAN8742A** PHY 칩이 내장되어 있습니다.

| 항목 | 값 |
|------|-----|
| PHY Chip | LAN8742A |
| Interface | RMII (Reduced MII) |
| PHY Address | 0 |
| Connector | RJ45 (보드 내장) |

### RMII 핀 매핑

| 기능 | GPIO | 설명 |
|------|------|------|
| ETH_REF_CLK | PA1 | 50MHz Reference Clock |
| ETH_MDIO | PA2 | Management Data I/O |
| ETH_MDC | PC1 | Management Data Clock |
| ETH_CRS_DV | PA7 | Carrier Sense / Data Valid |
| ETH_RXD0 | PC4 | Receive Data 0 |
| ETH_RXD1 | PC5 | Receive Data 1 |
| ETH_TX_EN | PG11 | Transmit Enable |
| ETH_TXD0 | PG13 | Transmit Data 0 |
| ETH_TXD1 | PB13 | Transmit Data 1 |

### LED 및 USART

| 기능 | GPIO |
|------|------|
| LD1 (Green) | PB0 |
| LD3 (Red) | PB14 |
| USART3_TX | PD8 |
| USART3_RX | PD9 |

## ⚙️ CubeMX 설정

### 1. RCC 설정

**Pinout & Configuration → System Core → RCC**

| 항목 | 설정값 |
|------|--------|
| HSE | **BYPASS Clock Source** |

**Clock Configuration:**

| 파라미터 | 값 |
|----------|-----|
| SYSCLK | 216 MHz |
| HCLK | 216 MHz |
| APB1 | 54 MHz |
| APB2 | 108 MHz |

### 2. SYS 설정

**Pinout & Configuration → System Core → SYS**

| 항목 | 설정값 |
|------|--------|
| Timebase Source | **TIM1** |

> ⚠️ **중요**: LwIP가 SysTick을 사용하므로, HAL Timebase를 다른 타이머로 변경해야 합니다!

### 3. ETH 설정

**Pinout & Configuration → Connectivity → ETH**

#### 3.1 Mode

| 항목 | 설정값 |
|------|--------|
| Mode | **RMII** |

#### 3.2 Parameter Settings - General : Ethernet Configuration

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| Ethernet MAC Address | 00:80:E1:00:00:00 | 기본값 사용 |
| Tx Descriptor Length | 4 | 송신 디스크립터 수 |
| First Tx Descriptor Address | 0x2007c0a0 | 자동 설정 |
| Rx Descriptor Length | 4 | 수신 디스크립터 수 |
| First Rx Descriptor Address | 0x2007c000 | 자동 설정 |
| Rx Buffers Length | 1524 | 수신 버퍼 크기 |
| **Rx Mode** | **Interrupt Mode** | ⚠️ **필수 변경!** |

> ⚠️ **중요**: **Rx Mode**는 기본값이 **Polling Mode**입니다. 반드시 **Interrupt Mode**로 변경하세요! Polling Mode는 CPU 점유율이 높고 패킷 손실이 발생할 수 있습니다.

#### Rx Mode 옵션 비교

| Rx Mode | 장점 | 단점 | 권장 |
|---------|------|------|------|
| Polling Mode | 구현 간단 | CPU 점유율 높음, 패킷 손실 가능 | ❌ |
| **Interrupt Mode** | 효율적, 안정적 | 인터럽트 설정 필요 | ✅ **권장** |

#### 3.3 NVIC Settings

**ETH 인터럽트 활성화 (Interrupt Mode 사용 시 필수)**

| 인터럽트 | Enable | Preemption Priority |
|----------|--------|---------------------|
| Ethernet global interrupt | ✅ | 0 (기본값) |

### 4. LWIP 설정

**Pinout & Configuration → Middleware → LWIP**

#### 4.1 Mode

| 항목 | 설정값 |
|------|--------|
| LWIP | ✅ **Checked** |

#### 4.2 DHCP vs 고정 IP 선택

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    DHCP vs 고정 IP 비교                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   DHCP (Dynamic Host Configuration Protocol)                               │
│   ─────────────────────────────────────────                                │
│   STM32 ──── "IP 주세요" ────▶ 공유기/DHCP 서버                            │
│         ◀── "192.168.1.105" ────                                           │
│                                                                             │
│   ✅ 장점                        ❌ 단점                                   │
│   • 자동 IP 할당                 • IP가 변경될 수 있음                     │
│   • 네트워크 설정 간편           • DHCP 서버 필요                          │
│   • IP 충돌 방지                 • 부팅 시 IP 할당 대기 시간               │
│                                  • 서버 접속 시 IP 찾기 어려움             │
│                                                                             │
│   용도: 테스트, 가정용, 유동적 환경                                        │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   고정 IP (Static IP) ⭐ 권장                                              │
│   ─────────────────────────────                                            │
│   STM32 ──── "나는 192.168.1.100 이야" ────▶ 네트워크                      │
│                                                                             │
│   ✅ 장점                        ❌ 단점                                   │
│   • IP 항상 동일                 • 수동 설정 필요                          │
│   • 즉시 연결 가능               • IP 충돌 주의                            │
│   • 서버/장비 접속 용이          • 네트워크 변경 시 재설정                 │
│   • DHCP 서버 불필요                                                       │
│                                                                             │
│   용도: 산업용, 서버, 고정 장비, 공장 설비 ⭐                              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 4.3 General Settings (고정 IP 설정 - 권장)

**IPv4 - DHCP Options:**

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| LWIP_DHCP (DHCP Module) | **Disabled** | 고정 IP 사용 |

**IPv4 - Static Address (DHCP Disabled 시 표시됨):**

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| IP_ADDRESS | 192.168.1.100 | 보드 IP |
| NETMASK_ADDRESS | 255.255.255.0 | 서브넷 마스크 |
| GATEWAY_ADDRESS | 192.168.1.1 | 게이트웨이 |

**Platform Settings:**

| 파라미터 | 값 |
|----------|-----|
| PHY Driver | LAN8742 |

**Protocols Options:**

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| LWIP_ICMP (ICMP Module) | Enabled | Ping 응답 |
| LWIP_IGMP (IGMP Module) | Disabled | 멀티캐스트 (불필요) |
| LWIP_DNS (DNS Module) | Disabled | DNS (불필요) |
| LWIP_UDP (UDP Module) | Enabled | UDP 프로토콜 |
| MEMP_NUM_UDP_PCB | 4 | UDP 연결 수 |
| LWIP_TCP (TCP Module) | Enabled | TCP 프로토콜 |
| MEMP_NUM_TCP_PCB | 5 | TCP 연결 수 |

#### 4.4 Key Options

**⚠️ CubeMX 기본값 vs 성능 최적화 설정 비교**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    Key Options 설정 비교                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   Option A: CubeMX 기본값 (메모리 절약형)                                   │
│   ─────────────────────────────────────────                                │
│   • MEM_SIZE:    1600 Bytes                                                │
│   • TCP_MSS:     536 Bytes                                                 │
│   • TCP_WND:     2144 Bytes                                                │
│   • TCP_SND_BUF: 1072 Bytes                                                │
│                                                                             │
│   ✅ 장점                        ❌ 단점                                   │
│   • RAM 사용량 최소              • TCP 성능 제한                           │
│   • 메모리 부족한 MCU에 적합     • 대용량 데이터 전송 시 느림              │
│                                  • 패킷 분할 증가                          │
│                                                                             │
│   용도: 메모리 제한 환경, 간단한 통신                                      │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   Option B: 성능 최적화 설정 ⭐ 권장                                        │
│   ─────────────────────────────────────                                    │
│   • MEM_SIZE:    10240 Bytes (10KB)                                        │
│   • TCP_MSS:     1460 Bytes (이더넷 MTU 최적화)                            │
│   • TCP_WND:     5840 Bytes (4 × TCP_MSS)                                  │
│   • TCP_SND_BUF: 5840 Bytes (4 × TCP_MSS)                                  │
│                                                                             │
│   ✅ 장점                        ❌ 단점                                   │
│   • 최대 TCP 성능               • RAM 사용량 증가 (~15KB)                  │
│   • 이더넷 MTU에 최적화         • 메모리 부족한 MCU에 부적합               │
│   • 대용량 전송 효율적                                                     │
│   • 패킷 분할 최소화                                                       │
│                                                                             │
│   용도: 산업용, 고성능 통신, STM32F7 (512KB RAM) ⭐                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

> 💡 **STM32F767은 512KB RAM**을 가지고 있으므로 **성능 최적화 설정을 권장**합니다.

**CubeMX에서 변경할 값 (Key Options 탭):**

**Infrastructure - Heap and Memory Pools Options:**

| 파라미터 | 기본값 | 권장값 | 설명 |
|----------|--------|--------|------|
| MEM_SIZE | 1600 | **10240** | Heap 메모리 크기 (10KB) |

**Infrastructure - Internal Memory Pool Sizes:**

| 파라미터 | 기본값 | 권장값 | 설명 |
|----------|--------|--------|------|
| MEMP_NUM_TCP_SEG | 16 | **24** | TCP 세그먼트 큐 수 |

> ⚠️ **주의**: TCP_SND_BUF를 늘리면 TCP_SND_QUEUELEN이 자동으로 증가합니다 (17).
> **MEMP_NUM_TCP_SEG는 반드시 TCP_SND_QUEUELEN 이상**이어야 합니다!
> 
> ```
> 규칙: MEMP_NUM_TCP_SEG >= TCP_SND_QUEUELEN
> 
> 에러 발생 시:
> "MEMP_NUM_TCP_SEG must be between 17 and 2,147,483,647"
> → MEMP_NUM_TCP_SEG를 17 이상으로 설정하세요.
> ```

**Callback - TCP Options:**

| 파라미터 | 기본값 | 권장값 | 설명 |
|----------|--------|--------|------|
| TCP_MSS | 536 | **1460** | 이더넷 MTU 최적화 |
| TCP_WND | 2144 | **5840** | TCP 수신 윈도우 (4×MSS) |
| TCP_SND_BUF | 1072 | **5840** | TCP 송신 버퍼 (4×MSS) |

**기타 Key Options (기본값 유지):**

| 파라미터 | 값 | 설명 |
|----------|-----|------|
| TCP_TTL | 255 | Time-To-Live |
| TCP_QUEUE_OOSEQ | Enabled | Out-of-order 패킷 큐잉 |
| LWIP_TCP_SACK_OUT | Disabled | Selective ACK |
| ETH_RX_BUFFER_CNT | 12 | 수신 버퍼 개수 |
| TCP_SND_QUEUELEN | 17 | 자동 계산됨 (변경 불필요) |

#### 4.5 Checksum Settings (하드웨어 체크섬)

| 파라미터 | 값 |
|----------|-----|
| CHECKSUM_GEN_IP | 0 |
| CHECKSUM_GEN_UDP | 0 |
| CHECKSUM_GEN_TCP | 0 |
| CHECKSUM_GEN_ICMP | 0 |
| CHECKSUM_CHECK_IP | 0 |
| CHECKSUM_CHECK_UDP | 0 |
| CHECKSUM_CHECK_TCP | 0 |

> 💡 STM32F7는 하드웨어 체크섬을 지원하므로 소프트웨어 체크섬을 비활성화합니다.

### 5. MPU 설정 (필수!)

**Pinout & Configuration → System Core → CORTEX_M7**

| 항목 | 설정값 |
|------|--------|
| MPU | ✅ **Enabled** |

#### MPU Region 0 (ETH DMA Descriptors)

| 파라미터 | 값 |
|----------|-----|
| MPU Region | Enabled |
| MPU Region Base Address | 0x30040000 |
| MPU Region Size | 256B |
| MPU SubRegion Disable | 0x0 |
| MPU TEX field level | 1 |
| MPU Access Permission | ALL ACCESS PERMITTED |
| MPU Instruction Access | DISABLE |
| MPU Shareability Permission | DISABLE |
| MPU Cacheable Permission | DISABLE |
| MPU Bufferable Permission | ENABLE |

> ⚠️ **중요**: STM32F7에서 Ethernet DMA가 정상 동작하려면 MPU 설정이 필수입니다!

### 6. USART3 설정

| 항목 | 설정값 |
|------|--------|
| Mode | Asynchronous |
| Baud Rate | 115200 |

### 7. GPIO 설정 (LED)

| 핀 | Mode | User Label |
|----|------|------------|
| PB0 | Output Push Pull | LD1 |
| PB14 | Output Push Pull | LD3 |

### 8. 코드 생성

**Ctrl+S** 또는 **Project → Generate Code**

## 💻 소스 코드

### main.c

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "lwip.h"
#include "tcp_echoserver.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
extern struct netif gnetif;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */

// printf 리다이렉션
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

/**
  * @brief  Link 상태 확인 및 출력
  */
void Print_IP_Info(void)
{
    printf("\r\n============================================\r\n");
    printf("  Network Configuration\r\n");
    printf("============================================\r\n");
    printf("  IP Address:  %s\r\n", ip4addr_ntoa(&gnetif.ip_addr));
    printf("  Netmask:     %s\r\n", ip4addr_ntoa(&gnetif.netmask));
    printf("  Gateway:     %s\r\n", ip4addr_ntoa(&gnetif.gw));
    printf("============================================\r\n\n");
}

/* USER CODE END 0 */

int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_LWIP_Init();

    /* USER CODE BEGIN 2 */
    printf("\r\n============================================\r\n");
    printf("  NUCLEO-F767ZI Ethernet TCP Echo Server\r\n");
    printf("  System Clock: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
    printf("============================================\r\n\n");

    printf("Waiting for Ethernet link...\r\n");

    // Link 연결 대기
    uint32_t link_timeout = HAL_GetTick();
    while (!netif_is_link_up(&gnetif))
    {
        MX_LWIP_Process();
        if (HAL_GetTick() - link_timeout > 10000)
        {
            printf("Link timeout! Check cable connection.\r\n");
            break;
        }
    }

    if (netif_is_link_up(&gnetif))
    {
        printf("Ethernet link is UP!\r\n");
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);  // Green LED ON

        Print_IP_Info();

        // TCP Echo Server 시작
        tcp_echoserver_init();
        printf("TCP Echo Server started on port 7\r\n\n");
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // LwIP 처리 (폴링 방식)
        MX_LWIP_Process();

        // Link 상태 표시
        static uint8_t prev_link_state = 0;
        uint8_t current_link_state = netif_is_link_up(&gnetif);

        if (current_link_state != prev_link_state)
        {
            prev_link_state = current_link_state;
            if (current_link_state)
            {
                printf("Link UP\r\n");
                HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
                HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
            }
            else
            {
                printf("Link DOWN\r\n");
                HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
            }
        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
```

### tcp_echoserver.h (Core/Inc에 생성)

```c
#ifndef __TCP_ECHOSERVER_H__
#define __TCP_ECHOSERVER_H__

#include "lwip/tcp.h"

#define ECHO_SERVER_PORT    7

void tcp_echoserver_init(void);

#endif /* __TCP_ECHOSERVER_H__ */
```

### tcp_echoserver.c (Core/Src에 생성)

```c
#include "tcp_echoserver.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

/* Echo Server 상태 구조체 */
struct echo_state
{
    struct tcp_pcb *pcb;
    struct pbuf *p;
};

/* 함수 프로토타입 */
static err_t echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void echo_error(void *arg, err_t err);
static err_t echo_poll(void *arg, struct tcp_pcb *tpcb);
static err_t echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void echo_send(struct tcp_pcb *tpcb, struct echo_state *es);
static void echo_close(struct tcp_pcb *tpcb, struct echo_state *es);

/**
  * @brief  TCP Echo Server 초기화
  */
void tcp_echoserver_init(void)
{
    struct tcp_pcb *pcb;
    err_t err;

    /* TCP PCB 생성 */
    pcb = tcp_new();

    if (pcb != NULL)
    {
        /* 포트 바인딩 */
        err = tcp_bind(pcb, IP_ADDR_ANY, ECHO_SERVER_PORT);

        if (err == ERR_OK)
        {
            /* Listen 모드로 전환 */
            pcb = tcp_listen(pcb);

            /* Accept 콜백 설정 */
            tcp_accept(pcb, echo_accept);

            printf("Echo server listening on port %d\r\n", ECHO_SERVER_PORT);
        }
        else
        {
            printf("Cannot bind port %d, error: %d\r\n", ECHO_SERVER_PORT, err);
            memp_free(MEMP_TCP_PCB, pcb);
        }
    }
}

/**
  * @brief  클라이언트 연결 Accept 콜백
  */
static err_t echo_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    struct echo_state *es;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);

    /* 우선순위 설정 */
    tcp_setprio(newpcb, TCP_PRIO_MIN);

    /* 상태 구조체 할당 */
    es = (struct echo_state *)mem_malloc(sizeof(struct echo_state));

    if (es != NULL)
    {
        es->pcb = newpcb;
        es->p = NULL;

        /* 콜백 함수 등록 */
        tcp_arg(newpcb, es);
        tcp_recv(newpcb, echo_recv);
        tcp_err(newpcb, echo_error);
        tcp_poll(newpcb, echo_poll, 1);
        tcp_sent(newpcb, echo_sent);

        printf("Client connected: %s:%d\r\n",
               ip4addr_ntoa(&newpcb->remote_ip),
               newpcb->remote_port);

        return ERR_OK;
    }
    else
    {
        echo_close(newpcb, es);
        return ERR_MEM;
    }
}

/**
  * @brief  데이터 수신 콜백
  */
static err_t echo_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct echo_state *es;

    LWIP_ASSERT("arg != NULL", arg != NULL);

    es = (struct echo_state *)arg;

    /* 연결 종료 확인 */
    if (p == NULL)
    {
        printf("Client disconnected\r\n");
        echo_close(tpcb, es);
        return ERR_OK;
    }

    /* 에러 확인 */
    if (err != ERR_OK)
    {
        if (p != NULL)
        {
            pbuf_free(p);
        }
        return err;
    }

    /* 수신 확인 */
    tcp_recved(tpcb, p->tot_len);

    /* 디버그 출력 */
    printf("Received %d bytes: ", p->tot_len);
    for (int i = 0; i < p->len && i < 32; i++)
    {
        char c = ((char *)p->payload)[i];
        if (c >= 32 && c < 127)
            printf("%c", c);
        else
            printf(".");
    }
    printf("\r\n");

    /* Echo 전송을 위해 pbuf 저장 */
    if (es->p == NULL)
    {
        es->p = p;
        echo_send(tpcb, es);
    }
    else
    {
        /* 기존 pbuf에 체인 */
        pbuf_chain(es->p, p);
    }

    return ERR_OK;
}

/**
  * @brief  데이터 전송
  */
static void echo_send(struct tcp_pcb *tpcb, struct echo_state *es)
{
    struct pbuf *ptr;
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) && (es->p != NULL) && (es->p->len <= tcp_sndbuf(tpcb)))
    {
        ptr = es->p;

        /* 데이터 전송 */
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, TCP_WRITE_FLAG_COPY);

        if (wr_err == ERR_OK)
        {
            es->p = ptr->next;

            if (es->p != NULL)
            {
                pbuf_ref(es->p);
            }

            pbuf_free(ptr);
        }
    }
}

/**
  * @brief  전송 완료 콜백
  */
static err_t echo_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct echo_state *es;

    LWIP_UNUSED_ARG(len);

    es = (struct echo_state *)arg;

    if (es->p != NULL)
    {
        echo_send(tpcb, es);
    }

    return ERR_OK;
}

/**
  * @brief  폴링 콜백
  */
static err_t echo_poll(void *arg, struct tcp_pcb *tpcb)
{
    struct echo_state *es;

    es = (struct echo_state *)arg;

    if (es != NULL)
    {
        if (es->p != NULL)
        {
            echo_send(tpcb, es);
        }
    }
    else
    {
        tcp_abort(tpcb);
        return ERR_ABRT;
    }

    return ERR_OK;
}

/**
  * @brief  에러 콜백
  */
static void echo_error(void *arg, err_t err)
{
    struct echo_state *es;

    LWIP_UNUSED_ARG(err);

    es = (struct echo_state *)arg;

    if (es != NULL)
    {
        printf("TCP Error: %d\r\n", err);
        mem_free(es);
    }
}

/**
  * @brief  연결 종료
  */
static void echo_close(struct tcp_pcb *tpcb, struct echo_state *es)
{
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);

    if (es != NULL)
    {
        if (es->p != NULL)
        {
            pbuf_free(es->p);
        }
        mem_free(es);
    }

    tcp_close(tpcb);
}
```

## 🔄 동작 방식

```
┌─────────────────────────────────────────────────────────────┐
│                     Ethernet Frame                           │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              LAN8742A PHY (RMII)                     │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              STM32F7 Ethernet MAC                    │    │
│  │                  (DMA Engine)                        │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                   LwIP Stack                         │    │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐             │    │
│  │  │   IP    │  │   TCP   │  │   UDP   │             │    │
│  │  └─────────┘  └─────────┘  └─────────┘             │    │
│  └─────────────────────────────────────────────────────┘    │
│                          │                                   │
│                          ▼                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              TCP Echo Server (Port 7)                │    │
│  │                                                      │    │
│  │   Client → Server: "Hello"                          │    │
│  │   Server → Client: "Hello" (Echo)                   │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## 🧪 테스트 방법

### 1. 하드웨어 연결

1. Ethernet 케이블로 NUCLEO 보드와 PC/Router 연결
2. USB 케이블로 ST-LINK 연결

### 2. PC 네트워크 설정 (직접 연결 시)

**고정 IP 설정:**

| 항목 | 값 |
|------|-----|
| IP Address | 192.168.1.10 |
| Netmask | 255.255.255.0 |
| Gateway | 192.168.1.1 |

### 3. Ping 테스트

```bash
# Linux/macOS
ping 192.168.1.100

# Windows
ping 192.168.1.100
```

**예상 결과:**
```
PING 192.168.1.100: 56 data bytes
64 bytes from 192.168.1.100: icmp_seq=0 ttl=255 time=0.5 ms
64 bytes from 192.168.1.100: icmp_seq=1 ttl=255 time=0.4 ms
```

### 4. TCP Echo 테스트

#### Netcat (nc) 사용

```bash
# Linux/macOS
nc 192.168.1.100 7

# 또는
echo "Hello STM32" | nc 192.168.1.100 7
```

#### Telnet 사용

```bash
telnet 192.168.1.100 7
```

#### Python 스크립트

```python
import socket

HOST = '192.168.1.100'
PORT = 7

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    
    message = b'Hello STM32 Ethernet!'
    s.sendall(message)
    print(f'Sent: {message.decode()}')
    
    data = s.recv(1024)
    print(f'Received: {data.decode()}')
```

## 📺 예상 출력 (USART3)

```
============================================
  NUCLEO-F767ZI Ethernet TCP Echo Server
  System Clock: 216 MHz
============================================

Waiting for Ethernet link...
Ethernet link is UP!

============================================
  Network Configuration
============================================
  IP Address:  192.168.1.100
  Netmask:     255.255.255.0
  Gateway:     192.168.1.1
============================================

Echo server listening on port 7
TCP Echo Server started on port 7

Client connected: 192.168.1.10:54321
Received 21 bytes: Hello STM32 Ethernet!
Client disconnected
```

## 🌐 DHCP 사용 시

### CubeMX 변경

**LWIP → General Settings:**

| 파라미터 | 값 |
|----------|-----|
| LWIP_DHCP | **Enabled** |

### 코드 변경 (main.c)

```c
/* USER CODE BEGIN Includes */
#include "lwip/dhcp.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 2 */
printf("Starting DHCP...\r\n");

// DHCP 시작
dhcp_start(&gnetif);

// IP 할당 대기
uint32_t dhcp_timeout = HAL_GetTick();
while (gnetif.ip_addr.addr == 0)
{
    MX_LWIP_Process();
    
    if (HAL_GetTick() - dhcp_timeout > 30000)
    {
        printf("DHCP timeout!\r\n");
        break;
    }
}

if (gnetif.ip_addr.addr != 0)
{
    printf("DHCP IP acquired!\r\n");
    Print_IP_Info();
}
/* USER CODE END 2 */
```

## 🔍 트러블슈팅

### Link가 올라오지 않는 경우

- [ ] Ethernet 케이블 연결 확인
- [ ] LAN8742A PHY Address가 0인지 확인
- [ ] RMII 핀 매핑 확인 (특히 ETH_REF_CLK = PA1)

### Ping이 안 되는 경우

- [ ] IP 주소 및 서브넷 설정 확인
- [ ] PC와 보드가 같은 네트워크에 있는지 확인
- [ ] 방화벽 설정 확인
- [ ] MPU 설정 확인 (필수!)

### DMA 에러가 발생하는 경우

- [ ] MPU Region 설정 확인
- [ ] ETH DMA Descriptor 주소 확인
- [ ] Cache 설정 확인

### TCP 연결이 안 되는 경우

- [ ] 포트 번호 확인 (기본 7)
- [ ] `tcp_echoserver_init()` 호출 확인
- [ ] LwIP 버퍼 크기 확인

### SysTick 관련 문제

- [ ] HAL Timebase가 TIM1으로 설정되었는지 확인
- [ ] `MX_LWIP_Process()` 주기적 호출 확인

## 📁 프로젝트 구조

```
06_Ethernet_TCP/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── tcp_echoserver.h          # Echo Server 헤더
│   │   ├── stm32f7xx_hal_conf.h
│   │   └── stm32f7xx_it.h
│   └── Src/
│       ├── main.c                     # 메인 로직
│       ├── tcp_echoserver.c           # Echo Server 구현
│       ├── stm32f7xx_hal_msp.c
│       ├── stm32f7xx_it.c
│       └── system_stm32f7xx.c
├── Drivers/
│   ├── CMSIS/
│   └── STM32F7xx_HAL_Driver/
├── LWIP/
│   ├── App/
│   │   └── lwip.c                     # LwIP 초기화
│   └── Target/
│       ├── ethernetif.c               # Ethernet Interface
│       └── lwipopts.h                 # LwIP 옵션
├── Middlewares/
│   └── Third_Party/
│       └── LwIP/                      # LwIP 스택
├── 06_Ethernet_TCP.ioc
└── README.md
```

## 📚 참고 자료

- [NUCLEO-F767ZI User Manual (UM1974)](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
- [STM32F767ZI Reference Manual (RM0410) - Ethernet](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [LAN8742A Datasheet](https://www.microchip.com/en-us/product/LAN8742A)
- [LwIP Documentation](https://www.nongnu.org/lwip/2_1_x/index.html)
- [AN3966: LwIP TCP/IP stack demonstration for STM32F4x7](https://www.st.com/resource/en/application_note/an3966-lwip-tcpip-stack-demonstration-for-stm32f4x7-microcontrollers-stmicroelectronics.pdf)

## 📝 라이선스

This project is licensed under the MIT License.

## ✍️ Author

Created for STM32 embedded systems learning and development.
