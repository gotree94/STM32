#ifndef F411_DHT_H_
#define F411_DHT_H_

#include <stdint.h>

typedef struct {
	uint32_t timestamp_ms;
	uint32_t errors;
	int16_t temperature_c_x10;
	uint16_t humidity_rh_x10;
	uint8_t valid;
} Dht_Result;

int dht_init(void);
void dht_task(void);
int dht_get_result(Dht_Result *result);

#endif
