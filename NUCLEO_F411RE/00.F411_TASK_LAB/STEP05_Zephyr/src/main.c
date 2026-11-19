#include "dht.h"
#include "ultrasonic.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define LOG_QUEUE_LEN 8U
#define LOG_LINE_SIZE 96U

#define STACK_LED 512U
#define STACK_US 1024U
#define STACK_DHT 1024U
#define STACK_LOGGER 1024U

#define PRIO_DHT 2
#define PRIO_US 3
#define PRIO_LED 4
#define PRIO_LOGGER 5

#define LED_BLINK_PERIOD_MS 500U
#define US_TASK_PERIOD_MS 1U
#define DHT_TASK_PERIOD_MS 10U

typedef struct {
	char line[LOG_LINE_SIZE];
} LogMessage;

K_MSGQ_DEFINE(log_queue, sizeof(LogMessage), LOG_QUEUE_LEN, 4U);

K_THREAD_STACK_DEFINE(led_stack, STACK_LED);
K_THREAD_STACK_DEFINE(us_stack, STACK_US);
K_THREAD_STACK_DEFINE(dht_stack, STACK_DHT);
K_THREAD_STACK_DEFINE(logger_stack, STACK_LOGGER);

static struct k_thread led_thread;
static struct k_thread us_thread;
static struct k_thread dht_thread;
static struct k_thread logger_thread;

static void log_send(const char *fmt, ...)
{
	LogMessage msg;
	va_list args;

	va_start(args, fmt);
	vsnprintf(msg.line, sizeof(msg.line), fmt, args);
	va_end(args);

	(void)k_msgq_put(&log_queue, &msg, K_NO_WAIT);
}

static void led_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	for (;;) {
		gpio_pin_toggle_dt(&led);
		k_sleep(K_MSEC(LED_BLINK_PERIOD_MS));
	}
}

static void us_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	for (;;) {
		Ultrasonic_Result result;

		ultrasonic_task();
		if (ultrasonic_get_result(&result) != 0) {
			log_send("D,%u,%u,%u,%d,%u,%u\r\n",
				 result.timestamp_ms,
				 result.distance_ref_mm,
				 result.distance_mm,
				 result.delta_mm,
				 result.valid,
				 result.errors);
		}

		k_sleep(K_MSEC(US_TASK_PERIOD_MS));
	}
}

static void dht_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	for (;;) {
		Dht_Result env;
		int speed_x10;

		dht_task();
		if (dht_get_result(&env) == 0) {
			k_sleep(K_MSEC(DHT_TASK_PERIOD_MS));
			continue;
		}

		speed_x10 = 0;
		if (env.valid != 0U) {
			speed_x10 = 3313 + (606 * (int)env.temperature_c_x10) / 1000;
			ultrasonic_set_sound_speed_x10((uint16_t)speed_x10);
		}

		log_send("E,%u,%d,%u,%u,%u,%d\r\n",
			 env.timestamp_ms,
			 env.temperature_c_x10,
			 env.humidity_rh_x10,
			 env.valid,
			 env.errors,
			 speed_x10);

		k_sleep(K_MSEC(DHT_TASK_PERIOD_MS));
	}
}

static void logger_entry(void *arg1, void *arg2, void *arg3)
{
	LogMessage msg;

	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	for (;;) {
		if (k_msgq_get(&log_queue, &msg, K_FOREVER) == 0) {
			printk("%s", msg.line);
		}
	}
}

int main(void)
{
	printk("=== F411_TASK_LAB Step 5: Zephyr ===\r\n");
	printk("Zephyr RTOS: 4 threads, GPIO DT, message queue logger\r\n");
	printk("HC-SR04: TRIG=PA1, ECHO=PA0, 5 Hz sample, 1 Hz median\r\n");
	printk("DHT11/DHT22: DATA=PA4, 0.5 Hz, sound speed compensation\r\n");

	if (ultrasonic_init() != 0) {
		printk("ERROR: ultrasonic init failed\r\n");
		return 0;
	}

	if (dht_init() != 0) {
		printk("ERROR: DHT init failed\r\n");
		return 0;
	}

	if (!gpio_is_ready_dt(&led)) {
		printk("ERROR: LED GPIO not ready\r\n");
		return 0;
	}

	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) != 0) {
		printk("ERROR: LED GPIO config failed\r\n");
		return 0;
	}

	k_thread_create(&logger_thread, logger_stack, K_THREAD_STACK_SIZEOF(logger_stack),
			logger_entry, NULL, NULL, NULL, PRIO_LOGGER, 0, K_NO_WAIT);
	k_thread_create(&led_thread, led_stack, K_THREAD_STACK_SIZEOF(led_stack),
			led_entry, NULL, NULL, NULL, PRIO_LED, 0, K_NO_WAIT);
	k_thread_create(&us_thread, us_stack, K_THREAD_STACK_SIZEOF(us_stack),
			us_entry, NULL, NULL, NULL, PRIO_US, 0, K_NO_WAIT);
	k_thread_create(&dht_thread, dht_stack, K_THREAD_STACK_SIZEOF(dht_stack),
			dht_entry, NULL, NULL, NULL, PRIO_DHT, 0, K_NO_WAIT);

	k_thread_name_set(&logger_thread, "LOG");
	k_thread_name_set(&led_thread, "LED");
	k_thread_name_set(&us_thread, "US");
	k_thread_name_set(&dht_thread, "DHT");

	return 0;
}
