/**
  ******************************************************************************
  * @file           : temperature.h
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
