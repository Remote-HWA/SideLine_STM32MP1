/**
  ******************************************************************************
  * @file           : calibration.h
  * @brief          : An automated way to turn a DLYB block into a voltage sensor
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

#ifndef INC_CALIBRATION_H_
#define INC_CALIBRATION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

double 	Measure_Variability(uint8_t dlval, uint16_t clkval);
void 	Auto_Find(uint8_t nTmin, uint8_t nTmax, uint8_t * odlval,uint8_t * oclkval,uint8_t * ominHW,uint8_t * omaxHW,double * oNTransition,double * oVar);
void 	Print_DL_State(uint32_t nSample,uint8_t dlval,uint8_t clkval);
void 	Find_Clock_Delay_Pair(uint8_t dlmin, uint8_t dlmax,uint8_t clkmin, uint8_t clkmax,uint32_t val);
double 	Get_Mean_HW(uint32_t nSample,uint8_t dlval,uint8_t clkval);
uint32_t  Get_HW(uint8_t dlval,uint8_t clkval);
#endif /* INC_CALIBRATION_H_ */
