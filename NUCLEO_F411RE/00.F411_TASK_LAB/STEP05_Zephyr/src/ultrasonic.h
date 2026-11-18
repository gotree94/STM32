#ifndef F411_ULTRASONIC_H_
#define F411_ULTRASONIC_H_

#include <stdint.h>

typedef struct {
	uint32_t timestamp_ms;
	uint32_t distance_ref_mm;
	uint32_t distance_mm;
	int32_t delta_mm;
	uint8_t valid;
	uint32_t errors;
} Ultrasonic_Result;

int ultrasonic_init(void);
void ultrasonic_task(void);
int ultrasonic_get_result(Ultrasonic_Result *result);
uint8_t ultrasonic_is_idle(void);
void ultrasonic_set_sound_speed_x10(uint16_t speed_x10);

#endif
