/*
 * main_cortex_A7.h
 *
 *  Created on: 22 oct. 2020
 *      Author: Joseph Gravellier
 */

#ifndef MAIN_CORTEX_A7_H_
#define MAIN_CORTEX_A7_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <linux/types.h>
#include <linux/hw_breakpoint.h>
#include <fcntl.h>
#include <sys/mman.h>
//#include "gtk/gtk.h"

#include "cpa.h"
#include "register.h"
#include "filter.h"
#include "temperature.h"
#include "calibration.h"

#define DLYB_BASE_ADRAM 0x58007000
#define DRAM_BASE_ADRAM 0xD0000000
#define SRAM_BASE_ADRAM 0x10030000
#define DLYB_LENGTH 12
#define NCLASS 256
#define NBYTE 16

extern uint32_t DLYB_CFGR;
extern uint32_t DLYB_CR;
extern uint32_t SDMMC_CLK;
extern uint32_t v_base_addr;
extern uint32_t v_ddr_base_addr;

void 		Init_CM4(char * filename);
void 		Command_Helper(void);
int 		Map_Registers(int *fd, void **c, int c_addr, int c_size);
uint32_t 	AES_attack(uint8_t dlval, uint32_t sumSample, uint32_t iTrace);


uint32_t 	AES_DMA_attack_Auto(uint32_t nSample);
double 		Get_Mean_Custom(uint32_t nSample,uint8_t dlval,uint8_t clkval);
uint32_t 	AES_DMA_Attack_Custom(uint8_t dlval,uint32_t nSample);
uint8_t 	Decode_DelayLine_Custom(uint32_t reg,uint8_t dlval);

//gtk
//void GTK_Auto(GtkWidget *widget,gpointer   data);

/*typedef struct DL_conf
{
	uint32_t valreg[10][10];
	uint8_t valdl[10];
	uint8_t valclk[10];
	double valvar[10];
	double nTransition;
	double nOnes;
	struct DL_conf* next;
} DL_conf;


typedef struct  {
	int	next_DL;
	DL_conf** DL_list;
} Graphe;*/

#endif /* MAIN_CORTEX_A7_H_ */
