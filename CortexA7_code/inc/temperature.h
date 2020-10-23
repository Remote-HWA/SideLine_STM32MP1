/*
 * temperature.h
 *
 *  Created on: 22 oct. 2020
 *      Author: 10055748
 */

#ifndef INC_TEMPERATURE_H_
#define INC_TEMPERATURE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

void Decrease_Temperature(uint32_t duration);
int Increase_Temperature(uint32_t attempts);

#endif /* INC_TEMPERATURE_H_ */
