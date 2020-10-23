/**
  ******************************************************************************
  * @file           : register.c
  * @brief          : Manipulation on hardware registers
  ******************************************************************************
  * @attention
  *
  * Copyright (c) Joseph Gravellier 2020 Thales.
  * All rights reserved.
  *
  *
  ******************************************************************************
*/

#include "register.h"

/*
 *
 */
void Write_Register(uintptr_t Addr, uint32_t Value)
{
    volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
    *LocalAddr = Value;
}

/*
 *
 */
uint32_t Read_Register(uintptr_t Addr)
{
	return *(volatile uint32_t *) Addr;
}

/*
 * Modify the value
 */
void Modify_Register(uintptr_t Addr, uint32_t Value,uint32_t mask)
{
    uint32_t data = Read_Register(Addr);
    data = (data & (~mask)) | Value;
    Write_Register(Addr,data);
}

/*
 * Print the binary representation of a 32bit max value
 */
void printBits(uint8_t size, uint32_t data)
{
    for (int i = size-1; i >= 0; i--) {
            printf("%d", (data>>i)  & 1);
    }
}

/*
 * Compute and return register Hamming Weight
 */
uint8_t Count_Ones(uint32_t reg,uint8_t sizeReg)
{

	uint32_t nOnes = 0;

	for(int i = 0 ; i < sizeReg ; i++)
	{
		 if((reg >> i)&1)
		 {
			 nOnes += 1;
		 }
	}

	return nOnes;
}

/*
 * Count 0->1 and 1->0 transitions inside a register
 */
uint8_t Count_Transitions(uint32_t reg,uint8_t sizeReg)
{
	uint32_t state = (reg>>(sizeReg-1)) & 1; // first bit
	uint8_t nTransition = 0;

	for(int i = sizeReg-2 ; i >= 0 ; i--)
	{
		 if(state != ((reg>>i) & 1)) //if the next bit is different
		 {
			 nTransition += 1; //increment transition counter
			 state = ((reg>>i) & 1); //change state
		 }
	}

	return nTransition;
}
