
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "kb.h"
#include "sdk_uart.h"
#include "pca9538.h"
#include "oled.h"
#include "fonts.h"
#include "calc.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define LONG_PRESS_MS  800
/* USER CODE END PD */

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  oled_Init();
  Set_Keyboard();          /* настройка PCA9538 клавиатуры - один раз */
  calc_Init();
  calc_Draw();

  uint32_t eqStart  = 0;
  uint8_t  clearDone = 1;
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */
    uint8_t k = KB_GetEvent();
    if( k != KEY_NONE ) {
        /* отладка: индекс нажатой кнопки в терминал (для проверки раскладки) */
        char msg[] = "Key 00\n";
        msg[4] = (char)('0' + k / 10);
        msg[5] = (char)('0' + k % 10);
        UART_Transmit((uint8_t*)msg);

        calc_OnKey(k);
        calc_Draw();

        if( calc_IsEquals(k) ) {
            eqStart = HAL_GetTick();
            clearDone = 0;
        }
    }

    /* долгое нажатие "=" -> сброс */
    uint8_t held = KB_GetHeld();
    if( !clearDone ) {
        if( held == KEY_NONE || !calc_IsEquals(held) ) {
            clearDone = 1;                     /* кнопку отпустили раньше */
        } else if( HAL_GetTick() - eqStart >= LONG_PRESS_MS ) {
            calc_Clear();
            calc_Draw();
            clearDone = 1;
        }
    }

    HAL_Delay(10);
    /* USER CODE END 3 */
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
