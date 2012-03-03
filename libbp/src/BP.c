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

BPResult BP_connect(BP * this, const char * deviceName) {
	struct termios tios;

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
	result = BP_write(this, 0X0F);
	if (result != BPOK) {
		Log_error("Fail to set the BP to text mode.\n");
		goto fail;
	}

	if (tcsetattr(this->deviceDescriptor, TCSAFLUSH,
			&this->originalDeviceConfiguration) < 0) {
		Log_error("Error while restoring device attributes.\n");
		result = BPFAIL;
		goto fail;
	}

	if (this->deviceDescriptor >= 0)
		close(this->deviceDescriptor);

	return result;

	fail:
	return result;

}

