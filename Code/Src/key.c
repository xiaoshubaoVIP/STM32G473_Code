#include "key.h"
#include "usart.h"
#include "cmsis_os.h"
#include "Task.h"


#define KeyPort		GPIOC
#define KeyPin		GPIO_PIN_13

#define KEYCOUNTERNUMLONG 				    (300)		    //3000MS
#define KEYCOUNTERNUMCONTINUOUS_LIMIT 	    (100)			//300MS
#define KEYCOUNTERNUMINTERVAL_LIMIT 		(80)		    //200MS
#define KEYCOUNTERNUMSHORT_LIMIT 		    (70)			//400MS
#define KEYCOUNTERNUMSHORT 				    (1)			    //20MS
#define KEYCOUNTERREPEAT 					(20)		    //20MS



#define KEY1_PRESS()		(HAL_GPIO_ReadPin(KeyPort, KeyPin) == GPIO_PIN_RESET)
//#define KEY2_PRESS()		(HAL_GPIO_ReadPin(KeyPort, KEY2_PIN) == GPIO_PIN_RESET)

KeyApi KeyApiTag;
KeyApi* pKeyApiTag = &KeyApiTag;


const osThreadAttr_t KeyScanTask_attributes = {
	.name = "KeyScan Task",
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = 128 * 2
};


#define KeyQueue_Length 10
QueueHandle_t KeyQueue = NULL;


/**********************************************************************/
//Description:	Key_Init()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void KeyInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(KeyPort, KeyPin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = KeyPin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(KeyPort, &GPIO_InitStruct);

	pKeyApiTag->keyCounter  = 0;
	pKeyApiTag->keyClickFlag = 0;
	pKeyApiTag->keyClickCnt = 0;
	pKeyApiTag->keyValue = KEY_DEFAULT;
	pKeyApiTag->keyCurrent =  KEY_DEFAULT;
	pKeyApiTag->keyValueLast = KEY_DEFAULT;
	pKeyApiTag->keyType = TYPE_NONE;
}

 /**********************************************************************/
 //Description:  GetKeyApiTag()
 //Parameters:
 //Return:
 //Date:
 //Author:	 quanwu.xu
 /**********************************************************************/
 KeyApi* GetKeyApiTag(void)
 {
	 return pKeyApiTag;
 }


 /**********************************************************************/
 //Description:  Key_Init()
 //Parameters:S
 //Return:
 //Date:
 //Author:	 quanwu.xu
 /**********************************************************************/
 uint8_t GetKeyClickFlag(void)
 {
	 return pKeyApiTag->keyClickFlag;
 }


 /**********************************************************************/
 //Description:  Key_Init()
 //Parameters:
 //Return:
 //Date:
 //Author:	 quanwu.xu
 /**********************************************************************/
 void SetKeyClickFlag(uint8_t Flag)
 {
	 pKeyApiTag->keyClickFlag = Flag;
 }


/**********************************************************************/
//Description:	Key_Init()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
static uint8_t KeyGpioValue()
 {
	uint8_t ret = KEY_DEFAULT;
	
	if(KEY1_PRESS())
	{
		ret |= KEY1_PRESS;
	}
	
#if 0
	if(KEY2_PRESS())
	{
		ret |= KEY2_PRESS;
	}
#endif

	return ret;
 }


/**********************************************************************/
//Description:	Key_Init()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void KeyCheck(void)
{
	static uint8_t flag = 0 ;
	KeyApi* pKeyApi = GetKeyApiTag();
	if (GetKeyClickFlag())
	{
		flag = 0;
	}
	else
	{
		if (KeyGpioValue() == KEY_DEFAULT)			//	high
		{
			flag = 1;
			SetKeyClickFlag(0);
			pKeyApi->keyCounter = 0;
			pKeyApi->keyCurrent = KEY_DEFAULT;
			pKeyApi->Combinationkey = 0;
		}
		else
		{	
			if(flag == 1)
			{
				SetKeyClickFlag(1);
			}
		}
	}
}



