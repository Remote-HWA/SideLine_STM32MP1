/*
 * cpa.h
 *
 *  Created on: 22 oct. 2020
 *      Author: 10055748
 */

#ifndef INC_CPA_H_
#define INC_CPA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>


double ** 	CorrelateClasses(double * GlobalVariance, double * GlobalAverage, double ** ClassAverage,
uint32_t * ClassPopulation, uint32_t ClassNb,uint32_t HypNb, uint32_t nTrace,uint32_t NbSpecifiedSample,
 uint32_t UseSBox,uint32_t Octet,uint32_t bCov,uint32_t bClassif);

uint8_t 	CPA_Results(double ** Correlation,uint16_t nClass,uint32_t nSample,uint8_t keyByte);

uint32_t	HammingWeight(uint32_t Value);

#endif /* INC_CPA_H_ */
