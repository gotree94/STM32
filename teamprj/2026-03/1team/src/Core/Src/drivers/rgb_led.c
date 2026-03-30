/* rgb_led.c - 주황색 수정 버전 */

#include "main.h"
#include "drivers/rgb_led.h"

void RGB_Init(void)
{
    RGB_Off();
}

void RGB_Set(int color)
{
    switch (color)
    {
    case RGB_COLOR_GREEN:   // 0 - 🟢 초록색

        HAL_GPIO_WritePin(GPIOC, GREEN_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, RED_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, BLUE_Pin, GPIO_PIN_RESET);
        break;

    case RGB_COLOR_RED:     // 1 - 🔴 빨강색

        HAL_GPIO_WritePin(GPIOC, GREEN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, RED_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, BLUE_Pin, GPIO_PIN_RESET);
        break;

    case RGB_COLOR_ORANGE:  // 2 - 🟠 주황색 (GREEN + RED)

        HAL_GPIO_WritePin(GPIOC, GREEN_Pin, GPIO_PIN_SET);   // GREEN ON
        HAL_GPIO_WritePin(GPIOC, RED_Pin, GPIO_PIN_SET);     // RED ON
        HAL_GPIO_WritePin(GPIOC, BLUE_Pin, GPIO_PIN_RESET);  // BLUE OFF
        break;

    default:
        RGB_Off();
        break;
    }
}

void RGB_Off(void)
{

    HAL_GPIO_WritePin(GPIOC, RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, BLUE_Pin, GPIO_PIN_RESET);
}
