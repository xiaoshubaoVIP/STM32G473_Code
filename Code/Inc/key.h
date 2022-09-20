#ifndef __key_H
#define __key_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"


typedef enum{
	KEY1_PRESS = 0x01,
	KEY2_PRESS = 0x02,
	KEY3_PRESS = 0x04,
    KEY4_PRESS = 0x08,
    KEY5_PRESS = 0x10,
    
	KEY_DEFAULT = 0x80,
}eKeyValue;

typedef enum{
	Key_1 = 0x00,
	Key_2,
	Key_3,
	Key_4,
	Key_5,
}eKeyNum;


typedef enum{
	TYPE_NONE = 0x00,
	TYPE_SHORT = 0x01,
	TYPE_CONTINUOUS = 0x02,
	TYPE_LONG = 0x04,
	TYPE_WAIT = 0x10,
}eKeyType;


typedef struct _KeyApi{
	uint16_t keyCounter;	  
	uint8_t keyClickFlag;	
	eKeyType keyType;	
	eKeyValue keyCurrent;
	eKeyValue keyValue;
	eKeyValue keyValueLast;
	uint8_t  Combinationkey;
	uint8_t keyClickCnt;
}KeyApi;


typedef enum _eKeyActionType
{
	eKeyMsgNull = 0x00,
	eKeyMsgTest,
	eKeyMsgPair,
}eKeyActionType;

typedef struct _KeyActionQueueMsg
{
	eKeyActionType KeyActionType;
	int Data;
}KeyActionQueueMsg;

extern QueueHandle_t KeyQueue;

void KeyInit(void);
void KeyTask(void *pvParameters);

#ifdef __cplusplus
}
#endif
#endif /*__ tim_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
