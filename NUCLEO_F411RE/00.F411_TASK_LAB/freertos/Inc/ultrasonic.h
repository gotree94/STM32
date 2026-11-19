#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef struct
{
  uint32_t timestamp_ms;
  uint32_t errors;
  uint16_t distance_ref_mm;
  uint16_t distance_mm;
  uint8_t valid;
} Ultrasonic_Result;

extern volatile uint16_t g_trace_echo_ticks;
extern volatile uint8_t g_trace_echo_edge;
extern volatile uint8_t g_trace_echo_state;
extern volatile uint32_t g_trace_edge_count;

void Ultrasonic_Init(void);
void Ultrasonic_Task(void);
uint8_t Ultrasonic_GetResult(Ultrasonic_Result *result);
uint16_t Ultrasonic_GetLastEchoTicks(void);
uint8_t Ultrasonic_IsIdle(void);
void Ultrasonic_SetSoundSpeedX10(uint16_t speed_x10);
void Ultrasonic_OnEchoEdge(void);

#ifdef __cplusplus
}
#endif

#endif
