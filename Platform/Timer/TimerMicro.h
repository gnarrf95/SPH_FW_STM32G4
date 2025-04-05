/*
 * TimerMicro.h
 *
 *  Created on: Apr 5, 2025
 *      Author: manue
 */

#ifndef TIMER_TIMERMICRO_H_
#define TIMER_TIMERMICRO_H_

#include <stdint.h>
#include <stm32g4xx_hal.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct
{
	uint32_t timestamp;
} TimerMicro_t;



void TimerMicro_Init(TIM_HandleTypeDef *htim);

uint32_t TimerMicro_Reset(TimerMicro_t *pThis);
uint32_t TimerMicro_Check(TimerMicro_t *pThis);



#ifdef __cplusplus
}
#endif

#endif /* TIMER_TIMERMICRO_H_ */
