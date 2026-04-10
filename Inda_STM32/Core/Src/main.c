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
	preparado,
	combate,
	estrategia,
	debug,
	testIR,
	testLinea,
	stop
}MenuZumo;
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
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define convVolt 0.000825396  //3.38/4095
#define divisorRes  0.259259		//r1=100k r2 35k
/*
#define LL_sharp valorfinalestable[3]
#define LR_sharp valorfinalestable[2]
#define RL_sharp valorfinalestable[1]
#define RR_sharp valorfinalestable[0]
*/
#define LL_sharp sharp[3]
#define LR_sharp sharp[2]
#define RL_sharp sharp[1]
#define RR_sharp sharp[0]
#define valorDistancia  30.0  // o 16000

#define filtroShift 1
#define UmbralRuido 10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
Motores motorZumo;
MenuZumo menu;
uint16_t adc_value[8];
uint16_t array[15]={};


uint16_t start=0u;
uint16_t toggle=0u;
uint16_t address=0u;
uint16_t command=0u;
volatile uint16_t RC5_trama=0u;
volatile uint8_t RC5_count=0u;
volatile bool 	RC5_state=false;
volatile uint16_t IR_38KHZ=0;

uint8_t standby=0;


float voltaje=0;
float corrienteML=0;
float corrienteMR=0;
float sharp[4]={};


bool pulsoConstante=false;
int8_t seleccionEstrategia=0;

bool validarGiro=false;
bool direccionGiro=false;

uint32_t tiempo=0u;
uint32_t tiempoAnterior=0u;
uint32_t tiempoAnteriorVoltaje=0u;
uint32_t tiempoMenu=0u;
uint32_t tiempoGiro=0u;


int32_t valorfiltrado[4]={2048u,2048u,2048u,2048u};
int32_t valorfinalestable[4]={2048u,2048u,2048u,2048u};

bool leftLine=false;
bool rightLine=false;
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
void printADC_Volt_Amp();
void printFiltroIR();
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


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  	  RC5_recepcion();	//Aca se reciben datos y se obtiene address y command

	  	  tiempo=HAL_GetTick();
	  	  if((tiempo-tiempoAnterior)>3)
	  	  {
	  		  conversionADC();
	  		  tiempoAnterior=tiempo;
	  	  }

	  	  tiempo=HAL_GetTick();
	  	  if(tiempo-tiempoAnteriorVoltaje>666)
	  	  {
	  		  if(voltaje<6.7)
	  		  {
	  			  HAL_GPIO_TogglePin(LED_ALARMA_GPIO_Port, LED_ALARMA_Pin);
	  			  detener();
	  		  }
	  		  tiempoAnteriorVoltaje=tiempo;
	  	  }

	  	  if (menu==combate) {
	  		  motorZumo.enable=true;

	  		  if(seleccionEstrategia==0)
	  		  {

	  			  estrategia0();
	  		  }
	  		  else if(seleccionEstrategia==1)
	  		  {
	  			  estrategia1();
	  		  }
	  		else if(seleccionEstrategia==2)
	  			  		  {
	  			  			  estrategia2();
	  			  		  }


	  		  motores(&motorZumo);
	  	  }



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

    	uint8_t RC5_bit=(HAL_GPIO_ReadPin(IR_38KHZ_GPIO_Port, IR_38KHZ_Pin)==GPIO_PIN_SET)?1:0;
    	RC5_trama=(RC5_trama<<1)|RC5_bit;
    	RC5_count++;
    	__HAL_TIM_SET_AUTORELOAD(&htim2,1777);
    	    	if(RC5_count>=15)
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
            HAL_NVIC_DisableIRQ(EXTI4_IRQn);
            HAL_NVIC_DisableIRQ(EXTI3_IRQn);


            RC5_trama = 0;
            RC5_count = 0;

            // 2. Sincronización: Esperamos 1333us para leer el primer bit en su zona estable
            __HAL_TIM_SET_AUTORELOAD(&htim2, 1332);
            __HAL_TIM_SET_COUNTER(&htim2, 0);
            HAL_TIM_Base_Start_IT(&htim2);

    }
    else if (GPIO_Pin == Left_Line_Pin) {
    	if(HAL_GPIO_ReadPin(Left_Line_GPIO_Port, Left_Line_Pin)==0){
    		if(seleccionEstrategia==1 || seleccionEstrategia==0)
    		{
    			leftLine=true;
    			rightLine=false;
    			HAL_GPIO_WritePin(LED_ALARMA_GPIO_Port, LED_ALARMA_Pin, 1);
    		}
    			else{
    				detener();
    			}
    		}
    	}

    else if (GPIO_Pin == Right_Line_Pin) {
    	if(seleccionEstrategia==1 || seleccionEstrategia==0)
    	{
    		leftLine=false;
    		rightLine=true;
    		HAL_GPIO_WritePin(LED_ALARMA_GPIO_Port, LED_ALARMA_Pin, 0);

    	}
    	else{
    		detener();
    	}
    }
    else if (GPIO_Pin == Back_Line_Pin) {
    	detener();
    }

}
/*
 * Codigo importante para la recepcion
 * se recibe 2 bit mas para evitar una segunda pulsacion rapida
 *
 */
