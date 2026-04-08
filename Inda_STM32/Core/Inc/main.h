/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_2_Pin GPIO_PIN_13
#define LED_2_GPIO_Port GPIOC
#define LED_ALARMA_Pin GPIO_PIN_14
#define LED_ALARMA_GPIO_Port GPIOC
#define LED_OK_Pin GPIO_PIN_15
#define LED_OK_GPIO_Port GPIOC
#define IR_RR_Pin GPIO_PIN_0
#define IR_RR_GPIO_Port GPIOA
#define IR_RL_Pin GPIO_PIN_1
#define IR_RL_GPIO_Port GPIOA
#define IR_LR_Pin GPIO_PIN_2
#define IR_LR_GPIO_Port GPIOA
#define IR_LL_Pin GPIO_PIN_3
#define IR_LL_GPIO_Port GPIOA
#define Selector_Pin GPIO_PIN_4
#define Selector_GPIO_Port GPIOA
#define Current_ML_Pin GPIO_PIN_5
#define Current_ML_GPIO_Port GPIOA
#define Current_MR_Pin GPIO_PIN_6
#define Current_MR_GPIO_Port GPIOA
#define VOLTAJE_Pin GPIO_PIN_7
#define VOLTAJE_GPIO_Port GPIOA
#define Output_1_Pin GPIO_PIN_0
#define Output_1_GPIO_Port GPIOB
#define Output_2_Pin GPIO_PIN_1
#define Output_2_GPIO_Port GPIOB
#define Output_3_Pin GPIO_PIN_2
#define Output_3_GPIO_Port GPIOB
#define Output_4_Pin GPIO_PIN_12
#define Output_4_GPIO_Port GPIOB
#define EN_MOTOR_Pin GPIO_PIN_12
#define EN_MOTOR_GPIO_Port GPIOA
#define SERVO_Pin GPIO_PIN_15
#define SERVO_GPIO_Port GPIOA
#define Right_Line_Pin GPIO_PIN_3
#define Right_Line_GPIO_Port GPIOB
#define Right_Line_EXTI_IRQn EXTI3_IRQn
#define Back_Line_Pin GPIO_PIN_4
#define Back_Line_GPIO_Port GPIOB
#define Back_Line_EXTI_IRQn EXTI4_IRQn
#define Left_Line_Pin GPIO_PIN_5
#define Left_Line_GPIO_Port GPIOB
#define Left_Line_EXTI_IRQn EXTI9_5_IRQn
#define IR_38KHZ_Pin GPIO_PIN_8
#define IR_38KHZ_GPIO_Port GPIOB
#define IR_38KHZ_EXTI_IRQn EXTI9_5_IRQn
#define LED_1_Pin GPIO_PIN_9
#define LED_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
