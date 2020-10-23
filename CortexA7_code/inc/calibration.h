/*
 * calibration.h
 *
 *  Created on: 22 oct. 2020
 *      Author: 10055748
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
#endif /* INC_CALIBRATION_H_ */
