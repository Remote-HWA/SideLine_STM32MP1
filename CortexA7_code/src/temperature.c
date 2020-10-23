/*
 * temperature.c
 *
 *  Created on: 22 oct. 2020
 *      Author: 10055748
 */

#include "temperature.h"

int Increase_Temperature(uint32_t attempts)
{
	int ret = 0;//,clockcycles = 0;

	printf("\n\r");
	for(int iTest = 0 ; iTest < attempts ; iTest++)
	{
		uint64_t rnd = rand();
		double r_d = sqrt((double)rnd) * sqrt((double)rnd);
		long double r_ld = sqrtl((long double)rnd) * sqrtl((long double)r_d);
		printf("r_ld: %f\r",r_ld);
	}

	return ret;
}

void Decrease_Temperature(uint32_t duration)
{
	usleep(duration);
}
