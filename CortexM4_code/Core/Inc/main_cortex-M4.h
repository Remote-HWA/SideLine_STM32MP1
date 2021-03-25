/**
******************************************************************************
* @file           : main_cortex_M4.h
* @brief          : CA7-to-CM4 side-channel attack using DLYB blocks in
* 					STM32MP1 SoCs
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

#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <string_ANSSI.h>
#include "stm32mp1xx_hal.h"
#include "stm32mp15xx_disco.h"
#include "math.h"
#include "byte_mask_aes.h"
#include "aes_openssl.h"
#include "masked_combined.h"
#include "aes.h"
#include "aes_ANSSI.h"
#include "stdlib.h"
#include "register.h"

#define OPENSSL 0
#define TINYAES 1
#define MASKEDAES 2
#define HIGHERORDER 3
#define ANSSIAES 4
/* Exported functions prototypes ---------------------------------------------*/
void 	MX_GPIO_Init(void);
void 	MX_DMA_Init(void);
void 	Compute_AES(void);
void 	TransferComplete(DMA_HandleTypeDef *DmaHandle);
void 	TransferError(DMA_HandleTypeDef *DmaHandle);
void 	Error_Handler(void);
/* Private defines -----------------------------------------------------------*/
#define DEFAULT_IRQ_PRIO      1U

#endif /* __MAIN_H */


