/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "FreeRTOS.h"
#include "Task.h"


#define LedRedPort		GPIOA
#define LedRedPin		GPIO_PIN_9
#define LedGreenPort	GPIOA
#define LedGreenPin		GPIO_PIN_10
#define LedBluePort		GPIOA
#define LedBluePin		GPIO_PIN_8


typedef enum _eLedType
{
	eLedRed = 0,
	eLedGreen,
	eLedBlue,
	eLedYellow,
}eLedType;


typedef enum _eLedStates
{
	eLedOn = 0,
	eLedOff,
}eLedStates;


void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  /*Configure GPIO pins : PA8 PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_I2C4;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_I2C3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}


void LedControl(eLedType Type, eLedStates States)
{
	switch(Type)
	{
		case eLedRed:
			if(States == eLedOn)
			{
				HAL_GPIO_WritePin(LedRedPort, LedRedPin, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(LedRedPort, LedRedPin, GPIO_PIN_RESET);
			}
			break;

		case eLedBlue:
			if(States == eLedOn)
			{
				HAL_GPIO_WritePin(LedBluePort, LedBluePin, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(LedBluePort, LedBluePin, GPIO_PIN_RESET);
			}
			break;

		case eLedGreen:
			if(States == eLedOn)
			{
				HAL_GPIO_WritePin(LedGreenPort, LedGreenPin, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(LedGreenPort, LedGreenPin, GPIO_PIN_RESET);
			}
			break;

		default:
			break;
	}
}

/**********************************************************************/
//Description:	LedFlashTask()
//Parameters:
//Return:
//Date:
//Author:		quanwu.xu
/**********************************************************************/
void LedFlashTask(void *pvParameters)
{
	for(;;)
	{
		LedControl(eLedRed, eLedOn);		
		vTaskDelay(1000/portTICK_RATE_MS);	
		//osDelay(1000/portTICK_RATE_MS)
		LedControl(eLedRed, eLedOff);		
		vTaskDelay(1000/portTICK_RATE_MS);	
		//osDelay(1000/portTICK_RATE_MS)
	}	
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
