/*
 * SerialHandler.c
 *
 *  Created on: Apr 5, 2025
 *      Author: manue
 */
#ifdef STM32G431xx

#include <Serial/SerialHandler.h>
#include <Timer/TimerMicro.h>
#include <stddef.h>

#include <main.h>




#define USART_INSTANCE		pThis->pUsart->Instance

#define DMARX_INSTANCE		pThis->pUsart->hdmarx->DmaBaseAddress
#define DMARX_CHANNEL		pThis->pUsart->hdmarx->Instance
#define DMARX_INDEX			pThis->pUsart->hdmarx->ChannelIndex

#define DMATX_INSTANCE		pThis->pUsart->hdmatx->DmaBaseAddress
#define DMATX_CHANNEL		pThis->pUsart->hdmatx->Instance
#define DMATX_INDEX			pThis->pUsart->hdmatx->ChannelIndex



#ifdef DEBUG
	static TimerMicro_t gSerialTransmitTimer;
	static volatile uint32_t gSerialLastTransmitDuration = 0;
#endif



//------------------------------------------------------------------------------
//
static inline void DMARX_Reset(SerialHandler_Instance_t *pThis)
{
	DMARX_CHANNEL->CCR = 0;
	DMARX_CHANNEL->CCR |= DMA_CCR_MINC;
	DMARX_CHANNEL->CCR |= (DMA_CCR_TCIE | DMA_CCR_TEIE);

	DMARX_CHANNEL->CNDTR = pThis->bufferSize;
	DMARX_CHANNEL->CPAR = (uint32_t)&USART_INSTANCE->RDR;
	DMARX_CHANNEL->CMAR = (uint32_t)pThis->pBuffer;

	DMARX_INSTANCE->IFCR |= 0xF << (DMARX_INDEX * 4);
}

//------------------------------------------------------------------------------
//
static inline void DMARX_Continue(SerialHandler_Instance_t *pThis)
{
	uint32_t data_sent = pThis->bufferSize - DMARX_CHANNEL->CNDTR;

	DMARX_CHANNEL->CCR |= DMA_CCR_MINC;

	DMARX_CHANNEL->CPAR = (uint32_t)&USART_INSTANCE->RDR;
	DMARX_CHANNEL->CMAR = (uint32_t)&pThis->pBuffer[data_sent];

	DMARX_INSTANCE->IFCR |= 0xF << (DMARX_INDEX * 4);
}



//------------------------------------------------------------------------------
//
void SerialHandler_Init(SerialHandler_Instance_t *pThis)
{
	assert_param(pThis != NULL);
	assert_param(pThis->pUsart != NULL);

	USART_INSTANCE->CR1 &= ~USART_CR1_UE;
	DMARX_Reset(pThis);



	// Enable Listener
	if(pThis->pListenerCallback != NULL)
	{
		// Data Buffer
		assert_param(pThis->pBuffer != NULL);
		assert_param(pThis->bufferSize != 0);

		// Stop Bytes
		if(pThis->triggerOnStopBytes)
		{
			assert_param(pThis->pStopBytes != NULL);
			assert_param(pThis->stopBytesSize != 0);

			USART_INSTANCE->CR2 &= ~USART_CR2_ADD_Msk;
			USART_INSTANCE->CR2 |= (pThis->pStopBytes[pThis->stopBytesSize - 1] << USART_CR2_ADD_Pos) & USART_CR2_ADD_Msk;

			USART_INSTANCE->ICR |= USART_ICR_CMCF;
			USART_INSTANCE->CR1 |= USART_CR1_CMIE;
		}

		if(pThis->triggerOnTimeout)
		{
			USART_INSTANCE->RTOR &= ~USART_RTOR_RTO_Msk;
			USART_INSTANCE->RTOR |= (pThis->timeoutBitsCount << USART_RTOR_RTO_Pos) & USART_RTOR_RTO_Msk;

			USART_INSTANCE->ICR |= USART_ICR_RTOCF;
			USART_INSTANCE->CR1 |= USART_CR1_RTOIE;
			USART_INSTANCE->CR2 |= USART_CR2_RTOEN;
		}

		DMARX_CHANNEL->CCR |= DMA_CCR_EN;
		USART_INSTANCE->CR3 |= USART_CR3_DMAR;
		USART_INSTANCE->CR1 |= USART_CR1_RE;
	}

	USART_INSTANCE->CR1 |= USART_CR1_TE;
	USART_INSTANCE->CR1 |= USART_CR1_UE;
}

//------------------------------------------------------------------------------
//
bool SerialHandler_IsTxBusy(SerialHandler_Instance_t *pThis)
{
	assert_param(pThis!= NULL);

	return (DMATX_CHANNEL->CCR & DMA_CCR_EN);
}

