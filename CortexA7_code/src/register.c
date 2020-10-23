/*
 * register.c
 *
 *  Created on: 22 oct. 2020
 *      Author: 10055748
 */
#include "register.h"

void Write_Register(uintptr_t Addr, uint32_t Value)
{
    volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
    *LocalAddr = Value;
}

uint32_t Read_Register(uintptr_t Addr)
{
	return *(volatile uint32_t *) Addr;
}

void Modify_Register(uintptr_t Addr, uint32_t Value,uint32_t mask)
{
    uint32_t data = Read_Register(Addr);
    data = (data & (~mask)) | Value;
    Write_Register(Addr,data);
}

void printBits(uint8_t size, uint32_t data)
{
    for (int i = size-1; i >= 0; i--) {
            printf("%d", (data>>i)  & 1);
    }
}

uint8_t Count_Ones(uint32_t reg,uint8_t sizeReg)
{

	uint32_t nOnes = 0;

	for(int i = 0 ; i < sizeReg ; i++)
	{
		 if((reg >> i)&1) //if the next bit is different
		 {
			 nOnes += 1; //increment transition counter
		 }
	}

	//printf("\n\rnOnes: %d",nOnes);

	return nOnes;
}

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

	//printf("\n\rnTransition: %d",nTransition);

	return nTransition;
}
