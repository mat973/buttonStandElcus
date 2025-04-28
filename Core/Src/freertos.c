/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"
#include <string.h> 

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
/* USER CODE BEGIN Variables */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern osThreadId_t keyBoardTaskHandle;
extern osThreadAttr_t keyBoardTask_attributes;
//-------------------------
// OS Threads
//-------------------------

uint8_t keyScanBuff[6];

uint32_t conditoonAll[6];

uint8_t checkerAll[6];
enum state {
	FIRST,
	BUTTON_CHECK,
	PWM,
	I2C_CHECK
};
enum direction{
	LEFT,
	RIGHT
};
const uint16_t left_button = ((uint16_t)0x0020);
const uint16_t right_button = ((uint16_t)0x0010);
const uint8_t zero = 0x6F;
const uint8_t point = 0xA5;
const uint8_t voskl = 0x21;

// uint8_t data[7];
const uint8_t slaveAdress[6] = {0x22, 0x32, 0x42, 0x52, 0x62, 0x72};

uint8_t i2cDataBuff[16];

enum state change_state( enum state st, enum direction dir){
	switch (st){
		case FIRST:
			if(dir == LEFT){
				return I2C_CHECK;
			}
			return BUTTON_CHECK;
		case BUTTON_CHECK:
			if(dir == LEFT){
				return FIRST;
			}
			return PWM;
		case PWM:
			if(dir == LEFT){
				return FIRST;
			}
		case I2C_CHECK:
			return 0;
	}
}

void writer(uint8_t i2cDataBuffCopy[7])
{
    char msg[64];
    char button[16];
    char encod1[3];
    char encod2[3];
    char encodePush1;
    char encodePush2;
    char adc1[3];
    char adc2[3];
    char adress_char[6];

    uint16_t cheker = ((i2cDataBuffCopy[0] & 0xF) << 12) | ((i2cDataBuffCopy[0] >> 4) << 8) |
                      ((i2cDataBuffCopy[1] & 0xF) << 4) | ((i2cDataBuffCopy[1] >> 8) << 8);

    lcdSetCursorPosition(4, 0);
    for (uint16_t i = 0; i < 16; i++)
    {
        button[15 - i] = 0x30 + (cheker & 0x1);
        cheker >>= 1;
    }

    lcdPrintStr(button, 12);

    lcdSetCursorPosition(0, 1);
    sprintf(encod1, "%3d", i2cDataBuffCopy[2]);

    lcdPrintStr(encod1, 3);

    lcdSetCursorPosition(4, 1);
    lcdPrintStr(((i2cDataBuffCopy[1] & 0x40) == 0x40) ? "1" : "0", 1);

    lcdSetCursorPosition(15, 1);
    lcdPrintStr(((i2cDataBuffCopy[1] & 0x80) == 0x80) ? "1" : "0", 1);

    lcdSetCursorPosition(17, 1);
    sprintf(encod1, "%3d", i2cDataBuffCopy[3]);
    lcdPrintStr(encod1, 3);

    sprintf(adc1, "%3d", i2cDataBuffCopy[4]);
    lcdSetCursorPosition(0, 3);
    lcdPrintStr(adc1, 3);

    lcdSetCursorPosition(7, 3);
    lcdPrintStr(checkerAll, 6);

    sprintf(adc2, "%3d", i2cDataBuffCopy[5]);
    lcdSetCursorPosition(17, 3);
    lcdPrintStr(adc2, 3);

    //    slaveAdress[0] = 0x22;

    //    data[0] = sizeof(i2cDataBuff);
}

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartKeyBoardTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
			memset(keyScanBuff, 0, sizeof(keyScanBuff));
        // KeyBoard
        for (uint8_t i = 0; i < 4; i++)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 << i, GPIO_PIN_SET);
					  HAL_Delay(1);
            for (uint8_t j = 0; j < 4; j++)
            {
                if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0 << j) == GPIO_PIN_SET){
                    //keyScanBuff[i / 2] |= (1 << ((i % 2) * 4 + j));
								uint8_t bit_pos = i *4 + j; 
								if (bit_pos < 8) {
                    keyScanBuff[0] |= (1 << bit_pos);
                } else {
                    keyScanBuff[1] |= (1 << (bit_pos - 8));
                }
								}
            }
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 << i, GPIO_PIN_RESET);
        }

