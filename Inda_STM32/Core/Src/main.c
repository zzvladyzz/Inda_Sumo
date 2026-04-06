/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t adc_value[8];
uint16_t array[14]={};

volatile uint8_t RC5_trama=0u;
volatile uint8_t RC5_count=0u;
volatile bool 	RC5_state=false;
volatile uint16_t IR_38KHZ=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void motores(uint16_t MotorIzquierdo,uint16_t MotorDerecho);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_value, 8);
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,0);
__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,0);
__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0);
HAL_GPIO_WritePin(EN_MOTOR_GPIO_Port, EN_MOTOR_Pin, GPIO_PIN_RESET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  char buffer[30];
	  /*
	  		sprintf(buffer,"LL= %u",adc_value[3]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), 1);
	  		sprintf(buffer,"  LR=  %u",adc_value[2]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), 1);
	  		sprintf(buffer,"  RL= %u",adc_value[1]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), 1);
	  		sprintf(buffer," RR= %u \r\n",adc_value[0]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), 1);
	  		*/
	  		if(HAL_GPIO_ReadPin(Left_Line_GPIO_Port, Left_Line_Pin)==1){
	  			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 1);
	  		}
	  		else{
	  			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 0);
	  		}
	  		if(HAL_GPIO_ReadPin(Back_Line_GPIO_Port, Back_Line_Pin)==1){
	  			  			HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 1);
	  			  		}
	  			  		else{
	  			  			HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 0);
	  			  		}
	  		if(HAL_GPIO_ReadPin(Right_Line_GPIO_Port, Right_Line_Pin)==1){
	  			  			HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, 1);
	  			  		}
	  			  		else{
	  			  			HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, 0);
	  			  		}
	  		if(RC5_state==true)
	  		{
	  			RC5_state=false;
	  			uint16_t address=(RC5_trama>>6)&0x1F;
	  			uint16_t command=(RC5_trama)&0x3F;
	  			uint16_t start=(RC5_trama>>12)&0x03;
	  			uint16_t toggle=(RC5_trama>>11)&0x01;

	  			sprintf(buffer," start %u ",start);
	  			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
	  			sprintf(buffer," toogle %u ",toggle);
	  			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
	  			sprintf(buffer," address %u ",address);
	  			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	  			sprintf(buffer," command %u ",command);
	  			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);


	  			sprintf(buffer," \r\n ");
				HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
				HAL_Delay(500);
				HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	  		}

/*
	  		sprintf(buffer," Boton= %u ",adc_value[4]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	  		sprintf(buffer," A_ML= %u ",adc_value[5]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	  		sprintf(buffer," A_MR= %u ",adc_value[6]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	  		sprintf(buffer," voltage= %u \r\n",adc_value[7]);
	  		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	  		HAL_Delay(1000);*/

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {

    	uint8_t RC5_bit=(HAL_GPIO_ReadPin(IR_38KHZ_GPIO_Port, IR_38KHZ_Pin)==GPIO_PIN_SET)?0:1;
    	HAL_GPIO_TogglePin(SERVO_GPIO_Port, SERVO_Pin);
    	RC5_trama=(RC5_trama<<1)|RC5_bit;
    	RC5_count++;
    	__HAL_TIM_SET_AUTORELOAD(&htim2,1777);
    	    	if(RC5_count>=14)
    	    	{
    	    		HAL_TIM_Base_Stop_IT(&htim2);
    	    		RC5_state=true;
    	    		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);


    	    	}
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_8)
    {
        // 1. Bloqueamos el EXTI para que los cambios de bit no reinicien el Timer
            HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);

            RC5_trama = 1;
            RC5_count = 1;

            // 2. Sincronización: Esperamos 1333us para leer el primer bit en su zona estable
            __HAL_TIM_SET_AUTORELOAD(&htim2, 1332);
            __HAL_TIM_SET_COUNTER(&htim2, 0);
            HAL_TIM_Base_Start_IT(&htim2);

    }
}
void motores(uint16_t MotorIzquierdo,uint16_t MotorDerecho)
{
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,MotorDerecho);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,999);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,MotorIzquierdo);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,999);
}
/* USER CODE END 4 */

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
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
