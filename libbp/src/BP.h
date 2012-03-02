/*
 *
 *  Created on: Feb 29, 2012
 *      Author: juan
 */

#ifndef BP_H_
#define BP_H_
#include <stdint.h>
#include "Errors.h"
#include <termios.h>

#define BUFFER_SIZE     1024
#define TIMEOUT_MS		5
typedef enum _BP_PIN_CONFIG {
	BP_PIN_CONFIG_OUTPUT = 0, BP_PIN_CONFIG_INPUT = 1
} BP_PIN_CONFIG;

typedef enum _BP_PIN_STATE {
	BP_PIN_STATE_OFF = 0, BP_PIN_STATE_ON = 1
} BP_PIN_STATE;

typedef enum _BP_PIN {
	BP_PIN_POWER = 1 < 6,
	BP_PIN_PULLUP = 1 < 5,
	BP_PIN_AUX = 1 < 4,
	BP_PIN_MOSI = 1 < 3,
	BP_PIN_CLK = 1 < 2,
	BP_PIN_MISO = 1 < 1,
	BP_PIN_CS = 1 < 0,
} BP_PIN;

typedef struct _BP {
	int deviceDescriptor;
	char buffer[BUFFER_SIZE];
	struct termios originalDeviceConfiguration;
} BP;

BPResult BP_connect(BP * this, const char * deviceName);
BPResult BP_enterBinaryMode(BP * this);
/**
 * input (INPUT) or output(OUTPUT).
 * @param this
 * @param aux
 * @param mosi
 * @param clk
 * @param miso
 * @param cs
 * @return
 */
BPResult BP_configure(BP * this, BP_PIN_CONFIG aux, BP_PIN_CONFIG mosi,
		BP_PIN_CONFIG clk, BP_PIN_CONFIG miso, BP_PIN_CONFIG cs);
/**
 * Configure pins as In or OUT
 * @param this
 * @param pin
 * @param state
 * @return
 */
BPResult BP_pinSet(BP * this, BP_PIN pin, BP_PIN_STATE state);

/**
 * Transfer the pin setting to  the BP board
 * @see BP_pinSet
 * @param this
 * @return
 */
BPResult BP_writePins(BP * this);

/**
 * Disconnects and resets the BP board.
 * @param this
 * @return
 */
BPResult BP_disconnect(BP * this);

#endif /* BP_H_ */
