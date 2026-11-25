#include "dht.h"
#include "main.h"

#define DHT_PORT GPIOA
#define DHT_PIN GPIO_PIN_4

#define DHT_READ_PERIOD_MS 2000U
#define DHT_START_LOW_US 20000U
#define DHT_RELEASE_US 30U
#define DHT_RESPONSE_TIMEOUT_US 200U
#define DHT_BIT_TIMEOUT_US 200U
#define DHT_ONE_THRESHOLD_US 50U

static uint32_t s_lastReadTick;
static uint32_t s_errorCount;
static Dht_Result s_result;
static volatile uint8_t s_resultReady;

static void DHT_SetPinOutput(void);
static void DHT_SetPinInput(void);
static uint32_t DHT_NowUs(void);
static void DHT_DelayUs(uint32_t us);
static uint8_t DHT_WaitLevel(GPIO_PinState level, uint32_t timeout_us, uint32_t *duration);
static uint8_t DHT_ReadRaw(uint8_t *data);
static uint8_t DHT_Decode(const uint8_t *data, int32_t *temperature_c_x10, uint32_t *humidity_rh_x10);

void Dht_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);

  s_lastReadTick = HAL_GetTick();
  s_errorCount = 0U;
  s_resultReady = 0U;
  s_result.timestamp_ms = 0U;
  s_result.errors = 0U;
  s_result.temperature_c_x10 = 0;
  s_result.humidity_rh_x10 = 0U;
  s_result.valid = 0U;
}

void Dht_Task(void)
{
  uint32_t now = HAL_GetTick();
  uint8_t data[5];
  int32_t temperature_c_x10;
  uint32_t humidity_rh_x10;
  uint8_t ok = 0U;

  if ((now - s_lastReadTick) < DHT_READ_PERIOD_MS)
  {
    return;
  }
  s_lastReadTick = now;

  DHT_SetPinOutput();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  DHT_DelayUs(DHT_START_LOW_US);
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  DHT_DelayUs(DHT_RELEASE_US);
  DHT_SetPinInput();

  if (DHT_ReadRaw(data) != 0U)
  {
    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);

    if (checksum == data[4])
    {
      if (DHT_Decode(data, &temperature_c_x10, &humidity_rh_x10) != 0U)
      {
        s_result.temperature_c_x10 = (int16_t)temperature_c_x10;
        s_result.humidity_rh_x10 = (uint16_t)humidity_rh_x10;
        s_result.valid = 1U;
        ok = 1U;
      }
    }
  }

  if (ok == 0U)
  {
    s_result.valid = 0U;
    s_errorCount++;
  }

  s_result.timestamp_ms = now;
  s_result.errors = s_errorCount;
  s_resultReady = 1U;
}

uint8_t Dht_GetResult(Dht_Result *result)
{
  if ((result == NULL) || (s_resultReady == 0U))
  {
    return 0U;
  }

  *result = s_result;
  s_resultReady = 0U;
  return 1U;
}

static void DHT_SetPinOutput(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_OD;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);
}

static void DHT_SetPinInput(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin = DHT_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &gpio);
}

static uint32_t DHT_NowUs(void)
{
  return TIM2->CNT;
}

static void DHT_DelayUs(uint32_t us)
{
  uint32_t start = DHT_NowUs();

  while ((DHT_NowUs() - start) < us)
  {
  }
}

static uint8_t DHT_WaitLevel(GPIO_PinState level, uint32_t timeout_us, uint32_t *duration)
{
  uint32_t start = DHT_NowUs();

  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) != level)
  {
    if ((DHT_NowUs() - start) >= timeout_us)
    {
      return 0U;
    }
  }

  if (duration != NULL)
  {
    *duration = DHT_NowUs() - start;
  }

  return 1U;
}

static uint8_t DHT_ReadRaw(uint8_t *data)
{
  uint32_t duration;
  uint8_t i;
  uint8_t bit;

  for (i = 0U; i < 5U; i++)
  {
    data[i] = 0U;
  }

  if (DHT_WaitLevel(GPIO_PIN_RESET, DHT_RESPONSE_TIMEOUT_US, NULL) == 0U)
  {
    return 0U;
  }
  if (DHT_WaitLevel(GPIO_PIN_SET, DHT_RESPONSE_TIMEOUT_US, NULL) == 0U)
  {
    return 0U;
  }

  for (i = 0U; i < 40U; i++)
  {
    if (DHT_WaitLevel(GPIO_PIN_RESET, DHT_BIT_TIMEOUT_US, NULL) == 0U)
    {
      return 0U;
    }
    if (DHT_WaitLevel(GPIO_PIN_SET, DHT_BIT_TIMEOUT_US, NULL) == 0U)
    {
      return 0U;
    }
    if (DHT_WaitLevel(GPIO_PIN_RESET, DHT_BIT_TIMEOUT_US, &duration) == 0U)
    {
      return 0U;
    }

    bit = (duration > DHT_ONE_THRESHOLD_US) ? 1U : 0U;
    data[i / 8U] = (uint8_t)((data[i / 8U] << 1U) | bit);
  }

  return 1U;
}

static uint8_t DHT_Decode(const uint8_t *data, int32_t *temperature_c_x10, uint32_t *humidity_rh_x10)
{
  uint32_t humidity;
  int32_t temperature;

  humidity = ((uint32_t)data[0] << 8) | data[1];
  temperature = (int32_t)(((uint16_t)(data[2] & 0x7FU) << 8) | data[3]);
  if ((data[2] & 0x80U) != 0U)
  {
    temperature = -temperature;
  }

  if ((humidity <= 1000U) && (temperature >= -400) && (temperature <= 800))
  {
    *humidity_rh_x10 = humidity;
    *temperature_c_x10 = temperature;
    return 1U;
  }

  if ((data[1] <= 9U) && (data[3] <= 9U))
  {
    humidity = ((uint32_t)data[0] * 10U) + data[1];
    temperature = ((int32_t)data[2] * 10) + (int32_t)data[3];

    if ((humidity <= 1000U) && (temperature >= 0) && (temperature <= 800))
    {
      *humidity_rh_x10 = humidity;
      *temperature_c_x10 = temperature;
      return 1U;
    }
  }

  return 0U;
}
