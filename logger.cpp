/*
 * logger.c
 *
 * Created on: 03.01.2014
 * Last edited: 16.06.2017
 *
 * Author: racnets
 */

#include <stdlib.h>    // EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>     // fopen, fprintf, printf
#include <sys/time.h>

#include "logger.h"

using namespace std;

FILE* fd = NULL;
double t0;

double getTime() {
	struct timeval tp;
	gettimeofday( &tp, NULL );
	return tp.tv_sec + tp.tv_usec/1E6;
}

int setupLogger(const char* filename) {
	t0 = getTime();

	fd = fopen(filename, "w");
	if (fd != NULL) return EXIT_SUCCESS;
	else return EXIT_FAILURE;
}

void doLogging(int id, int mvCount, int mvCountZero, int mvCountNotZero, int mvSumX, int mvSumY) {
	double t = getTime();

	if (fd != NULL) {
		fprintf(fd, "%d\t%u\t%u\t%u\t%u\t%u\t%f\t%d\t%d\t%d\t%d\t%d\n", id, 0, 0, 0, 0, 0, t - t0, mvCount, mvCountZero, mvCountNotZero, mvSumX, mvSumY);
	} else {
		printf("%d\t%u\t%u\t%u\t%u\t%u\t%f\t%d\t%d\t%d\t%d\t%d\n", id, 0, 0, 0, 0, 0, t - t0, mvCount, mvCountZero, mvCountNotZero, mvSumX, mvSumY);
	}
}

void loggingCleanUp(void) {
	if (fd != NULL) fclose(fd);
}
