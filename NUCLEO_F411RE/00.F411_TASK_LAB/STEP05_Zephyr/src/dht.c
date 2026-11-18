#include "dht.h"

#include <string.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

#define SENSOR_NODE DT_NODELABEL(f411_sensors)

static const struct gpio_dt_spec dht = GPIO_DT_SPEC_GET(SENSOR_NODE, dht_gpios);

#define DHT_READ_PERIOD_MS 2000U
#define DHT_START_LOW_US 20000U
#define DHT_RELEASE_US 30U
#define DHT_RESPONSE_TIMEOUT_US 200U
#define DHT_BIT_TIMEOUT_US 100U
#define DHT_ONE_THRESHOLD_US 50U

static uint32_t s_last_read_ms;
static uint32_t s_errors;
static uint8_t s_has_result;

static Dht_Result s_result;

static uint32_t wait_for_level(int level, uint32_t timeout_us)
{
	uint32_t start = k_cycle_get_32();

	while (gpio_pin_get_dt(&dht) != level) {
		if (k_cyc_to_us_floor32(k_cycle_get_32() - start) > timeout_us) {
			return 0U;
		}
	}

	return k_cyc_to_us_floor32(k_cycle_get_32() - start);
}

static uint32_t measure_high_time(void)
{
	uint32_t start = k_cycle_get_32();

	while (gpio_pin_get_dt(&dht) == 1) {
		if (k_cyc_to_us_floor32(k_cycle_get_32() - start) > DHT_BIT_TIMEOUT_US) {
			break;
		}
	}

	return k_cyc_to_us_floor32(k_cycle_get_32() - start);
}

static int read_bytes(uint8_t *data)
{
	uint8_t i;

	memset(data, 0, 5);

	gpio_pin_configure_dt(&dht, GPIO_OUTPUT);
	gpio_pin_set_dt(&dht, 1);
	k_busy_wait(1000U);
	gpio_pin_set_dt(&dht, 0);
	k_busy_wait(DHT_START_LOW_US);
	gpio_pin_set_dt(&dht, 1);
	k_busy_wait(DHT_RELEASE_US);
	gpio_pin_configure_dt(&dht, GPIO_INPUT);

	if ((wait_for_level(0, DHT_RESPONSE_TIMEOUT_US) == 0U) ||
	    (wait_for_level(1, DHT_RESPONSE_TIMEOUT_US) == 0U)) {
		return -EIO;
	}

	for (i = 0U; i < 40U; i++) {
		uint32_t high_us;

		if (wait_for_level(1, DHT_BIT_TIMEOUT_US) == 0U) {
			return -EIO;
		}

		high_us = measure_high_time();
		if (high_us > DHT_ONE_THRESHOLD_US) {
			data[i / 8U] |= (uint8_t)(1U << (7U - (i % 8U)));
		}
	}

	if (((data[0] + data[1] + data[2] + data[3]) & 0xFFU) != data[4]) {
		return -EIO;
	}

	return 0;
}

static int decode(const uint8_t *data, uint16_t *humidity_x10, int16_t *temp_x10)
{
	uint16_t humidity = ((uint16_t)data[0] << 8) | data[1];
	int16_t raw_temp = (int16_t)(((uint16_t)data[2] << 8) | data[3]);

	if ((humidity <= 1000U) && (raw_temp >= -400) && (raw_temp <= 800)) {
		*humidity_x10 = humidity;
		*temp_x10 = raw_temp;
		return 0;
	}

	humidity = ((uint16_t)data[0] * 10U) + data[1];
	raw_temp = (int16_t)(((uint16_t)data[2] * 10U) + data[3]);
	if ((humidity <= 1000U) && (raw_temp >= 0) && (raw_temp <= 800)) {
		*humidity_x10 = humidity;
		*temp_x10 = raw_temp;
		return 0;
	}

	return -EINVAL;
}

int dht_init(void)
{
	if (!gpio_is_ready_dt(&dht)) {
		return -ENODEV;
	}

	s_last_read_ms = k_uptime_get_32();
	return gpio_pin_configure_dt(&dht, GPIO_INPUT);
}

void dht_task(void)
{
	uint32_t now = k_uptime_get_32();
	uint8_t data[5];
	uint16_t humidity;
	int16_t temp;

	if ((s_last_read_ms != 0U) && ((now - s_last_read_ms) < DHT_READ_PERIOD_MS)) {
		return;
	}
	s_last_read_ms = now;

	s_result.timestamp_ms = now;
	s_result.valid = 0U;

	if (read_bytes(data) != 0) {
		s_errors++;
		s_result.errors = s_errors;
		s_has_result = 1U;
		return;
	}

	if (decode(data, &humidity, &temp) != 0) {
		s_errors++;
		s_result.errors = s_errors;
		s_has_result = 1U;
		return;
	}

	s_result.humidity_rh_x10 = humidity;
	s_result.temperature_c_x10 = temp;
	s_result.valid = 1U;
	s_result.errors = s_errors;
	s_has_result = 1U;
}

int dht_get_result(Dht_Result *result)
{
	if ((result == NULL) || (s_has_result == 0U)) {
		return 0;
	}

	*result = s_result;
	s_has_result = 0U;
	return 1;
}