//------------------------------------------------------------------------------
//
ErrorStatus SerialHandler_SendData(SerialHandler_Instance_t *pThis, uint8_t *pData, uint32_t dataSize)
{
	assert_param(pThis != NULL);
	assert_param(pData != NULL);
	assert_param(dataSize != 0);

	if(SerialHandler_IsTxBusy(pThis))
	{
		return ERROR;
	}

	DMATX_CHANNEL->CCR = 0;
	DMATX_CHANNEL->CCR |= DMA_CCR_MINC;
	DMATX_CHANNEL->CCR |= DMA_CCR_DIR;
	DMATX_CHANNEL->CCR |= (DMA_CCR_TCIE | DMA_CCR_TEIE);

	DMATX_CHANNEL->CNDTR = dataSize;
	DMATX_CHANNEL->CPAR = (uint32_t)&USART_INSTANCE->TDR;
	DMATX_CHANNEL->CMAR = (uint32_t)pData;

	DMATX_INSTANCE->IFCR |= 0xF << (DMATX_INDEX * 4);

#ifdef DEBUG
	TimerMicro_Reset(&gSerialTransmitTimer);
#endif

	DMATX_CHANNEL->CCR |= DMA_CCR_EN;
	USART_INSTANCE->CR3 |= USART_CR3_DMAT;

	return SUCCESS;
}



//------------------------------------------------------------------------------
//
void SerialHandler_UartIrqHandler(SerialHandler_Instance_t *pThis)
{
	if(USART_INSTANCE->ISR & USART_ISR_RTOF)
	{
		if(pThis->triggerOnTimeout)
		{
			USART_INSTANCE->CR3 &= ~USART_CR3_DMAR;

			uint32_t dataSize = pThis->bufferSize - DMARX_CHANNEL->CNDTR;
			if(dataSize > 0)
			{
				pThis->pListenerCallback(dataSize, SERIALHANDLER_CALLBACK_TIMEOUT);
			}

			DMARX_Reset(pThis);
			DMARX_CHANNEL->CCR |= DMA_CCR_EN;
			USART_INSTANCE->CR3 |= USART_CR3_DMAR;
		}

		USART_INSTANCE->ICR |= USART_ICR_RTOCF;
	}

	if(USART_INSTANCE->ISR & USART_ISR_CMF)
	{
		if(pThis->triggerOnStopBytes)
		{
			USART_INSTANCE->CR3 &= ~USART_CR3_DMAR;

			uint32_t dataSize = pThis->bufferSize - DMARX_CHANNEL->CNDTR;

			bool stopBytesReceived = true;
			for(uint32_t ctr = 0; ctr < pThis->stopBytesSize; ctr++)
			{
				uint32_t bufferPos = (dataSize - pThis->stopBytesSize) + ctr;

				if(pThis->pBuffer[bufferPos] != pThis->pStopBytes[ctr])
				{
					stopBytesReceived = false;
					break;
				}
			}

			if(stopBytesReceived)
			{
				pThis->pListenerCallback(dataSize, SERIALHANDLER_CALLBACK_STOP_BYTES);

				DMARX_Reset(pThis);
				DMARX_CHANNEL->CCR |= DMA_CCR_EN;
			}

			USART_INSTANCE->CR3 |= USART_CR3_DMAR;
		}

		USART_INSTANCE->ICR |= USART_ICR_CMCF;
	}
}

//------------------------------------------------------------------------------
//
void SerialHandler_DmaRxIrqHandler(SerialHandler_Instance_t *pThis)
{
	if(DMARX_INSTANCE->ISR & (0x2 << (DMARX_INDEX * 4)))
	{
		USART_INSTANCE->CR3 &= ~USART_CR3_DMAR;

		pThis->pListenerCallback(pThis->bufferSize, SERIALHANDLER_CALLBACK_BUFFER_OVERRUN);

		DMARX_Reset(pThis);
		DMARX_CHANNEL->CCR |= DMA_CCR_EN;
		USART_INSTANCE->CR3 |= USART_CR3_DMAR;
	}

	if(DMARX_INSTANCE->ISR & (0x8 << (DMARX_INDEX * 4)))
	{
		USART_INSTANCE->CR3 &= ~USART_CR3_DMAR;

		DMARX_Reset(pThis);
		DMARX_CHANNEL->CCR |= DMA_CCR_EN;
		USART_INSTANCE->CR3 |= USART_CR3_DMAR;
	}
}

//------------------------------------------------------------------------------
//
void SerialHandler_DmaTxIrqHandler(SerialHandler_Instance_t *pThis)
{
	// TC Flag
	if(DMATX_INSTANCE->ISR & (0x2 << (DMATX_INDEX * 4)))
	{
		USART_INSTANCE->CR3 &= ~USART_CR3_DMAT;
		DMATX_CHANNEL->CCR &= ~DMA_CCR_EN;

#ifdef DEBUG
		gSerialLastTransmitDuration = TimerMicro_Check(&gSerialTransmitTimer);
#endif

		DMATX_INSTANCE->IFCR |= 0x2 << (DMATX_INDEX * 4);
	}

	// Error Flag
	if(DMATX_INSTANCE->ISR & (0x8 << (DMATX_INDEX * 4)))
	{
		USART_INSTANCE->CR3 &= ~USART_CR3_DMAT;
		DMATX_CHANNEL->CCR &= ~DMA_CCR_EN;

		DMATX_INSTANCE->IFCR |= 0x8 << (DMATX_INDEX * 4);
	}
}



#endif /* STM32G431xx */
