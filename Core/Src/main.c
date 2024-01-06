/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "utils.h"
#include "lcd12864serial.h"
#include <stdio.h>
//#include <regex.h>
#include "stdlib.h"
#include "string.h"
#include "drf1605h.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

extern uint8_t rxbuffer_uart5[512];
extern uint8_t RxBuf[512];
extern uint8_t MainBuf[MainBuf_SIZE];
extern uint8_t MainBuf_uart5[MainBuf_SIZE_uart5];
extern uint16_t oldPos_uart5;
extern uint16_t newPos_uart5;
extern bool uart_idle_data_prepared;
extern uint16_t newPos;
extern uint16_t oldPos;
extern u32 tim2_count;
extern bool is_uart5_idle;
extern DMA_HandleTypeDef hdma_usart2_rx;
bool should_read_buffer = false;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TOTAL_DEVICE 9
ZigbeeDevice devices[TOTAL_DEVICE];
u8 currentdevice_index = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_UART5_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  
  u8 i = 0;
  for(i=0; i<TOTAL_DEVICE; i++)
  {
    ZigbeeDevice d = {.id = i + 1,.is_online = false,.ask_time = 0,.last_response_time= 0 };
    devices[i] = d;
  }

  HAL_TIM_Base_Start(&htim1);
  //HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Start_IT(&htim2);
  Lcd_Init();
  //HAL_UART_Transmit(&huart2,"hello",5,0xFFFF);
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuf, RxBuf_SIZE);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  bool is_ok = check_connection_ok();
  if (is_ok)
  {
    //led_signal();
    LCD_Display_Words(0,0,(uint8_t*)"okok");
  }
  else {
    LCD_Display_Words(0,0,(uint8_t*)"sad");
    Error_Handler();
  }
  HAL_UARTEx_ReceiveToIdle_IT(&huart5,rxbuffer_uart5,255);
  uint8_t current_time[10];
  u32 last_tim2_count_to_refresh = tim2_count;
  u32 last_tim2_count_to_send_zigbee = tim2_count;
  while (1)
  {
    if (uart_idle_data_prepared)
    {
      uart_idle_data_prepared = false;
      int len = strlen((char *)(MainBuf + oldPos));
      HAL_UART_Transmit_IT(&huart2,(uint8_t *)(MainBuf + oldPos),len);
      Lcd_Init();
    }
    if (tim2_count - last_tim2_count_to_refresh > 20)
    {
      Lcd_Init();
      LCD_Display_Words(1,0,(uint8_t *)"ok");
      last_tim2_count_to_refresh = tim2_count;
    }
    if (is_uart5_idle)
    {
      is_uart5_idle = false;
      int len = strlen((char*)MainBuf_uart5);
      //rxbuffer_uart5[7] = 0;
      HAL_UART_Transmit(&huart2,(uint8_t *)(MainBuf_uart5),len,0xFFFF);
      //LCD_Display_Words(0,0,rxbuffer_uart5);
    }
    if (tim2_count - last_tim2_count_to_send_zigbee > 30)
    {
      int group_id = currentdevice_index + 1;
			char sent[50];
			RTC_TimeTypeDef sTime = {0};
			HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);
			sprintf((char *)sent,"[start-id:%dz]time:%02d:%02d:%02d[id:%dz-end]",group_id,sTime.Hours,sTime.Minutes,sTime.Seconds,group_id);
			transparent_send((uint8_t *)sent,strlen(sent));
			currentdevice_index++;
			if (currentdevice_index >= TOTAL_DEVICE)
			{
				currentdevice_index = 0;
				continue;
			}
			last_tim2_count_to_send_zigbee = tim2_count;
    }
    HAL_Delay(50);


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    HAL_Delay(200);
    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_8);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
