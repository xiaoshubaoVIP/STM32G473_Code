/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "key.h"
#include "gpio.h"
#include "usart.h"
#include "adc.h"
#include "i2c.h"

osThreadId_t osThreadIdArray[eMaxTask] ;

const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 1
};
  
const osThreadAttr_t KeyTask_attributes = {
  .name = "Key Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

const osThreadAttr_t LedTask_attributes = {
  .name = "Led Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 2
};

const osThreadAttr_t AdcTask_attributes = {
  .name = "Adc Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

const osThreadAttr_t I2CTask_attributes = {
  .name = "I2C Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

const osThreadAttr_t MainTask_attributes = {
.name = "Main Task",
.priority = (osPriority_t) osPriorityNormal,
.stack_size = 128 * 4

};


void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */
void StartDefaultTask(void *argument);
void MainTask(void *argument);
void AdcSampleTask(void *argument);
void Sthc3SensorI2cReadTask(void *argument);


void MX_FREERTOS_Init(void) 
{
	osThreadIdArray[eDefaultTask] = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
	osThreadIdArray[eKeyTask] = osThreadNew(KeyTask, NULL, &KeyTask_attributes);
	osThreadIdArray[eLedTask] = osThreadNew(LedFlashTask, NULL, &LedTask_attributes);
	osThreadIdArray[eAdcTask] = osThreadNew(AdcSampleTask, NULL, &AdcTask_attributes);
	osThreadIdArray[eI2CTask] = osThreadNew(Sthc3SensorI2cReadTask, NULL, &I2CTask_attributes);
	osThreadIdArray[eMainTask] = osThreadNew(MainTask, NULL, &MainTask_attributes);
}

void StartDefaultTask(void *argument)
{
	for(;;)
	{
		osDelay(1);
	}
}

void AdcSampleTask(void *argument)
{
	#define ADCSAMPLECNT 10
	uint32_t AdcValue[2] = {0};
	uint16_t Adc1, Adc2, Adc3, Adc4;
	for(;;)
	{
		AdcValue[0] = 0;
		AdcValue[1] = 0;
		for(int i=0; i<(1<<ADCSAMPLECNT); i++)
		{
			if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED) != HAL_OK)
			{
				printflog("cali fail\r\n");  
			}
			for(int j=0; j<2; j++)
			{
				HAL_ADC_Start(&hadc3);							//开启ADC
				HAL_ADC_PollForConversion(&hadc3, 0xFFFF);		//轮询转换
				AdcValue[j] += HAL_ADC_GetValue(&hadc3);		
			}
			HAL_ADC_Stop(&hadc3);
		}
		Adc1 = AdcValue[0] >> ADCSAMPLECNT;
		Adc2 = AdcValue[1] >> ADCSAMPLECNT;



		AdcValue[0] = 0;
		AdcValue[1] = 0;
		for(int i=0; i<(1<<ADCSAMPLECNT); i++)
		{
			if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
			{
				printflog("cali fail\r\n");  
			}
			for(int j=0; j<2; j++)
			{
				HAL_ADC_Start(&hadc2);							//开启ADC
				HAL_ADC_PollForConversion(&hadc2, 0xFFFF);		//轮询转换
				AdcValue[j] += HAL_ADC_GetValue(&hadc2);		
			}
			HAL_ADC_Stop(&hadc2);
		}
		Adc3 = AdcValue[0] >> ADCSAMPLECNT;
		Adc4 = AdcValue[1] >> ADCSAMPLECNT;


		printflog("\nAdc1=%d:%.3fmV\n", Adc1, (Adc1*2.5/4096));
		printflog("Adc2=%d:%.3fmV\n", Adc2, (Adc2*2.5/4096));
		printflog("Adc3=%d:%.3fmV\n", Adc3, (Adc3*2.5/4096));
		printflog("Adc4=%d:%.3fmV\n", Adc4, (Adc4*2.5/4096));

		vTaskDelay(1000/portTICK_RATE_MS);	
	}
}


#define SHTC3_ADDR					(0x70)
#define SHTC3_CMD_DEV_WRITE 		(0xE0)		//(SHTC3_ADDR<<1)
#define SHTC3_CMD_DEV_READ			(0xE1)		//((SHTC3_ADDR<<1)|0x01)
#define SHTC3_CMD_SOFT_RESET		0x805D		//软件复位命令
#define SHTC3_CMD_READ_ID			0xEFC8		//读取SHTC3 ID命令
#define SHTC3_CMD_WAKEUP			0x3517
#define SHTC3_CMD_READ_HUM_DATA 	0x5c24
#define SHTC3_CMD_READ_TEMP_DATA	0x7866
#define SHTC3_CMD_READ_SLEEP		0xB098
typedef struct
{
	uint8_t HumidityMSB;
	uint8_t HumidityLSB;
	uint8_t HumidityCRC;

	uint8_t temperatureMSB;
	uint8_t temperatureLSB;
	uint8_t temperatureCRC;
}Sthc3MeasureData;

uint8_t Sthc3WriteByte(uint16_t Cmd)
{
	uint8_t WriteCmd[2];
	WriteCmd[0] = (uint8_t)(Cmd>>8);
	WriteCmd[1] = (uint8_t)(Cmd);

	if(HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)SHTC3_CMD_DEV_WRITE , (uint8_t*)&WriteCmd, sizeof(WriteCmd), 1000) != HAL_OK)
	{
		Error_Handler();
		printf("err (L=%d)\n", __LINE__);
	}

	while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
	return 0;
}

