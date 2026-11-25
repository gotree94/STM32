#ifndef DHT_H
#define DHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  uint32_t timestamp_ms;
  uint32_t errors;
  int16_t temperature_c_x10;
  uint16_t humidity_rh_x10;
  uint8_t valid;
} Dht_Result;

void Dht_Init(void);
void Dht_Task(void);
uint8_t Dht_GetResult(Dht_Result *result);

#ifdef __cplusplus
}
#endif

#endif
