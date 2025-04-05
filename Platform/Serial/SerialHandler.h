/*
 * SerialHandler.h
 *
 *  Created on: Apr 5, 2025
 *      Author: manue
 */

#ifndef SERIAL_SERIALHANDLER_H_
#define SERIAL_SERIALHANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stm32g4xx_hal.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef enum
{
	SERIALHANDLER_CALLBACK_BUFFER_OVERRUN = 0,
	SERIALHANDLER_CALLBACK_STOP_BYTES = 1,
	SERIALHANDLER_CALLBACK_TIMEOUT = 2,
} SerialHandler_CallbackReason_t;

typedef void(* SerialHandler_ListenerCallback_t)(uint32_t, SerialHandler_CallbackReason_t);

typedef struct
{
	UART_HandleTypeDef *pUsart;

	uint8_t *pBuffer;
	uint32_t bufferSize;

	bool triggerOnTimeout;
	uint32_t timeoutBitsCount;

	bool triggerOnStopBytes;
	uint8_t *pStopBytes;
	uint32_t stopBytesSize;

	SerialHandler_ListenerCallback_t pListenerCallback;
} SerialHandler_Instance_t;



void SerialHandler_Init(SerialHandler_Instance_t *pThis);

bool SerialHandler_IsTxBusy(SerialHandler_Instance_t *pThis);
ErrorStatus SerialHandler_SendData(SerialHandler_Instance_t *pThis, uint8_t *pData, uint32_t dataSize);



void SerialHandler_UartIrqHandler(SerialHandler_Instance_t *pThis);
void SerialHandler_DmaRxIrqHandler(SerialHandler_Instance_t *pThis);
void SerialHandler_DmaTxIrqHandler(SerialHandler_Instance_t *pThis);



#ifdef __cplusplus
}
#endif

#endif /* SERIAL_SERIALHANDLER_H_ */
