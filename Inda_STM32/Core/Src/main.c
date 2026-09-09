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


typedef enum{
	adelante,
	atras
}Direccion;
typedef struct{
	int16_t PWM_Left;
	int16_t PWM_Right;
	bool enable;
	Direccion direccion;
}Motores;

typedef struct{
	uint16_t start;
	uint16_t toggle;
	uint16_t address;
	uint16_t command;
	volatile uint16_t trama;
	volatile uint8_t count;
	volatile bool 	state;
	volatile bool   IntQ;
	volatile uint16_t IR;
}RC5s;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define convVolt 0.000825396  //3.38/4095
#define divisorRes  0.259259		//r1=100k r2 35k

#define LL_sharp sharp[3]
#define LR_sharp sharp[2]
#define RL_sharp sharp[1]
#define RR_sharp sharp[0]
#define distanciaMaxima  33.0  // o 16000
#define distanciaMinima  15.0  // o 16000



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
Motores motorZumo;
RC5s	rc5={0};


uint16_t adc_value[8];
uint16_t array[15]={};


uint8_t standby=0;
bool combate=false;

float voltaje=0;
float corrienteML=0;
float corrienteMR=0;
float sharp[4]={};


bool pulsoConstante=false;
int8_t seleccionEstrategia=0;


uint32_t tiempo=0u;
uint32_t tiempoAnterior=0u;
uint32_t tiempoAnteriorVoltaje=0u;
uint32_t tiempoMenu=0u;
uint32_t tiempoGiro=0u;



bool leftLine=false;
bool rightLine=false;
bool buscar=false;
uint8_t accion=0;
char buffer[30];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void motores(Motores *motor);
void detener();
void printADC();
void printADC_IR();
void printADC_Volt_Amp();;
void printRC5();
void RC5_recepcion();
void conversionADC();
void filtroSharp();
void estrategia2();
void estrategia1();
void estrategia0();

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

motorZumo.PWM_Left=0;
motorZumo.PWM_Right=0;
motorZumo.enable=0;
motorZumo.direccion=adelante;
motores(&motorZumo);

HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 1);
HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 1);

HAL_Delay(1000);
HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, 0);
HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, 0);


__HAL_TIM_SET_AUTORELOAD(&htim2, 1332);
__HAL_TIM_SET_COUNTER(&htim2, 0);
__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  	  RC5_recepcion();	//Aca se reciben datos y se obtiene address y command

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
    	static bool Frecuencia=true;
    	if(Frecuencia)
    	{
    		__HAL_TIM_SET_AUTORELOAD(&htim2,1777);
    		Frecuencia=false;
    		HAL_GPIO_WritePin(SERVO_GPIO_Port, SERVO_Pin, 0);
    	}
    	uint8_t RC5_bit=(HAL_GPIO_ReadPin(IR_38KHZ_GPIO_Port, IR_38KHZ_Pin)==GPIO_PIN_SET)?1:0;
    	rc5.trama=(rc5.trama<<1)|RC5_bit;
    	    	if(rc5.count++>11)
    	    	{
    	    		HAL_TIM_Base_Stop_IT(&htim2);
    	    		rc5.count=0;
    	    		rc5.state=true;
    	    		rc5.IntQ=false;
    	    		Frecuencia=true;
    	            __HAL_TIM_SET_AUTORELOAD(&htim2, 1332);
    	            __HAL_TIM_SET_COUNTER(&htim2, 0);
    	    		//__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
    	    	}


    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == IR_38KHZ_Pin)
    {
    	HAL_GPIO_WritePin(SERVO_GPIO_Port, SERVO_Pin, 1);

        // 1. Bloqueamos el EXTI para que los cambios de bit no reinicien el Timer
		if(!rc5.IntQ){
    		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
            HAL_NVIC_DisableIRQ(EXTI4_IRQn);
            HAL_NVIC_DisableIRQ(EXTI3_IRQn);


            rc5.trama = 1;
            rc5.count = 0;
            rc5.IntQ=true;
            rc5.state=false;


            // 2. Sincronización: Esperamos 1333us para leer el primer bit en su zona estable
            //__HAL_TIM_SET_AUTORELOAD(&htim2, 1332);
           // __HAL_TIM_SET_COUNTER(&htim2, 0);
            HAL_TIM_Base_Start_IT(&htim2);
		}

    }
    else if (GPIO_Pin == Left_Line_Pin) {
    	}
    else if (GPIO_Pin == Right_Line_Pin) {
    }
    else if (GPIO_Pin == Back_Line_Pin) {
    }

}
/*
 * Codigo importante para la recepcion
 * se recibe 2 bit mas para evitar una segunda pulsacion rapida
 *
 */
void RC5_recepcion()
{
	if(rc5.state)
	{
		uint16_t datos=0;
		for(uint8_t c=0;c<=15;c++)
		{
			datos=(rc5.trama>>c)&0x01;
		sprintf(buffer,"%u ",datos);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		}
		sprintf(buffer," \r\n ");
			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		// Se desfasa 2 posiciones mas para evitar leer en rebote de ir
			rc5.start=(rc5.trama>>14)&0x03;
		if(rc5.start==1){
			rc5.toggle=(rc5.trama>>13)&0x01;
			rc5.address=(rc5.trama>>8)&0x1F;
			rc5.command=(rc5.trama>>2)&0x3F;
			//printRC5();
		}
		rc5.trama=0;
		rc5.state=false;

		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		HAL_NVIC_EnableIRQ(EXTI4_IRQn);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	}
	// Se para todo y se deberia inicializar los valores a 0
	if(rc5.address==2 && rc5.command==0x08)
	{
		detener();
	}
	// comando para entrar en standby y iniciar robot

	if(rc5.address==1 && rc5.command==4)
	{
		standby++;
		if(standby==1)
		{
			HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_SET);

		}
		else if(standby ==2)
		{
			combate=false;

			HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_RESET);
			standby=1 ;
		}
		rc5.start=0;
		rc5.address=0;
		rc5.command=0;
	}
}


