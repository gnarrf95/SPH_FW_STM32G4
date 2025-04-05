/*
 * TimerMicro.c
 *
 *  Created on: Apr 5, 2025
 *      Author: manue
 */
#ifdef STM32G431xx

#include <Timer/TimerMicro.h>
#include <stddef.h>

#include <main.h>



static TIM_HandleTypeDef *gTim = NULL;



//------------------------------------------------------------------------------
//
void TimerMicro_Init(TIM_HandleTypeDef *htim)
{
	assert_param(htim != NULL);

	gTim = htim;
	HAL_TIM_Base_Start(gTim);
}

//------------------------------------------------------------------------------
//
uint32_t TimerMicro_Reset(TimerMicro_t *pThis)
{
	assert_param(pThis != NULL);

	uint32_t timestampNow = __HAL_TIM_GET_COUNTER(gTim);
	uint32_t timeDifference = 0;

	if (pThis->timestamp > timestampNow)
	{
		timeDifference = (uint32_t)(((uint64_t)timestampNow + (uint64_t)UINT32_MAX + 1) - (uint64_t)(pThis->timestamp));
	}
	else
	{
		timeDifference = timestampNow - pThis->timestamp;
	}

	pThis->timestamp = timestampNow;
	return timeDifference;
}

//------------------------------------------------------------------------------
//
uint32_t TimerMicro_Check(TimerMicro_t *pThis)
{
	assert_param(pThis != NULL);

	uint32_t timestampNow = __HAL_TIM_GET_COUNTER(gTim);
	uint32_t timeDifference = 0;

	if (pThis->timestamp > timestampNow)
	{
		timeDifference = (uint32_t)(((uint64_t)timestampNow + (uint64_t)UINT32_MAX + 1) - (uint64_t)(pThis->timestamp));
	}
	else
	{
		timeDifference = timestampNow - pThis->timestamp;
	}

	return timeDifference;
}



#endif /* STM32G431xx */
