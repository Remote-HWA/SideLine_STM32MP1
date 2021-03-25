/**
  ******************************************************************************
  * @file           : register.h
  * @brief          : Manipulation on hardware registers
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

#ifndef REGISTER_H_
#define REGISTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void 		Write_Register(uintptr_t Addr, uint32_t Value);
uint32_t 	Read_Register(uintptr_t Addr);
void 		Modify_Register(uintptr_t Addr, uint32_t Value,uint32_t mask);
void 		printBits(uint8_t size, uint32_t data);
uint8_t 	Count_Transitions(uint32_t reg,uint8_t sizeReg);
uint8_t 	Count_Ones(uint32_t reg,uint8_t sizeReg);

#endif /* REGISTER_H_ */
