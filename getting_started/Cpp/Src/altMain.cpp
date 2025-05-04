/*
 * altMain.cpp
 *
 *  Created on: Apr 5, 2025
 *      Author: rbauer
 */
#define USE_QUANTUM
//#define USE_TIMER_INTERRUPT
//#define USE_UART_TX_INTERRUPT
#define USE_UART_TX_DATA
//#define USE_UART_RX_INTERRUPT
//#define USE_UART_RX_BLOCK
//#define USE_UART_RX
//#define USE_UART_DMA
//#define USE_POLLING

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "bsp.hpp"

#include "multiLed.hpp"
#include "digitalOut.hpp"
#include "console.h"
#include "fastRotaryEncoder.hpp"

volatile bool s_isSent = false;
volatile bool s_isReceived = false;

#ifndef USE_QUANTUM
#ifdef USE_UART_TX_DATA
uint8_t s_txData[10240];
uint8_t s_rxData[20];
#endif
#if 0
// printable chars => 32-127 decimal
uint16_t fillBuffer( uint8_t* pBuf, size_t bufSize )
{
	size_t lineSize = 96;
	size_t numLines = bufSize / (lineSize+2); // space for crlf
	int bufPos = 0;

	for ( size_t line = 0; line < numLines; line++)
	{
		char outChar = 32;

		for ( size_t index = 0; index < lineSize; index++ )
		{
			s_txData[bufPos] = outChar++;
			if ( outChar > 127 ) outChar = 32;
			bufPos += 1;
		}

		s_txData[bufPos++] = '\r';
		s_txData[bufPos++] = '\n';
	}

	s_txData[bufPos++] = 0;

	return bufPos;
}
#endif
#else // USE_QUANTUM
// Define functions for enabling/disabling HAL interrupts for critical sections
// and for setting/detecting Q system events.
#ifdef __cplusplus
extern "C" {
#endif

void QF_int_disable_(void)
{
	HAL_SuspendTick();
}

void QF_int_enable_(void)
{
	HAL_ResumeTick();
}

void QF_crit_entry_(void)
{
	HAL_SuspendTick();
}

void QF_crit_exit_(void)
{
	HAL_ResumeTick();
}

namespace BSP {
volatile static uint16_t s_sysAppInterrupt = 0;

volatile void QF_setSysAppEvent()
{
	s_sysAppInterrupt = 1;
}

volatile void QF_clearSysAppEvent()
{
	s_sysAppInterrupt = 0;
}

volatile uint16_t QF_getSysAppEvent()
{
	return s_sysAppInterrupt;
}
}

#ifdef __cplusplus
}
#endif
void appSysTickHandler()
{
	// Use this variable to communicate with QV::onIdle
	// to indicate that a critical interrupt from the app
	// has occurred and needs to be service.
	Q_SysTick_Handler();

	if ( !BSP::QF_getSysAppEvent() )
		BSP::QF_setSysAppEvent();
}
#endif

static uint8_t s_pins[] =
{
		kDigitalPin08, // PA9
		kDigitalPin09, // PC7
		kDigitalPin10, // PB6
		kDigitalPin11, // PA7
		kDigitalPin12, // PA6
};

static uint8_t s_numPins = sizeof(s_pins) / sizeof(s_pins[0]);

CMultiLed g_multiLed( s_pins, s_numPins );

#if 0
void FastRotaryEncoderChanged()
{
	CFastRotaryEncoder::FastRotaryChangedCallback();
}
#endif

void altMain()
{
#if defined(USE_POLLING) || defined(USE_TIMER_INTERRUPT)
	const char notifyLoop[] = "STM32 loop 8 times\n\r";
	const char prompt[] = "\r\nEnter reply: ";
	uint8_t count = 0;
	char outBuf[80];
	//int pin4 = 0;
	//int pin5 = 0;
	int position = 0;
	//FastRotaryEncoder.Setup();
	//FastRotaryEncoder.SetPosition(800);
#elif defined(USE_UART_RX)
	char outBuf[80];
#endif

	while ( 1 )
	{
#if defined(USE_POLLING)
		for ( size_t pinIndex = 0; pinIndex < g_multiLed.MaxPins(); pinIndex++ )
		{
			consoleDisplayArgs("Toggling LED %d\r\n", pinIndex);
			g_multiLed.SetLed(pinIndex, 1);
			HAL_Delay(1000);
			g_multiLed.SetLed(pinIndex, 0);
			HAL_Delay(1000);
			//pin4 = CDigitalOut::Read(kDigitalPin04);
			//pin5 = CDigitalOut::Read(kDigitalPin05);
	        //consoleDisplayArgs("INPUT:: D4=%d D5=%d\r\n", pin4, pin5);
			//position = FastRotaryEncoder.GetPosition();
	        //consoleDisplayArgs("Encoder:: position=%d\r\n", position);
		}

		// selecting MAX_LEDS index causes builtin LED to toggle
		g_multiLed.ToggleLed(g_multiLed.MAX_LEDS);

#elif defined(USE_QUANTUM)
		//consoleDisplay("Invoking bspMain\r\n");
		BSP::bspMain();
#elif defined(USE_TIMER_INTERRUPT)
		// Use timer interrupt to change period of timer pulses
		if ( g_timerInterruptFlag )
		{
			pinState = (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_RESET) ?
				  GPIO_PIN_SET : GPIO_PIN_RESET;

			snprintf(outBuf, sizeof(outBuf), "Toggling LED to %d\r\n", (int)pinState);
			HAL_UART_Transmit(&huart2, (const uint8_t*)outBuf, strlen(outBuf), 1000);
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			g_timerInterruptFlag = 0;

			if ( ++count > 7 )
			{
				HAL_UART_Transmit(&huart2, (const uint8_t*)notifyLoop, sizeof(notifyLoop), 1000);
				count = 0;
			}
		}
#elif defined(USE_UART_TX_INTERRUPT)
		if ( g_uartDataSent )
		{
			HAL_UART_Transmit_IT(&huart2, s_txData, txDataCount);
			g_uartDataSent = 0;
		}

		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_Delay(1000);
#elif defined(USE_UART_RX_INTERRUPT)
		if ( g_uartDataSent )
		{
			HAL_UART_Receive(&huart2, s_rxData, 5, HAL_MAX_DELAY);
			g_uartDataSent = 0;
		}

		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_Delay(1000);
#elif defined(USE_UART_RX_BLOCK)
		HAL_UART_Transmit(&huart2, (const uint8_t*)prompt, strlen(prompt), 1000);
		HAL_UART_Receive(&huart2, s_rxData, 5, HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2, s_rxData, 5, 1000);
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_Delay(1000);
#elif defined(USE_UART_DMA)
#endif
	}
}