/**********************************************************************/
//Description:	Key_Init()
//Parameters:
//Return:
//Date:
//Author:   quanwu.xu
/**********************************************************************/
void KeyGpioScan(void)
{
	KeyApi* pKeyApi = GetKeyApiTag();
	uint8_t KeyTemp = KeyGpioValue();

	if(GetKeyClickFlag())
	{
		if (pKeyApi->keyCurrent == KeyTemp) 					//按下按键
		{
			pKeyApi->keyCounter++;
			if(pKeyApi->keyCounter >= KEYCOUNTERNUMLONG)		//10 ms * 300 = 3000ms
			{
				pKeyApi->keyType = TYPE_LONG;
				pKeyApi->keyValue = (eKeyValue)(pKeyApi->keyCurrent & 0x7F);
				SetKeyClickFlag(0);
			}

			if(pKeyApi->keyCounter > 30)
			{
				pKeyApi->Combinationkey = 1;
			}
		}
		else
		{
			if(KeyTemp == KEY_DEFAULT)
			{
				if((pKeyApi->keyCounter >= KEYCOUNTERNUMSHORT)	&& (pKeyApi->keyCounter
												< KEYCOUNTERNUMSHORT_LIMIT))//短按按键
				{
					pKeyApi->keyType = TYPE_SHORT;
					pKeyApi->keyClickCnt++;
					pKeyApi->keyValue = (eKeyValue)(pKeyApi->keyCurrent & 0x7F);
					SetKeyClickFlag(0);
				}
				else
				{
					pKeyApi->keyCounter = 0;
					pKeyApi->keyValue = KEY_DEFAULT;
					pKeyApi->keyCurrent = KEY_DEFAULT;
					SetKeyClickFlag(0);
				}
			}
			else if((pKeyApi->keyCurrent > KeyTemp) && (pKeyApi->Combinationkey == 1))
			{
				if((pKeyApi->keyCounter >= KEYCOUNTERNUMSHORT)	&&	(pKeyApi->keyCounter
												< KEYCOUNTERNUMSHORT_LIMIT))//短按按键
				{
					pKeyApi->keyValue =  (eKeyValue)(pKeyApi->keyCurrent-KeyTemp);
					pKeyApi->keyCounter  = 0;
				}
				else
				{
					pKeyApi->keyCounter  = 0;
					pKeyApi->keyValue = KEY_DEFAULT;
					pKeyApi->keyCurrent = KEY_DEFAULT;
				}
			}
			else
			{
				pKeyApi->keyCounter  = 0;
				pKeyApi->keyValue = KEY_DEFAULT;
				pKeyApi->keyCurrent = KEY_DEFAULT;
			}
		}
		pKeyApi->keyCurrent =  (eKeyValue)KeyTemp;
	}
}



/**********************************************************************/
//Description:	isNewKey()
//Parameters:
//Return:
//Date:
//Authot:  	quanwu.xu
/**********************************************************************/
FlagStatus isNewKey(void)
{
	FlagStatus ret = RESET;
	KeyApi* pKeyApi = GetKeyApiTag();
	if (pKeyApi->keyValue != KEY_DEFAULT)
	{
		pKeyApi->keyValueLast = pKeyApi->keyValue;
		pKeyApi->keyValue = KEY_DEFAULT;
		ret = SET;
	}

	return ret;
}



/**********************************************************************/
//Description:	Key_Init()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
uint8_t GetKeyType(void)
{
	return pKeyApiTag->keyType;
}


/**********************************************************************/
//Description:	Key1Handle()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void Key1Handle(eKeyType Type)
{
	KeyActionQueueMsg KeyActionQueueMsgTag;
	switch(Type)
	{
		case TYPE_SHORT:
			KeyActionQueueMsgTag.KeyActionType = eKeyMsgTest;
			KeyActionQueueMsgTag.Data = 0;
			break;

		case TYPE_CONTINUOUS:
			break;

		case TYPE_LONG:
			KeyActionQueueMsgTag.KeyActionType = eKeyMsgPair;
			KeyActionQueueMsgTag.Data = 0;
			break;

		default:
			break;
	}

	if(KeyQueue != NULL)
	{
		if(errQUEUE_FULL == xQueueSendToBack(KeyQueue,  
									(void *)&KeyActionQueueMsgTag, 5/portTICK_RATE_MS))										
		{
			printflog("Send Key Queue Fail\n");
		}
	}
}


