/*
 *
 *  Created on: Feb 29, 2012
 *      Author: juan
 */
#include "BP.h"
#include <stdio.h>
#include <fcntl.h>
#include "Log.h"

BPResult BP_connect(BP * this, const char * deviceName){
	Log_debug("Connecting.\n");
	this->deviceDescriptor = open(deviceName, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (this->deviceDescriptor < 1){
		return BPFAIL;
	}
	return BPOK;
}

