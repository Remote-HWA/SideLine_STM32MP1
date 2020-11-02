/*
******************************************************************************
* @file           : main_cortex_A7.h
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
#include <gtk/gtk.h>
#include <cairo.h>
#include <sys/time.h>

#include "slope/slope.h"
#include "cpa.h"
#include "register.h"
#include "filter.h"
#include "temperature.h"
#include "calibration.h"

#define DLYB_BASE_ADRAM 0x58007000
#define DRAM_BASE_ADRAM 0x10040000
#define SRAM_BASE_ADRAM 0x10030000
#define DLYB_LENGTH 12
#define NCLASS 256
#define NBYTE 16
#define OPENSSL 0
#define TINYAES 1

extern uint32_t DLYB_CFGR;
extern uint32_t DLYB_CR;
extern uint32_t SDMMC_CLK;
extern uint32_t v_base_addr;
extern uint32_t v_ddr_base_addr;

void 		Init_CM4(char * filename);
void 		Command_Helper(void);
int 		Map_Registers(int *fd, void **c, int c_addr, int c_size);
uint32_t 	AES_SCA(void);
uint8_t 	Launch_AES(void);
void 		Profile_Init(int cpa);
void 		Profile_deInit(int cpa);

void 			Init_GTK(void);
static gboolean rescale(GtkWidget *button, gpointer data);
static gboolean autocalibration(GtkWidget *button, gpointer data);
static gboolean exitview(GtkWidget *button, gpointer data);
static gboolean view_timer_callback(GtkWidget *widget);
static gboolean cpaupdate(GtkWidget *button, gpointer data);
static gboolean byteselect(GtkWidget *button, gpointer data);
static gboolean aes_timer_callback(GtkWidget *widget);

#endif /* MAIN_CORTEX_A7_H_ */
