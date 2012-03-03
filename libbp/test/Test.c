/*
 *  Created on: Feb 29, 2012
 *      Author: juan
 */
#include <BP.h>
#include <stdio.h>
#include <unistd.h>

BPResult setPinAndWait(BP * bp, BP_PIN pin, BP_PIN_STATE state) {
	BPResult result;
	result = BP_pinSet(bp, pin, state);
	if (result == BPOK) {
		printf("configure OK \n");
	} else {
		printf("configure FAIL :( \n");
		return BPFAIL;
	}

	result = BP_writePins(bp);
	if (result == BPOK) {
		printf("configure OK \n");
	} else {
		printf("configure FAIL :( \n");
		return BPFAIL;
	}

	usleep(500000);
	return BPOK;
}

BPResult testPinWrite(BP * bp) {
	BPResult result;
	int i, j = 0;
	BP_PIN pin[] = { BP_PIN_POWER, BP_PIN_AUX, BP_PIN_MOSI, BP_PIN_CLK,
			BP_PIN_MISO, BP_PIN_CS };
	result = BP_configure(bp, BP_PIN_CONFIG_OUTPUT, BP_PIN_CONFIG_OUTPUT,
			BP_PIN_CONFIG_OUTPUT, BP_PIN_CONFIG_OUTPUT, BP_PIN_CONFIG_OUTPUT);
	if (result == BPOK) {
		printf("configure OK \n");
	} else {
		printf("configure FAIL :( \n");
		return BPFAIL;
	}

	for (i = 0; i < 6; i++) {

		setPinAndWait(bp, pin[i], BP_PIN_STATE_ON);
		setPinAndWait(bp, pin[i], BP_PIN_STATE_OFF);
	}

	for (j = 1; j < 56; j++) {

		for (i = 0; i < 6; i++) {
			setPinAndWait(bp, pin[i], BP_PIN_STATE_ON);
		}

		for (i = 0; i < 6; i++) {
			setPinAndWait(bp, pin[i], BP_PIN_STATE_OFF);
		}
	}
	return BPOK;

}

int main(int argc, char **argv) {
	BP bp;
	BPResult result;
	result = BP_connect(&bp, "/dev/ttyACM0");

	if (result == BPOK) {
		printf("connect OK!! \n");
	} else {
		printf("Connect fail :( \n");
		return -1;
	}

	testPinWrite(&bp);

	usleep(5000);
	result = BP_disconnect(&bp);
	if (result == BPOK) {
		printf("Disconnect OK!! \n");
	} else {
		printf("Disconnect fail :( \n");
	}
	return 0;
}

