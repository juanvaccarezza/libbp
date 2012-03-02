/*
 *  Created on: Feb 29, 2012
 *      Author: juan
 */
#include <BP.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
	BP bp;
	BPResult result;
	result = BP_connect(&bp,"/dev/ttyACM0");

	if (result == BPOK) {
		printf("connect OK!! \n");
	} else {
		printf("Connect fail :( \n");
	}
	sleep(5);
	result = BP_disconnect(&bp);
	if (result == BPOK) {
		printf("Disconnect OK!! \n");
	} else {
		printf("Disconnect fail :( \n");
	}
	return 0;
}

