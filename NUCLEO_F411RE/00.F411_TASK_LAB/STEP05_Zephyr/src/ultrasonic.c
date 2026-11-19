#include "ultrasonic.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

#define SENSOR_NODE DT_NODELABEL(f411_sensors)

static const struct gpio_dt_spec trig = GPIO_DT_SPEC_GET(SENSOR_NODE, trigger_gpios);
static const struct gpio_dt_spec echo = GPIO_DT_SPEC_GET(SENSOR_NODE, echo_gpios);

static struct gpio_callback echo_cb;

#define US_SAMPLE_COUNT 5U
#define US_TRIGGER_PERIOD_MS 200U
#define US_PUBLISH_PERIOD_MS 1000U
#define US_ECHO_TIMEOUT_MS 35U
#define US_TRIG_PULSE_US 10U
#define US_MIN_WIDTH_US 150U
#define US_MAX_WIDTH_US 30000U
#define US_MIN_DISTANCE_MM 20U
#define US_MAX_DISTANCE_MM 4000U
#define US_REF_SOUND_X10 3430U

enum us_state {
	US_STATE_IDLE,
	US_STATE_WAIT_ECHO,
};

static enum us_state s_state;
static uint32_t s_last_trigger_ms;
static uint32_t s_last_publish_ms;
static uint32_t s_trigger_ms;
static volatile uint32_t s_echo_start_cycle;
static volatile uint32_t s_echo_width_us;
static volatile uint8_t s_echo_seen;
static volatile uint8_t s_echo_done;

static uint16_t s_samples_ref[US_SAMPLE_COUNT];
static uint16_t s_samples_corr[US_SAMPLE_COUNT];
static uint8_t s_sample_index;
static uint8_t s_sample_count;
static uint16_t s_sound_speed_x10 = US_REF_SOUND_X10;
static uint32_t s_errors;
static uint8_t s_has_result;

static Ultrasonic_Result s_result;

static void echo_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	uint32_t now;
	int level;

	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	level = gpio_pin_get_dt(&echo);
	if (level < 0) {
		return;
	}

	now = k_cycle_get_32();
	if (level != 0) {
		s_echo_start_cycle = now;
		s_echo_seen = 1U;
		return;
	}

	if (s_echo_seen != 0U) {
		s_echo_width_us = k_cyc_to_us_floor32(now - s_echo_start_cycle);
		s_echo_done = 1U;
		s_echo_seen = 0U;
	}
}

static void trigger_pulse(void)
{
	uint32_t start = k_cycle_get_32();

	gpio_pin_set_dt(&trig, 1);
	while (k_cyc_to_us_floor32(k_cycle_get_32() - start) < US_TRIG_PULSE_US) {
	}
	gpio_pin_set_dt(&trig, 0);
}

static uint16_t median_u16(uint16_t *values, uint8_t count)
{
	uint8_t i;
	uint8_t j;

	for (i = 1U; i < count; i++) {
		uint16_t key = values[i];

		j = i;
		while ((j > 0U) && (values[j - 1U] > key)) {
			values[j] = values[j - 1U];
			j--;
		}
		values[j] = key;
	}

	return values[count / 2U];
}

static void store_sample(uint32_t width_us)
{
	uint32_t ref = (width_us * US_REF_SOUND_X10) / 20000U;
	uint32_t corr = (width_us * s_sound_speed_x10) / 20000U;

	if ((corr < US_MIN_DISTANCE_MM) || (corr > US_MAX_DISTANCE_MM)) {
		s_errors++;
		return;
	}

	s_samples_ref[s_sample_index] = (uint16_t)ref;
	s_samples_corr[s_sample_index] = (uint16_t)corr;
	s_sample_index = (s_sample_index + 1U) % US_SAMPLE_COUNT;
	if (s_sample_count < US_SAMPLE_COUNT) {
		s_sample_count++;
	}
}

static void publish(uint32_t now)
{
	uint16_t ref[US_SAMPLE_COUNT];
	uint16_t corr[US_SAMPLE_COUNT];
	uint8_t i;

	if (s_sample_count == 0U) {
		s_result.timestamp_ms = now;
		s_result.distance_ref_mm = 0U;
		s_result.distance_mm = 0U;
		s_result.delta_mm = 0;
		s_result.valid = 0U;
		s_result.errors = s_errors;
		s_has_result = 1U;
		return;
	}

	for (i = 0U; i < s_sample_count; i++) {
		ref[i] = s_samples_ref[i];
		corr[i] = s_samples_corr[i];
	}

	s_result.timestamp_ms = now;
	s_result.distance_ref_mm = median_u16(ref, s_sample_count);
	s_result.distance_mm = median_u16(corr, s_sample_count);
	s_result.delta_mm = (int32_t)s_result.distance_mm - (int32_t)s_result.distance_ref_mm;
	s_result.valid = 1U;
	s_result.errors = s_errors;
	s_has_result = 1U;
}

int ultrasonic_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&trig) || !gpio_is_ready_dt(&echo)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&trig, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&echo, GPIO_INPUT);
	if (ret != 0) {
		return ret;
	}

	gpio_init_callback(&echo_cb, echo_isr, BIT(echo.pin));
	ret = gpio_add_callback(echo.port, &echo_cb);
	if (ret != 0) {
		return ret;
	}

	return gpio_pin_interrupt_configure_dt(&echo, GPIO_INT_EDGE_BOTH);
}

void ultrasonic_task(void)
{
	uint32_t now = k_uptime_get_32();

	if (s_state == US_STATE_IDLE) {
		if ((now - s_last_trigger_ms) >= US_TRIGGER_PERIOD_MS) {
			s_echo_done = 0U;
			s_echo_seen = 0U;
			s_echo_width_us = 0U;
			trigger_pulse();
			s_trigger_ms = now;
			s_last_trigger_ms = now;
			s_state = US_STATE_WAIT_ECHO;
		}
	} else {
		if (s_echo_done != 0U) {
			uint32_t width = s_echo_width_us;

			s_echo_done = 0U;
			s_state = US_STATE_IDLE;
			if ((width >= US_MIN_WIDTH_US) && (width <= US_MAX_WIDTH_US)) {
				store_sample(width);
			} else {
				s_errors++;
			}
		} else if ((now - s_trigger_ms) >= US_ECHO_TIMEOUT_MS) {
			s_errors++;
			s_echo_seen = 0U;
			s_state = US_STATE_IDLE;
		}
	}

	if ((now - s_last_publish_ms) >= US_PUBLISH_PERIOD_MS) {
		s_last_publish_ms = now;
		publish(now);
	}
}

int ultrasonic_get_result(Ultrasonic_Result *result)
{
	if ((result == NULL) || (s_has_result == 0U)) {
		return 0;
	}

	*result = s_result;
	s_has_result = 0U;
	return 1;
}

uint8_t ultrasonic_is_idle(void)
{
	return (s_state == US_STATE_IDLE) ? 1U : 0U;
}

void ultrasonic_set_sound_speed_x10(uint16_t speed_x10)
{
	if ((speed_x10 >= 2500U) && (speed_x10 <= 4000U)) {
		s_sound_speed_x10 = speed_x10;
	}
}
