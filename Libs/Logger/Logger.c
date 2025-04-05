/*
 * Logger.c
 *
 *  Created on: Apr 5, 2025
 *      Author: manue
 */

#include <Logger.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>



#define LOGGER_BUFFER_SIZE		512

#define LOGGER_FILEPATH_SHORT(filepath)		(strrchr(filepath, '/') ? strrchr(filepath, '/') + 1 : filepath)

static const char LOGGER_VERBOSITY_STRINGS[LOGGER_VERBOSITY_COUNT][4] =
{
	"DBG",
	"INF",
	"WRN",
	"ERR"
};

static char gMessageBuffer[LOGGER_BUFFER_SIZE];
static char gLogBuffer[LOGGER_BUFFER_SIZE];



//------------------------------------------------------------------------------
//
void Logger_LogMessage(Logger_Verbosity_e verbosity, const char *pFormat, ...)
{
	uint32_t timestamp = Logger_GetTimestamp();

	if(pFormat == 0)
	{
		return;
	}
	if(verbosity >= LOGGER_VERBOSITY_COUNT)
	{
		return;
	}

	va_list args;
	va_start(args, pFormat);
	vsnprintf(gMessageBuffer, LOGGER_BUFFER_SIZE, pFormat, args);
	va_end(args);

	const uint32_t timeSeconds = timestamp / 1000000;
	const uint32_t timeMicroRemaining = timestamp - (timeSeconds * 1000000);



	int bufferSize = snprintf(gLogBuffer, LOGGER_BUFFER_SIZE, "[ %06lu.%06lu - %s ] %s",
		timeSeconds, timeMicroRemaining,
		LOGGER_VERBOSITY_STRINGS[verbosity],
		gMessageBuffer
	);

	if (bufferSize > 0)
	{
		Logger_SendMessage((char *)gLogBuffer, (uint32_t)bufferSize);
	}
}

//------------------------------------------------------------------------------
//
void Logger_LogMessageTrace(Logger_Verbosity_e verbosity, const char *pFileName, uint32_t lineNumber, const char *pFormat, ...)
{
	uint32_t timestamp = Logger_GetTimestamp();

	if(pFileName == NULL || pFormat == 0)
	{
		return;
	}
	if(verbosity >= LOGGER_VERBOSITY_COUNT)
	{
		return;
	}

	va_list args;
	va_start(args, pFormat);
	vsnprintf(gMessageBuffer, LOGGER_BUFFER_SIZE, pFormat, args);
	va_end(args);

	const char *pFileNameShort = LOGGER_FILEPATH_SHORT(pFileName);

	const uint32_t timeSeconds = timestamp / 1000000;
	const uint32_t timeMicroRemaining = timestamp - (timeSeconds * 1000000);



	int bufferSize = snprintf(gLogBuffer, LOGGER_BUFFER_SIZE, "[ %06lu.%06lu - %s - %s:%lu ] %s\r\n",
		timeSeconds, timeMicroRemaining,
		LOGGER_VERBOSITY_STRINGS[verbosity],
		pFileNameShort, lineNumber,
		gMessageBuffer
	);

	if (bufferSize > 0)
	{
		Logger_SendMessage((char *)gLogBuffer, (uint32_t)bufferSize);
	}
}

//------------------------------------------------------------------------------
//
__attribute__ ((weak)) uint32_t Logger_GetTimestamp(void)
{
	return 0;
}

//------------------------------------------------------------------------------
//
__attribute__ ((weak)) void Logger_SendMessage(char *pBuffer, uint32_t bufferSize)
{
	if(pBuffer != NULL)
	{
		printf("%.*s", (int)bufferSize, pBuffer);
	}
}