/**********************************************************************/
//Description:	Key2Handle()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void Key2Handle(eKeyType Type)
{
	switch(Type)
	{
		case TYPE_SHORT:
			break;

		case TYPE_CONTINUOUS:
			break;

		case TYPE_LONG:
			break;

		default:
			break;
	}
}


/**********************************************************************/
//Description:	Key3Handle()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void Key3Handle(eKeyType Type)
{
	switch(Type)
	{
		case TYPE_SHORT:
			break;

		case TYPE_CONTINUOUS:
			break;

		case TYPE_LONG:
			break;

		default:
			break;
	}
}

/**********************************************************************/
//Description:	Key4Handle()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void Key4Handle(eKeyType Type)
{
	switch(Type)
	{
		case TYPE_SHORT:
			break;

		case TYPE_CONTINUOUS:
			break;

		case TYPE_LONG:
			break;

		default:
			break;
	}
}

/**********************************************************************/
//Description:	Key5Handle()
//Parameters:
//Return:
//Date:
//Author:	quanwu.xu
/**********************************************************************/
void Key5Handle(eKeyType Type)
{
	switch(Type)
	{
		case TYPE_SHORT:
			break;

		case TYPE_CONTINUOUS:
			break;

		case TYPE_LONG:
			break;

		default:
			break;
	}
}

/**********************************************************************/
//Description:	Key_Handle()
//Parameters:
//Return:
//Date:
//AuthorF:      quanwu.xu
/**********************************************************************/
void KeyHandle(KeyApi *pKeyTag)
{
   KeyApi *pKeyApi = pKeyTag;

	switch (pKeyApi->keyValueLast)
	{
		case KEY1_PRESS:
		{
			printflog("key in\n");
			Key1Handle(pKeyApi->keyType);
		}
		break;


		case KEY2_PRESS:
		{
			Key2Handle(pKeyApi->keyType);
		}
		break;

		case KEY3_PRESS:
		{
			Key3Handle(pKeyApi->keyType);
		}
		break;


		case KEY4_PRESS:
		{
			Key4Handle(pKeyApi->keyType);
		}
		break;


		case KEY5_PRESS:
		{
			Key5Handle(pKeyApi->keyType);
		}
		break;

		default:
			break;
	}

	pKeyApi->keyType = TYPE_NONE;
	pKeyApi->keyValueLast = KEY_DEFAULT;
}


/**********************************************************************/
//Name:			Polling10sTask()
//Description:																  
//Parameters:                        												  
//Return:   																  
//Date:           
//Author:		quanwu.xu  
/**********************************************************************/
static void KeyScanTask(void *pvParameters)
{
	for(;;)
	{
		KeyGpioScan();
		vTaskDelay(10/portTICK_RATE_MS);	
	}
}


/**********************************************************************/
//Name:			KeyTask()
//Description:																  
//Parameters:                        												  
//Return:   																  
//Date:           
//Author:		quanwu.xu  
/**********************************************************************/
void KeyTask(void *pvParameters)
{
	KeyApi *pKeyApi = NULL;
	
	osThreadId_t KeyThreadId  = osThreadNew(KeyScanTask, NULL, &KeyScanTask_attributes);
	if(KeyThreadId == NULL)
	{
		printflog("Create KeyTask Fail\n");	
	}
	
	KeyQueue = xQueueCreate(KeyQueue_Length, sizeof(KeyActionQueueMsg));
	if(KeyQueue == NULL)
	{
		printflog("Create KeyQueue Fail\n");
	}
	else
	{
		printflog("Create KeyQueue OK\n");
	}

	for(;;)
	{
		KeyCheck();
		if (isNewKey())
		{
 			pKeyApi = GetKeyApiTag();
			KeyHandle(pKeyApi);
		}
	}
}