uint16_t Sthc3ReadWord(uint16_t Cmd)
{
	uint8_t WriteCmd[2];
	uint16_t ReadData;
	
	WriteCmd[0] = (uint8_t)(Cmd>>8);
	WriteCmd[1] = (uint8_t)(Cmd);
	
	
	if(HAL_I2C_Master_Transmit(&hi2c4, (uint16_t)SHTC3_CMD_DEV_WRITE , (uint8_t*)WriteCmd,  sizeof(WriteCmd), 1000) != HAL_OK) 
	{
		Error_Handler();
	}

	while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);

	if(HAL_I2C_Master_Receive(&hi2c4, SHTC3_CMD_DEV_READ , (uint8_t*)&ReadData, 2, 1000) != HAL_OK) //接收word数据
	{
		Error_Handler();
	}
	return ReadData;
}


void Sthc3ReadHumiAndTemp(void)
{
	uint8_t WriteCmd[2];
	float hum,temp;
	Sthc3MeasureData Sthc3Data = {0};
	uint8_t HumValue, TempValue;

	
	WriteCmd[0] = (uint8_t)(SHTC3_CMD_READ_HUM_DATA>>8);
	WriteCmd[1] = (uint8_t)(SHTC3_CMD_READ_HUM_DATA);

	if(HAL_I2C_Master_Transmit(&hi2c4, SHTC3_CMD_DEV_WRITE , (uint8_t*)WriteCmd,  sizeof(WriteCmd), 1000) != HAL_OK) 
	{
		Error_Handler();
	}

	
	HAL_I2C_Master_Receive(&hi2c4, SHTC3_CMD_DEV_READ, (uint8_t *)&Sthc3Data, sizeof(Sthc3Data), 1000);

	hum = (Sthc3Data.HumidityMSB << 8)|(Sthc3Data.HumidityLSB);
	temp = (Sthc3Data.temperatureMSB << 8 )|Sthc3Data.temperatureLSB;
	HumValue = (uint8_t)(100*((float)hum/65535));				//湿度
	TempValue = (uint8_t)((175*((float)temp/65535)) - 45);		//温度

	printf("\nH:%d, T=%d\n", HumValue, TempValue);
}

void Sthc3SensorI2cReadTask(void *argument)
{
	uint16_t DevID = 0;
	uint16_t ReadData = 0;

	Sthc3WriteByte(SHTC3_CMD_WAKEUP);
	HAL_Delay(1000);


	ReadData = Sthc3ReadWord(SHTC3_CMD_READ_ID);
	DevID = (ReadData<<8)|ReadData>>8;

	printf("\n(0x%04x)ID=0x%04x\n", DevID, DevID&0x083F);
	for(;;)
	{
		Sthc3ReadHumiAndTemp();
		vTaskDelay(1000/portTICK_RATE_MS);	
	}
}


void MainTask(void *argument)
{
	BaseType_t xRecieveStatus = pdFAIL;
	KeyActionQueueMsg KeyActionQueueMsgTag;

	for(;;)
	{
		if(KeyQueue != NULL)
		{
			xRecieveStatus = xQueueReceive(KeyQueue, (void *)&KeyActionQueueMsgTag, portMAX_DELAY);	
		}

		if(xRecieveStatus == pdPASS)
		{
			printflog("KeyMsg:%d\n", KeyActionQueueMsgTag.KeyActionType);
		}
	}
}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
