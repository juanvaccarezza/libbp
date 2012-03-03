/*
 *  Created on: Feb 29, 2012
 *      Author: juan
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "BP.h"
#include "Log.h"

BPResult BP_oneByteCommandWR(BP * this, uint8_t command, uint8_t * response);

BPResult BP_connect(BP * this, const char * deviceName) {
	struct termios tios;
	this->pinValues=0;
	Log_debug("Connecting to %s.\n", deviceName);
	this->deviceDescriptor = open(deviceName, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (this->deviceDescriptor < 1) {
		Log_debug("ERROR open the device.\n");
		goto fail;
	}

	if (tcgetattr(this->deviceDescriptor, &this->originalDeviceConfiguration)
			< 0) {
		Log_debug("ERROR getting attributes\n.");
		goto fail;
	}

	memcpy(&tios, &this->originalDeviceConfiguration, sizeof(struct termios));

	tios.c_cflag = CS8 | CLOCAL | CREAD;
	tios.c_iflag = IGNPAR | BRKINT;
	tios.c_oflag = 0;
	tios.c_lflag = 0;
	if (cfsetspeed(&tios, B115200) < 0) {
		Log_debug("ERROR setting speed\n.");
		goto fail;
	}
	if (tcsetattr(this->deviceDescriptor, TCSANOW, &tios) < 0) {
		Log_debug("ERROR setting attributes\n.");
		goto fail;
	}

	if (BP_enterBinaryMode(this) != BPOK) {
		Log_debug("Error while entering binary mode.\n");
		goto fail;
	}

	return BPOK;

	fail: if (this != NULL) {
		if (this->deviceDescriptor >= 0)
			close(this->deviceDescriptor);
	}
	return BPFAIL;

}

BPResult BP_write(BP* this, uint8_t byte) {
	if (write(this->deviceDescriptor, &byte, sizeof(uint8_t))
			< sizeof(uint8_t)) {
		Log_debug("ERROR while writing");
		return BPFAIL;
	}
	return BPOK;
}

BPResult BP_read(BP * this, uint8_t * buffer, uint32_t size) {
	int32_t numc_read = 0;
	uint32_t numLeft = 0;
	int32_t error = 0;
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(this->deviceDescriptor, &rset);
	numLeft = size;

	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT_MS * 1000;

	error = select(this->deviceDescriptor + 1, &rset, NULL, NULL, &tv);
	if (error < 0) {
		Log_debug("ERRO select\n");
		return BPFAIL;
	}

	if (!FD_ISSET(this->deviceDescriptor, &rset)) {
		Log_debug("FD_ISSET:nothing to read\n");
		return BPTIMEOUT;
	}

	while (numLeft > 0) {
		numc_read = read(this->deviceDescriptor, &buffer[size - numLeft],
				numLeft);
		if (numc_read < 0) {
			Log_debug("ERROR  read\n");
			return BPFAIL;
		}
		numLeft -= numc_read;
	}

	return BPOK;
}

BPResult BP_enterBinaryMode(BP * this) {
	Log_debug("Entering binary.\n");
	bool done = false;
	int ntries = 0;
	uint32_t size = 5;
	uint8_t buffer[size];

	while (!done && ntries < 25) {
		BP_write(this, 0);
		usleep(1);
		BP_read(this, buffer, size);
		if (strncmp("BBIO1", (const char *) buffer, 5) == 0) {
			done = true;
		}
		ntries++;
	}
	if (!done) {
		return BPFAIL;
	}

	return BPOK;
}

BPResult BP_disconnect(BP * this) {
	BPResult result;

	if (this->deviceDescriptor < 0) {
		Log_error("Descriptor not open.\n");
		return BPFAIL;
	}

	result = BP_write(this, 0X0F);

	if (result != BPOK) {
		Log_error("Fail to set the BP to text mode.\n");
		return BPFAIL;
	}

	if (tcsetattr(this->deviceDescriptor, TCSAFLUSH,
			&this->originalDeviceConfiguration) < 0) {
		Log_error("Error while restoring device attributes.\n");
		result = BPFAIL;
		return BPFAIL;
	}

	if (close(this->deviceDescriptor) < 0) {
		Log_error("Error while closing the file\n");
		return BPFAIL;
	}
	return result;

}

BPResult BP_writePins(BP * this) {
	uint8_t newStatus;
	// add a 1 in the MSB of the command.
	uint8_t command = this->pinValues | (1 << 7);

	if (BP_oneByteCommandWR(this, command, &newStatus) == BPFAIL) {
		Log_error("Error commanding pin status");
		return BPFAIL;
	}

	this->pinValues = newStatus;
	return BPOK;
}

BPResult BP_configure(BP * this, BP_PIN_CONFIG aux, BP_PIN_CONFIG mosi,
		BP_PIN_CONFIG clk, BP_PIN_CONFIG miso, BP_PIN_CONFIG cs) {
	uint8_t response;
	uint8_t command = 1 << 6;
	command |= (aux == BP_PIN_CONFIG_INPUT ? BP_PIN_AUX : 0);
	command |= (mosi == BP_PIN_CONFIG_INPUT ? BP_PIN_MOSI : 0);
	command |= (clk == BP_PIN_CONFIG_INPUT ? BP_PIN_CLK : 0);
	command |= (miso == BP_PIN_CONFIG_INPUT ? BP_PIN_MISO : 0);
	command |= (cs == BP_PIN_CONFIG_INPUT ? BP_PIN_CS : 0);

	if (BP_oneByteCommandWR(this, command, &response)) {
		Log_error("Error commanding pin configuration");
		return BPFAIL;
	}

	if ((response & 0x1F) != (command & 0x1F)) {
		Log_debug(
				"Bit config readback differ from commanded expected: %X obtained: %X\n",
				(command & 0x1F), (response & 0x1F));
		return BPFAIL;
	}
	return BPOK;
}

BPResult BP_oneByteCommandWR(BP * this, uint8_t command, uint8_t * response) {
	Log_debug("BP_oneByteCommandWR: Sending command; %X\n", command);
	if (BP_write(this, command) == BPFAIL) {
		Log_error("Error writing command to device\n");
		return BPFAIL;
	}
	if (BP_read(this, response, 1) == BPFAIL) {
		Log_error("Error reading back command response status\n");
		return BPFAIL;
	}
	Log_debug("BP_oneByteCommandWR: Obtained command; %X\n", *response);
	return BPOK;
}

BPResult BP_pinSet(BP * this, BP_PIN pin, BP_PIN_STATE state) {
	Log_debug("Configuring pin %d with %d", pin, state);
	this->pinValues = (this->pinValues & ~pin)
			| (state == BP_PIN_STATE_ON ? pin : 0);
	return BPOK;
}

