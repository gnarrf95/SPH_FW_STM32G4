/*
 * Logger.h
 *
 *  Created on: Apr 5, 2025
 *      Author: manue
 */

#ifndef LOGGER_LOGGER_H_
#define LOGGER_LOGGER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef enum
{
	LOGGER_VERBOSITY_DEBUG = 0,
	LOGGER_VERBOSITY_INFO,
	LOGGER_VERBOSITY_WARNING,
	LOGGER_VERBOSITY_ERROR,

	LOGGER_VERBOSITY_COUNT
} Logger_Verbosity_e;



void Logger_LogMessage(Logger_Verbosity_e verbosity, const char *pFormat, ...)
	__attribute__ ((format (printf, 2, 3)));

void Logger_LogMessageTrace(Logger_Verbosity_e verbosity, const char *pFileName, uint32_t lineNumber, const char *pFormat, ...)
	__attribute__ ((format (printf, 4, 5)));

uint32_t Logger_GetTimestamp(void);
void Logger_SendMessage(char *pBuffer, uint32_t bufferSize);



#define LOG_DEBUG(fmt, ...)		Logger_LogMessageTrace(LOGGER_VERBOSITY_DEBUG, __FILE__, (uint32_t)__LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)		Logger_LogMessageTrace(LOGGER_VERBOSITY_INFO, __FILE__, (uint32_t)__LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)	Logger_LogMessageTrace(LOGGER_VERBOSITY_WARNING, __FILE__, (uint32_t)__LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)		Logger_LogMessageTrace(LOGGER_VERBOSITY_ERROR, __FILE__, (uint32_t)__LINE__, fmt, ##__VA_ARGS__)



#ifdef __cplusplus
}
#endif

#endif /* LOGGER_LOGGER_H_ */
