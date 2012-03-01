/*
 *
 *  Created on: Feb 29, 2012
 *      Author: juan
 */

#ifndef BP_H_
#define BP_H_
#include <stdint.h>
#include "Errors.h"

typedef struct _BP{
	uint8_t dummy;
	int deviceDescriptor;
} BP;

BPResult BP_connect(BP * this, const char * deviceName);

#endif /* BP_H_ */
