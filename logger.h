/*
 * logger.h
 *
 *  Created on: 03.01.2014
 *
 *  Author: racnets
 */

#ifndef LOGGER_H_
#define LOGGER_H_

int setupLogger(const char* filename);
void doLogging(int id, int mvCount, int mvCountZero, int mvCountNotZero, int mvSumX, int mvSumY);

void loggingCleanUp(void);

#endif /* LOGGER_H_ */
