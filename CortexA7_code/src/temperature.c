/**
  ******************************************************************************
  * @file           : temperature.c
  * @brief          : Modify CPU temperature by computational stress
  ******************************************************************************
  * @attention
  *
  * Copyright (c) Joseph Gravellier 2020 Thales.
  * Email: joseph.gravellier@external.thalesgroup.com
  * All rights reserved.
  *
  *
  ******************************************************************************
*/

#include "temperature.h"

int Increase_Temperature(uint32_t attempts)
{
	int ret = 0;//,clockcycles = 0;

	for(int iTest = 0 ; iTest < attempts ; iTest++)
	{
		uint64_t rnd = rand();
		double r_d = sqrt((double)rnd) * sqrt((double)rnd);
		long double r_ld = sqrtl((long double)rnd) * sqrtl((long double)r_d);
		printf("too_cold : %Lf\r",r_ld);
	}

	return ret;
}

void Decrease_Temperature(uint32_t duration)
{
	usleep(duration);
}