//        // ADC 1
//        HAL_ADC_PollForConversion(&hadc1, 100);
//        keyScanBuff[4] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1) / 16;
//        HAL_ADCEx_InjectedStop(&hadc1);

//        // ADC 2
//        HAL_ADC_PollForConversion(&hadc2, 100);
//        keyScanBuff[5] = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1) / 16;
//        HAL_ADCEx_InjectedStop(&hadc1);

        // Encoder 1
//        keyScanBuff[2] = __HAL_TIM_GET_COUNTER(&htim1) >> 1;

        // Encoder 2
        //		vPrintSring(pcTaskName);
   //     keyScanBuff[3] = __HAL_TIM_GET_COUNTER(&htim3) >> 1;

        osDelay(500);
    }
}

void StartDisplayTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
 //   lcdInit(&hi2c1, (uint8_t)0x27, (uint8_t)20, (uint8_t)4);
    keyBoardTaskHandle = osThreadNew(StartKeyBoardTask, NULL, &keyBoardTask_attributes);
    uint8_t counter;
		enum state st = FIRST;
    for (;;)
    {
			if(HAL_GPIO_ReadPin(GPIOB, left_button ) == GPIO_PIN_RESET){
				st = change_state(st, LEFT);
			}
			if(HAL_GPIO_ReadPin(GPIOB, right_button ) == GPIO_PIN_RESET){
				for(uint8_t i = 0; i < 127; i++ ) {
					st = change_state(st, LEFT);
				}
			}
			counter = 0;
		switch (st){
		case FIRST:
		counter = 2;
		break;
		case BUTTON_CHECK:
		counter = 3;
		break;
		case PWM:
		counter = 4;
		break;
		case I2C_CHECK:
			counter = 1;
		break;
	}

            for (uint8_t i; i < counter; i++)
            {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                HAL_Delay(500);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                HAL_Delay(500);
            }
	
        
        //		vPrintSring(pcTaskName);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));
    }
}
//void StartI2CTask(void *pvParameters)
//{
//    TickType_t xLastWakeTime;
//    xLastWakeTime = xTaskGetTickCount();

//    //   memset(data, 0, sizeof(data));
////    memset(button, 0x30, sizeof(button));

//    uint8_t i2cDataBuffCopy[7];
//    uint8_t i2cTxDataBuff[16];
//    uint8_t current_address;

//    //	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1);

//    for (;;)
//    {
//        current_address = slaveAdress[__HAL_TIM_GET_COUNTER(&htim1) >> 1];

//        for (uint8_t i = 0; i < 6; i++)
//        {

//            i2cTxDataBuff[0] = 0x41;
//            i2cTxDataBuff[1] = 0x10;

//            HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, slaveAdress[i], i2cTxDataBuff, 2, I2C_FIRST_AND_LAST_FRAME);
//            while ((HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY))
//            {
//            }

//            HAL_I2C_Master_Seq_Receive_IT(&hi2c1, slaveAdress[i], i2cDataBuff, 7, I2C_FIRST_AND_LAST_FRAME);
//            while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
//            {
//            }
//            if (slaveAdress[i] == current_address)
//            {
//                checkerAll[i] = zero;
//                memcpy(i2cDataBuffCopy, i2cDataBuff, 7);
//            }
//            else
//            {
//                if (*(uint32_t *)i2cDataBuff == conditoonAll[i])
//                {
//                    checkerAll[i] = point;
//                }
//                else
//                {

//                    checkerAll[i] = voskl;
//                    conditoonAll[i] = *(uint32_t *)i2cDataBuff;
//                }
//            }
//            memset(i2cDataBuff, 0, 7);
//        }

//        writer(i2cDataBuffCopy);
//        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250));
//    }
//}

/* USER CODE END Application */