void RC5_recepcion()
{
	if(RC5_state==true)
	{

		// Se desfasa 2 posiciones mas para evitar leer en rebote de ir
		start=(RC5_trama>>14)&0x03;
		if(start==1){
			toggle=(RC5_trama>>13)&0x01;
			address=(RC5_trama>>8)&0x1F;
			command=(RC5_trama>>2)&0x3F;
			printRC5();
		}

		RC5_state=false;
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		HAL_NVIC_EnableIRQ(EXTI4_IRQn);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	}
	// Se para todo y se deberia inicializar los valores a 0
	if(address==0x1F && command==0x3F)
	{
		detener();
	}
	// comando para entrar en standby y iniciar robot

	if(address==1 && command==9)
	{
		standby++;
		if(standby==1)
		{
			menu=preparado;
			HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_SET);

		}
		else if(standby ==2)
		{
			menu=combate;

			HAL_GPIO_WritePin(LED_OK_GPIO_Port, LED_OK_Pin, GPIO_PIN_RESET);
			standby=1 ;
		}
		start=0;
		address=0;
		command=0;
	}
}
void estrategia2()
{
	if( LR_sharp < valorDistancia || RL_sharp<valorDistancia)
	{
		motorZumo.PWM_Left=500;
		motorZumo.PWM_Right=500;
		validarGiro=false;
		direccionGiro=false;
	}
	else{
		if(validarGiro==false)
		{
			validarGiro=true;
			tiempoGiro=HAL_GetTick();
		}
		else
		{
			tiempo=HAL_GetTick();
			if((tiempo-tiempoGiro)>500)
			{
				direccionGiro=!direccionGiro;
				validarGiro=false;
			}
			if(direccionGiro)
			{
				motorZumo.PWM_Left=-300;
				motorZumo.PWM_Right=300;
			}
			else
			{
				motorZumo.PWM_Left=-300;
				motorZumo.PWM_Right=300;
			}

		}
	}

}
void estrategia0()
{
	if( LR_sharp < valorDistancia || RL_sharp<valorDistancia)
	{
		motorZumo.PWM_Left=600;
		motorZumo.PWM_Right=600;

	}
	else
	{
		motorZumo.PWM_Left=300;
		motorZumo.PWM_Right=-300;

	}
}
void estrategia1()
{

	// ir a linea iquierda en contrarla y buscar enemigo por derecha

	if(leftLine==true && rightLine==false)
	{
		accion=1;
	}
	else if(leftLine==false && rightLine==true)
	{
		accion=2;
	}
	else if (leftLine==false && rightLine==false) {
		accion=0;
	}
	if(LR_sharp < valorDistancia && RL_sharp<valorDistancia)
	{
		accion=0;
		leftLine=false;
		rightLine=false;
	}

switch (accion) {
	case 1:
		motorZumo.PWM_Left=250;
		motorZumo.PWM_Right=-250;

		break;
	case 2:
		motorZumo.PWM_Left=-250;
		motorZumo.PWM_Right=250;

			break;
	case 3:
		motorZumo.PWM_Left=400;
		motorZumo.PWM_Right=350;

			break;
	case 0:
		motorZumo.PWM_Left=450;
		motorZumo.PWM_Right=400;

			break;
	default:
		break;
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
	start=0;
	address=0;
	command=0;
	standby=0;
	menu=stop;
	leftLine=false;
	rightLine=false;
	validarGiro=false;
	direccionGiro=false;
	tiempoGiro=0;
	accion=0;
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
/*
	for(int8_t x=0;x<4;x++)
	{
	valorfiltrado[x]=valorfiltrado[x]+((adc_value[x]-valorfiltrado[x])>>filtroShift);

	}
	for(int8_t x=0;x<4;x++)
	{
		int32_t sumaABS=valorfiltrado[x] - valorfinalestable[x];
		if(sumaABS>=0)
		{
			if(sumaABS>UmbralRuido)
			{
				valorfinalestable[x] = valorfiltrado[x];
			}
		}
		else if(sumaABS<0)
				{
					sumaABS=-sumaABS;
					if(sumaABS>UmbralRuido)
					{
						valorfinalestable[x] = valorfiltrado[x];
					}
				}

	}
*/
	//validar pulso adc 4
	if(menu!=combate)
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

void printFiltroIR()
{
	sprintf(buffer,"LL= %lu",valorfinalestable[3]);
			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

			sprintf(buffer,"  LR=  %lu",valorfinalestable[2]);
			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

			sprintf(buffer,"  RL= %lu",valorfinalestable[1]);
			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

			sprintf(buffer," RR= %lu",valorfinalestable[0]);
			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

			sprintf(buffer,"\r\n");
			HAL_UART_Transmit(&huart3, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
			HAL_Delay(500);
}
void printRC5()
{
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