void detener()
{
	HAL_GPIO_TogglePin(LED_ALARMA_GPIO_Port, LED_ALARMA_Pin);
	motorZumo.PWM_Left=0;
	motorZumo.PWM_Right=0;
	motorZumo.enable=false;
	motorZumo.direccion=adelante;
	motores(&motorZumo);
	rc5.start=0;
	rc5.address=0;
	rc5.command=0;
	standby=0;
	combate=false;
	leftLine=false;
	rightLine=false;
	tiempoGiro=0;
	accion=0;
	buscar=false;
}
/*
 * funciona para activar motores con inversion pero si se cambia de motores
 * corregir los tim_channel_x y hacer pruebas
 */
void motores(Motores *motor)
{

	int16_t right=0;
	if((motor->PWM_Right)>=0)
	{
		motor->PWM_Right=((motor->PWM_Right)>999)?999:motor->PWM_Right;

		right=999-(motor->PWM_Right);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,999);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,right);
	}
	else if((motor->PWM_Right)<0)
	{
		motor->PWM_Right=-1*(motor->PWM_Right);
		motor->PWM_Right=((motor->PWM_Right)>999)?999:motor->PWM_Right;

		right=999-(motor->PWM_Right);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,right);
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,999);
	}

	int16_t left=0;
	if((motor->PWM_Left)>=0)
		{
		motor->PWM_Left=((motor->PWM_Left)>999)?999:motor->PWM_Left;
			left=999-(motor->PWM_Left);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,999);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,left);
		}
		else if((motor->PWM_Left)<0)
		{
			motor->PWM_Left=-(motor->PWM_Left);
			motor->PWM_Left=((motor->PWM_Left)>999)?999:motor->PWM_Left;

			left=999-(motor->PWM_Left);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_3,left);
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,999);
		}

	HAL_GPIO_WritePin(EN_MOTOR_GPIO_Port, EN_MOTOR_Pin, motor->enable);


}

void conversionADC()
{
	voltaje=adc_value[7]*convVolt;
	voltaje=voltaje/divisorRes;

	// corriente I=(V-1.65)/0.66
/*
	corrienteML=adc_value[5]*convVolt;
	corrienteML=corrienteML-1.65;
	corrienteML=corrienteML/0.66;

	corrienteMR=adc_value[6]*convVolt;
	corrienteMR=corrienteMR-1.65;
	corrienteMR=corrienteMR/0.66;*/
	// Valores IR en cm  d=65.302-27.77*V

	sharp[3]=adc_value[3]*convVolt;
	sharp[2]=adc_value[2]*convVolt;
	sharp[1]=adc_value[1]*convVolt;
	sharp[0]=adc_value[0]*convVolt;

	sharp[3]=65.302-sharp[3]*27.77;
	sharp[2]=65.302-sharp[2]*27.77;
	sharp[1]=65.302-sharp[1]*27.77;
	sharp[0]=65.302-sharp[0]*27.77;

	//validar pulso adc 4
	if(!combate)
	{
		if(adc_value[4]>2900 && adc_value[4]<3500)
		{
			if(pulsoConstante==false)
			{
				seleccionEstrategia++;
				if(seleccionEstrategia>3)
				{
					seleccionEstrategia=0;
				}
				pulsoConstante=true;
			}
		}
		else if (adc_value[4]>1400 && adc_value[4]<2400) {
			if(pulsoConstante==false)
			{
				seleccionEstrategia--;
				if(seleccionEstrategia<0)
				{
					seleccionEstrategia=3;
				}
				pulsoConstante=true;
			}
		}
		else if (adc_value[4]>3600 && adc_value[4]<4064) {
			if(pulsoConstante==false)
			{
				pulsoConstante=true;
			}

		}
		else{
			pulsoConstante=false;
		}
	}
	HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, ((seleccionEstrategia)>>1)&1);

	HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, (seleccionEstrategia)&1);

}

void printRC5()
{
		sprintf(buffer," start %u ",rc5.start);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
		sprintf(buffer," toogle %u ",rc5.toggle);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
		sprintf(buffer," address %u ",rc5.address);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		sprintf(buffer," command %u ",rc5.command);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		sprintf(buffer," \r\n ");
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

}
void printADC_IR()
{
	sprintf(buffer,"LL= %0.2f",sharp[3]);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		sprintf(buffer,"  LR=  %0.2f",sharp[2]);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		sprintf(buffer,"  RL= %0.2f",sharp[1]);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		sprintf(buffer," RR= %0.2f",sharp[0]);
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

		sprintf(buffer,"\r\n");
		HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
		HAL_Delay(500);
}
void printADC_Volt_Amp()
{


	sprintf(buffer," A_ML= %0.5f ",corrienteML);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer," A_MR= %0.5f ",corrienteMR);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer," voltage= %0.2f ",voltaje);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);


	sprintf(buffer,"\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
	HAL_Delay(500);

}
void printADC()
{

	sprintf(buffer," Boton= %u ",adc_value[4]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer," A_ML= %u ",adc_value[5]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer," A_MR= %u ",adc_value[6]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer," voltage= %u ",adc_value[7]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer,"LL= %u",adc_value[3]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer,"  LR=  %u",adc_value[2]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer,"  RL= %u",adc_value[1]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer," RR= %u",adc_value[0]);
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

	sprintf(buffer,"\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
	HAL_Delay(500);
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
