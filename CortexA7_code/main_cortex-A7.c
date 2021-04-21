/**
******************************************************************************
* @file           : main_cortex_A7.c
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

#define _GNU_SOURCE
#include "main_cortex_A7.h"



// Utility Global Variables
uint8_t verbose = 0;
time_t start_time = 0;
long current_time = 0;
struct timespec startTime, endTime;
uint32_t duration = 0;

// Correlation Power Analysis & Filter global variables
double * GlobalVariance;
double * GlobalAverage;
double *** ClassAverage;
double *** Correlation;
double ** maxCorrelation;
float * dataArrayStoreFilt;
uint32_t * dataArrayStore;
uint32_t ** ClassPopulation;
uint8_t bnum_selected = 14;
uint8_t localCPA = 0;
uint8_t eFilter = 0;
uint8_t saveCPA = 0;
BWHighPass* cpafilter;
BWLowPass* lowfilter;
uint8_t minByte = 0;
uint8_t maxByte = 16;
uint8_t aesType = 0;
uint32_t iTrace = 0;
uint32_t iClass = 0;
uint32_t iTry = 0;
uint32_t iSample = 0;
uint32_t nSample = 0;
uint32_t nSampleReal = 0;
uint32_t minSample = 0;
uint32_t maxSample = 0;
uint32_t nTrace = 0;
uint32_t iByte = 0;
uint32_t currentByte = 0;
uint32_t hotCount = 0;
uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};
uint8_t bestguess[16];
uint8_t ptArray[16];
uint32_t nRepeat = 1;

// DLYB calibration global variables
uint8_t currentDLval = 41;
uint8_t currentCLKval = 0;
double currentVar = 0;
uint8_t currentMinHW = 0;
uint8_t currentMaxHW = 0;
double currentnTransition = 0;
uint32_t counterHOT = 0;
uint32_t counterCOLD = 0;
uint32_t counterDiscard = 0;
uint32_t counterDiscardOverflow = 0;
uint32_t varHot = 500;
uint32_t varCold = 1000;
double severity = 0.001;

// Register virtual addresses global variables
int map_file = 0;
uint32_t DLYB_CFGR = 0;
uint32_t DLYB_CR = 0;
uint32_t SDMMC_CLK = 0;
uint32_t v_base_addr = 0;
uint32_t v_ddr_base_addr = 0;

// GTK global variables
GtkWidget *  window;
GtkWidget *  boxView;
GtkWidget *  buttonCalib,*buttonRescale,*buttonUpdate,*buttonByte,*buttonExit;
GtkWidget *	 view;
GtkWidget *  chart;
SlopeScale *scaleView,*  scaleAvg, *scaleCorr;
SlopeItem *seriesView,*   seriesAvg, **seriesCorr;
double *x, *y, *yAvg,*yVar;
gboolean timeoutView;
gboolean timeoutAES;
gboolean exitCallback = 0;
uint8_t viewMode = 0;
uint8_t updateend = 0;
uint8_t AESMode = 0;
uint8_t CPAMode = 0;
uint16_t shiftSpeed = 30;
uint16_t refreshRateAES = 500;
uint16_t refreshRateCPA = 1000;
char legendStr[200];


/*
 * Main function
 */
int main(int argc, char *argv[]) 
{

	double variance = 0.;
	uint32_t regval = 0;

	char * filename = "SideLine_CM4.elf";
	char command[50];
	char * user_input;
    volatile struct ddr_core{
        volatile uint32_t control;
    } *core;

    Init_CM4(filename);

    //mmap dlyb registers
    Map_Registers(&map_file,(void **) &core, DLYB_BASE_ADRAM, 0x2fff);
    v_base_addr = &core->control;
    printf("v_base_addr = %08x\n",v_base_addr);

    //mmap dram registers
    Map_Registers(&map_file,(void **) &core, DRAM_BASE_ADRAM, 0xffff);
    v_ddr_base_addr = &core->control;
    printf("v_ddr_base_addr = %08x\n",v_base_addr);

    Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

    DLYB_CR = v_base_addr + 0x1000;
    DLYB_CFGR = v_base_addr + 0x1000 + 0x4;
    SDMMC_CLK = v_base_addr + 0x4;

    printf("virtual DLYB_CR address: %08x\n",DLYB_CR);
    printf("virtual DLYB_CFGR address: %08x\n",DLYB_CFGR);
    printf("virtual SDMMC_CLK address: %08x\n",SDMMC_CLK);

    Auto_Find(1, 10,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);

	/* Print Hello Banner */
	printf("\n\n");
	printf(
	"   _____    __    ___                     \n"
	"  / __(_)__/ /__ / (_)__  ___             \n"
	" _\\ \\/ / _  / -_) / / _ \\/ -_)	       \n"
	"/___/_/\\_,_/\\__/_/_/_//_/\\__/  on STM32\n");

	Command_Helper();

	do{

	printf("SideLine>");
	fgets(command,50,stdin);
	command[strcspn(command,"\n")] = '\0';
	user_input = strtok(command," ");

		/********************** NULL COMMAND *******************/
		if(user_input == NULL){
			//do nothing
		}
		/***************** Increase Temperature Mode *************/
		else if((strcmp(user_input,"hot")==0) || (strcmp(user_input,"HOT")==0))
		{
			user_input = strtok(NULL," ");
			hotCount = (user_input == NULL)?1000:atoi(user_input);
			printf("Hot %d\n",hotCount);
			Increase_Temperature(hotCount);
		}
		/********************** GET MODE ***********************/
		else if((strcmp(user_input,"get")==0) || (strcmp(user_input,"GET")==0))
		{
			printf("Get Mode:\n");
			printf("the current dlval is %d\n",currentDLval);
			printf("the current clkval is %d\n",currentCLKval);
		}

		/********************** SET MODE ***********************/
		else if((strcmp(user_input,"set")==0) || (strcmp(user_input,"SET")==0))
		{
			printf("Set Mode:\n");
			//DLYB value (default 41)
			user_input = strtok(NULL," ");
			currentDLval = (user_input == NULL)?41:atoi(user_input);
			printf("dlval has been set to %d\n",currentDLval);

			//Clock PLL Div (default 0)
			user_input = strtok(NULL," ");
			currentCLKval = (user_input == NULL)?0:atoi(user_input);
			printf("clkval has been set to %d\n",currentCLKval);
		}

		/********************** VIEW MODE **********************/
		else if((strcmp(user_input,"view")==0) || (strcmp(user_input,"VIEW")==0))
		{
			printf("View Mode:\n");
			//number of DLYB value to print (default 1000)
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?1000:atoi(user_input);
			shiftSpeed = (uint16_t)(ceil((float)nSample / 50));
			printf("nSample: %d\n",nSample);
			printf("dlval: %d\n",currentDLval);
			printf("clkval: %d\n",currentCLKval);
			Print_DL_State(nSample,currentDLval,currentCLKval);
			printf("\n");

			//init filter
			lowfilter = create_bw_low_pass_filter(4, 15200,3000);

			//gtk
			viewMode = 1;
		   	gtk_init(&argc, &argv);
		   	Init_GTK();
		   	if (timeoutView != 0) {
		   	g_source_remove(timeoutView);
		   	}
		   	viewMode = 0;
		   	printf("\n");

			free_bw_high_pass(lowfilter);
		}

		/********************** VAR MODE **********************/
		else if((strcmp(user_input,"var")==0) || (strcmp(user_input,"VAR")==0))
		{
			printf("Measure Variability Mode:\n");
			printf("dlval: %d\n",currentDLval);
			printf("clkval: %d\n",currentCLKval);
			variance = Measure_Variability(currentDLval, currentCLKval);
			Print_DL_State(3,currentDLval,currentCLKval);
			printf("Computed Variance: %f\n",variance);
		}

		/********************** FIND MODE **********************/
		else if((strcmp(user_input,"find")==0) || (strcmp(user_input,"FIND")==0))
		{
			//Searched value (default 3640 for 0b111000111000)
			user_input = strtok(NULL," ");
			regval = (user_input == NULL)?3640:atoi(user_input);

			Find_Clock_Delay_Pair(0,127,0,3,regval);
		}

		/********************** AUTO MODE **********************/
		else if((strcmp(user_input,"auto")==0) || (strcmp(user_input,"AUTO")==0))
		{
			printf("Auto configuration Mode:\n");
			currentVar = 0;
			while(currentVar < 0.05)
			{
			Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
			}
		}

		/********************** HELP MODE **********************/
		else if((strcmp(user_input,"help")==0) || (strcmp(user_input,"?")==0) || (strcmp(user_input,"man")==0))
		{
			Command_Helper();
		}

		/******************** AES DISPLAY MODE *****************/
		else if((strcmp(user_input,"aes")==0) || (strcmp(user_input,"AES")==0))
		{
			// sample min (default 0)
			user_input = strtok(NULL," ");
			minSample = (user_input == NULL)?0:atoi(user_input);
			printf("\nSample Min: %d",minSample);

			// sample max (default 5000)
			user_input = strtok(NULL," ");
			maxSample = (user_input == NULL)?5000:atoi(user_input);
			printf("\nSample Max: %d",maxSample);

			nSample = maxSample - minSample;

			if(nSample > MAXSAMPLE)
			{
				printf("The sample number cannot exceed 8000\n");
				maxSample = 8000;
				minSample = 0;
				nSample = 0;		
			}			

			nSampleReal = maxSample - 0;
			printf("\nTotal Sample Number: %d",nSample);

			//number of traces
			user_input = strtok(NULL," ");
			nTrace = (user_input == NULL)?10000:atoi(user_input);

			//AES type
			user_input = strtok(NULL," ");
			aesType = (user_input == NULL)?TINYAES:atoi(user_input);

			iTrace = 0;
			localCPA = 1;
			eFilter = 0;
			counterHOT = 0;
			counterCOLD = 0;
			counterDiscard = 0;
			counterDiscardOverflow = 0;
			varHot = 500;
			varCold = 1000;

			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4
			usleep(100);
			Write_Register(v_ddr_base_addr+0x30,nSampleReal); // send nSample to CM4 for DMA
			Write_Register(v_ddr_base_addr+0x34,aesType); // send encryption type to CM4
			Write_Register(v_ddr_base_addr,0x666); // Refresh key expansion according to AES type
			usleep(100);
			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

			//Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
			Profile_Init(0);
			Print_AES_Type(aesType);
			printf("\nnSample: %d",nSample);
			printf("\nnTrace: %d",nTrace);
			printf("\ndlval: %d",currentDLval);
			printf("\nclkval: %d",currentCLKval);

			// Init time
			start_time = (uint32_t)time(NULL);

			// Init GTK
			AESMode = 1;
		   	gtk_init(&argc, &argv);
			Init_GTK();
			printf("\n");
			AESMode = 0;

			//free arrays
			Profile_deInit(0);
		}

		/****************** CPA MODE *******************/
		else if((strcmp(user_input,"cpa")==0) || (strcmp(user_input,"CPA")==0))
		{
			// Recalculate dl clk pair
			Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);

			// sample min (default 0)
			user_input = strtok(NULL," ");
			minSample = (user_input == NULL)?0:atoi(user_input);
			printf("\nSample Min: %d",minSample);

			// sample max (default 150)
			user_input = strtok(NULL," ");
			maxSample = (user_input == NULL)?150:atoi(user_input);
			printf("\nSample Max: %d",maxSample);

			nSample = maxSample - minSample;
			nSampleReal = maxSample - 0;
			printf("\nTotal Sample Number: %d",nSample);

			// Number of trace to acquire (default 100000)
			user_input = strtok(NULL," ");
			nTrace = (user_input == NULL)?1000000:atoi(user_input);
			printf("\nnTrace: %d",nTrace);

			//AES type
			user_input = strtok(NULL," ");
			aesType = (user_input == NULL)?OPENSSL:atoi(user_input);
			Print_AES_Type(aesType);

			// Enable CPA calculation (default 1 enabled)
			user_input = strtok(NULL," ");
			localCPA = (user_input == NULL)?1:atoi(user_input);
			printf("\nnlocalCPA: %d",localCPA);

			// Enable Butterworth Filter (default 1 enabled)
			user_input = strtok(NULL," ");
			eFilter = (user_input == NULL)?1:atoi(user_input);
			printf("\nFilter: %d",eFilter);

			// Enable Saving (default 1 enabled)
			user_input = strtok(NULL," ");
			saveCPA = (user_input == NULL)?1:atoi(user_input);
			printf("\nsaveCPA: %d",saveCPA);

			//nRepeat
			user_input = strtok(NULL," ");
			nRepeat = (user_input == NULL)?1:atoi(user_input);
			printf("\nnRepeat: %d",nRepeat);

			// Print info
			printf("\nDLval: %d",currentDLval);
			printf("\nCLKval: %d",currentCLKval);
			printf("\nMinHW: %d",currentMinHW);
			printf("\nMaxHW: %d",currentMaxHW);
			printf("\nnTransition: %f",currentnTransition);

			counterHOT = 0;
			counterCOLD = 0;
			counterDiscard = 0;
			counterDiscardOverflow = 0;
			varHot = 500;
			varCold = 1000;

			iTrace = 0;

			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

			// Init Local CPA & profiling
			if(localCPA)
			{
				Profile_Init(localCPA);
			}
			else
			{
				if(eFilter)
				{
					dataArrayStoreFilt = (float*)malloc(sizeof(float)*nSample);
				}
				else
				{
					dataArrayStore = (uint32_t*)malloc(sizeof(uint32_t)*nSample);
				}
			}

			// Init Filter
			if(eFilter)
			{
				cpafilter = create_bw_high_pass_filter(4, 15200, 900);
			}

			// Init time
			start_time = (uint32_t)time(NULL);

			// Print key used by CM4 for the example
			printf("\nkey : ");
			for(int k_i = 0 ; k_i < 16 ; k_i++){printf("%02x",exKeyArray[k_i]);}
			printf("\n");
			for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();}

			//Send DMA parameters to CM4
			Write_Register(v_ddr_base_addr+0x30,nSampleReal);
			Write_Register(v_ddr_base_addr+0x34,aesType);
			Write_Register(v_ddr_base_addr,0x666); // Refresh key expansion according to AES type
			usleep(100);
			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

			// Init GTK
			if(localCPA)
			{
				CPAMode = 1;
				gtk_init(&argc, &argv);
				Init_GTK();
				printf("\n");
				CPAMode = 0;
			}
			else //external CPA
			{
				while(Launch_AES() != 0);
			}

			//free arrays and filters
			if(eFilter){free_bw_high_pass(cpafilter);}

			if(localCPA)
			{Profile_deInit(localCPA);}
			else
			{
			if(eFilter)
			{free(dataArrayStoreFilt);}
			else{free(dataArrayStore);}
			}

		}
		else
		{
			printf("Unknown Command %s\n",user_input);
		}


	}
	while(1);
}


uint32_t AES_SCA(void)
{
		//uint8_t ptArray[16];
		uint8_t ctArray[16];
		uint32_t iSample = 0;
		uint8_t state = 1;
		uint32_t dataArray[nSample];
		uint32_t temp32to8 = 0;
		uint32_t temp8to32=0;
		float filteredDataArray[nSample];
		uint32_t count = 0;
		//double mean = 0.;

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (currentDLval << 8));

		// Generate a random plain text
		/*if(((iTrace % nRepeat) == 0) && (iTrace > 0)) //generate plaintext every nRepeat (default nRepeat = 1)
		{
			for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();}
		}*/

		// Convert 8bit plaintext into 32bit and send to CM4
		for(int u = 0 ; u < 4 ; u++)
		{
			temp8to32=0;
			for(int v = 0 ; v < 4 ; v++){
				temp8to32 |= (uint32_t)(ptArray[u*4+v])<<(24-8*v);
			}
			Write_Register(v_ddr_base_addr+0x4+u*4,temp8to32);
		}

		//Re-init communication, send *plaintext ready* to CM4
		Write_Register(v_ddr_base_addr,0x1);

		// Wait for CM4 *ready to encrypt*
		while(Read_Register(v_ddr_base_addr) != 2){}

		//send *start encryption* to CM4
		Write_Register(v_ddr_base_addr,0x3);

		//Wait for CM4 *end of encrypt*
		while(Read_Register(v_ddr_base_addr) != 4){}

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		// Import ciphertext from CM4 convert 32bit to 8bit
		for(int u = 0 ; u < 4 ; u++)
		{
			temp32to8 = Read_Register(v_ddr_base_addr+0x14+u*4);

			for(int v = 0 ; v < 4 ; v++)
			{
				ctArray[u*4+v] = (temp32to8 >> (24-v*8)) & 0xff;
			}
		}

		for(iSample = minSample ; iSample < maxSample ; iSample++)
		{
			dataArray[iSample-minSample] = Read_Register(v_ddr_base_addr+0x40+iSample*4);

			count = 0;
			for(int iBit = 0 ; iBit < DLYB_LENGTH ; iBit++)
			{
				count += (dataArray[iSample-minSample] >> (16+iBit)) & 1;
			}

			//mean += count;
			dataArray[iSample-minSample] = count-currentMinHW;
		}


		//mean = mean/(double)nSample;
		//printf("\nmean: %f",mean);

		//if((mean > currentMinHW+severity) && (mean < currentMaxHW-severity))
		//{
			state = 1;

			if(!localCPA)
			{
				for(iSample = 0 ; iSample < nSample ; iSample++)
				{
					if(eFilter)
					{
						dataArrayStoreFilt[iSample] += bw_high_pass(cpafilter,(float)(dataArray[iSample])/nRepeat);
					}
					else
					{
						dataArrayStore[iSample] += dataArray[iSample];
					}
				}


				if(((iTrace+1) % nRepeat) == 0)
				{

					printf("\nplaintext : ");
					for(int u = 0; u<16 ; u++){printf("%02x",ptArray[u]);}
					printf("\n");


					if(nRepeat == 1)
					{
						for(iSample = 0 ; iSample < nSample ; iSample++)
						{
							printf("%c",dataArray[iSample]+50);
						}

						if(iTrace < 10) // only print the first 10 ciphertexts to reduce size of file
						{
						printf("\nciphertext : ");
						for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}
						}
					}
					else
					{
						for(iSample = 0 ; iSample < nSample ; iSample++)
						{
							if(eFilter)
							{
								printf("%f ",dataArrayStoreFilt[iSample]);
								dataArrayStoreFilt[iSample] = 0;
							}
							else
							{
								printf("%d ",dataArrayStore[iSample]);
								dataArrayStore[iSample] = 0;
							}
						}

						printf("\nciphertext : ");
						for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}
					}
				}
			}
			else
			{

				if(eFilter)
				{

					/*********** FILTER ************/
					//printf("filtered value: \n");
					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						filteredDataArray[iSample] = bw_high_pass(cpafilter,dataArray[iSample]);
						//printf("%f ",filteredDataArray[iSample]);
					}
					//printf("\n");

					/*********** PROFILE ************/
					if(!AESMode)
					{
						for(iByte = minByte ; iByte < maxByte ; iByte++)
						{
							//printf("%02x ",ptArray[iByte]);
							ClassPopulation[iByte][ptArray[iByte]] += 1;

							for(iSample = 0 ; iSample < nSample ; iSample++)
							{
								ClassAverage[iByte][ptArray[iByte]][iSample] += (double)filteredDataArray[iSample];
							}
						}
						for(iSample = 0 ; iSample < nSample ; iSample++)
						{

							GlobalVariance[iSample] += pow((double)filteredDataArray[iSample],2);
							GlobalAverage[iSample] += (double)filteredDataArray[iSample];
						}
					}
					else
					{
						for(iSample = 0 ; iSample < nSample ; iSample++)
						{
							GlobalAverage[iSample] += (double)filteredDataArray[iSample];
						}
					}



				}
				else
				{
					/*********** PROFILE ************/
					if(!AESMode)
					{
						for(iByte = minByte ; iByte < maxByte ; iByte++)
						{

							ClassPopulation[iByte][ptArray[iByte]] += 1;
							for(iSample = 0 ; iSample < nSample ; iSample++)
							{
								ClassAverage[iByte][ptArray[iByte]][iSample] += (double)dataArray[iSample];
							}
						}

						for(iSample = 0 ; iSample < nSample ; iSample++)
						{
							GlobalVariance[iSample] += pow((double)dataArray[iSample],2);
							GlobalAverage[iSample] += (double)dataArray[iSample];
						}
					}
					else
					{
						for(iSample = 0 ; iSample < nSample ; iSample++)
						{
							GlobalAverage[iSample] += (double)dataArray[iSample];
						}
					}


				}

			}
		/*}
		else
		{
			state = 0;
		}*/

		return state;
}

void Profile_Init(int cpa)
{
	printf("\nInitializing profiling accumulators...");
	GlobalVariance = (double*)malloc(sizeof(double)*nSample);
	GlobalAverage = (double*)malloc(sizeof(double)*nSample);

	for(int iSample = 0 ; iSample < nSample ; iSample++)
	{
		GlobalVariance[iSample] = 0.;
		GlobalAverage[iSample] = 0.;
	}

	if(cpa)
	{
		maxCorrelation = (double**)malloc(sizeof(double*)*NBYTE);

		for(int iByte = 0 ; iByte < NBYTE ; iByte++)
		{
			maxCorrelation[iByte] = (double*)malloc(sizeof(double)*nSample);

			for(int iSample = 0 ; iSample < nSample ; iSample++)
			{
				maxCorrelation[iByte][iSample] = 0.;
			}
		}

		ClassPopulation = (uint32_t**)malloc(sizeof(uint32_t*)*NBYTE);
		ClassAverage = (double***)malloc(sizeof(double**)*NBYTE);
		Correlation = (double***)malloc(sizeof(double**)*NBYTE);

		for(int iByte = 0 ; iByte < NBYTE ; iByte++)
		{
			ClassAverage[iByte] = (double**)malloc(sizeof(double*)*NCLASS);
			Correlation[iByte] = (double**)malloc(sizeof(double*)*NCLASS);
			ClassPopulation[iByte] = (uint32_t*)malloc(sizeof(uint32_t)*NCLASS);


			for(int iClass = 0 ; iClass < NCLASS ; iClass++)
			{
				ClassPopulation[iByte][iClass] = 0;
				ClassAverage[iByte][iClass] = (double*)malloc(sizeof(double)*nSample);
				Correlation[iByte][iClass] = (double*)malloc(sizeof(double)*nSample);

				for(int iSample = 0 ; iSample < nSample ; iSample++)
				{
					ClassAverage[iByte][iClass][iSample] = 0.;
					Correlation[iByte][iClass][iSample] = 0.;
				}
			}
		}
	}

}

void Profile_deInit(int cpa)
{
	printf("\nFree memory...\n");
	free(GlobalAverage);
	free(GlobalVariance);

	if(cpa)
	{
		free(ClassAverage);
		free(maxCorrelation);
		free(Correlation);
		free(ClassPopulation);
	}
}

uint8_t Launch_AES(void)
{
	double mean = Get_Mean_HW(50,currentDLval,currentCLKval);

	if(mean >= currentMaxHW-0.1)
	{
		//printf("Too hot ! - mean : %f\n",mean);
		//printf("\nToo hot, countertemp: %d\n",counterDiscard);
		usleep(varCold);
		counterDiscard++;
		counterHOT++;
	}
	else if((mean > currentMinHW+severity) && (mean < currentMaxHW-severity))
	{
		//printf("ok ! - mean : %f\n",mean);



		iTrace += AES_SCA();

		if((iTrace%nRepeat==0) && (iTrace > 0))
		{
			for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();}
		}

		if((iTrace%1000==0) || (iTrace==nTrace))
		{
			current_time = (uint32_t)time(NULL);
			if(localCPA)
			{
			printf("\nProcessed traces: %d - Time: %.2f/%.2fmin - Progression: %.2f%%",iTrace,(double)(current_time - start_time)/60.0,((double)(current_time - start_time)/((double)(iTrace)/nTrace))/60.0,((double)(iTrace)/nTrace)*100);
			}
		}

		if(counterDiscard > 0)
		{
			counterDiscard--;
		}
	}
	else
	{
		//printf("\ncountertemp: %d, mean = %f, currentMinHW = %d, currentMaxHW = %d",counterDiscard,mean,currentMinHW,currentMaxHW);
		//printf("\nToo cold, countertemp: %d\n",counterDiscard);
		Increase_Temperature(varHot);
		counterDiscard++;
		counterCOLD++;
	}

	if(counterDiscard == 2000)
	{
		printf("\nDiscard Counter Overflow: %d !\n varCold value: %d\n varHot value: %d\n",counterDiscardOverflow,varCold,varHot);


		if(mean <= currentMaxHW-0.1)
		{
			usleep(5000000);
			varHot = 500;
			varCold += 4000;
		}
		else
		{
			Increase_Temperature(50000);
			varCold = 1000;
			varHot += 2000;
		}

		if(counterDiscardOverflow == 3)
		{
			currentVar = 0;
			while(currentVar < 0.1)
			{
				Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
			}

			varHot = 500;
			varCold = 1000;
			counterDiscardOverflow = 0;
		}

		counterDiscardOverflow++;
		counterDiscard = 0;

	}

	if(iTrace == nTrace){return 0;}else{return 1;}
}

/*
 * Command helper that can be called with "help" or "?" command
 */
void Command_Helper(void)
{
	printf("\nAvalaible AES implementations (AEStype):");
	printf("\nOPENSSL       0");
	printf("\nTINYAES       1");
	printf("\nMASKEDAES     2");
	printf("\nHIGHERORDER   3");
	printf("\nANSSIAES      4");

	printf("\n\nCommand Helper:");
	printf("\n-------------------------------------------------------------------------");
	printf("\n|    cmd    |              Parameters            |      Description     |");
	printf("\n|-----------------------------------------------------------------------|");
	printf("\n| set       | <dlValue> <clkValue>               | Set dlval and clkval |");
	printf("\n| get       |                                    | Print dlval clkval   |");
	printf("\n| view      | <nSample>                          | Print DLYB state     |");
	printf("\n| var       |                                    | Compute variance     |");
	printf("\n| find      | <decimal value>                    | Test a configuration |");
	printf("\n| auto      |                                    | Auto DLYB calibration|");
	printf("\n| aes       | <smin> <smax> <nTrace> <AEStype>   | AES overview test    |");
	printf("\n| cpa       | <smin> <smax> <nTrace> <AEStype>   | AES CPA attack       |");
	printf("\n-------------------------------------------------------------------------");
	printf("\nexample 1 : \"view 1000\"           = Display delay-line-based oscilloscope (1000 samples)");
	printf("\nexample 2 : \"aes 0 5000 10000 1\"  = Display the avg of 10000 tiny AES traces (5000 samples)");
	printf("\nexample 3 : \"cpa 0 150 1000000 0\" = Conduct CPA on OpenSSL AES (1000000 traces, 150 samples)\n\n");

}

/*
 * Start CM4 encryption application
 */
void Init_CM4(char * filename)
{
	char linux_cmd[200];
	system("rm /tmp/info");
	system("echo stop > /sys/class/remoteproc/remoteproc0/state");
	sprintf(linux_cmd,"rm /lib/firmware/%s",filename);
	printf("%s\n",linux_cmd);
	system(linux_cmd);
	sprintf(linux_cmd,"cp /home/root/SideLine/%s /lib/firmware",filename);
	printf("%s\n",linux_cmd);
	system(linux_cmd);
	sprintf(linux_cmd,"echo %s > /sys/class/remoteproc/remoteproc0/firmware",filename);
	printf("%s\n",linux_cmd);
	system(linux_cmd);
	printf("echo start > /sys/class/remoteproc/remoteproc0/state\n");
	system("echo start > /sys/class/remoteproc/remoteproc0/state");
}

/*
 * Obtain virtual addresses to control hardware registers
 */
int Map_Registers(int *fd, void **c, int c_addr, int c_size)
{
	if ((*fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
	fprintf(stderr, "Error: could not open /dev/mem!\n");
	return -1;
	}

	if ((*c = mmap(NULL, c_size, PROT_READ | PROT_WRITE,
	MAP_SHARED, *fd, c_addr)) == (void *) -1) {

	fprintf(stderr, "Error: could not map memory to file!\n");
	return -1;

	}

	return 0;
}


void Init_GTK(void)
{
   	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   	boxView    = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
   	gtk_box_set_spacing (GTK_BOX(boxView),20);
   	gtk_window_set_default_size(GTK_WINDOW(window), 20, 30);
   	gtk_window_set_deletable(GTK_WINDOW(window),FALSE);
   	gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
   	gtk_window_set_title (GTK_WINDOW(window),"SideLine");
   	//gtk_window_set_decorated(GTK_WINDOW(window),FALSE);
   	gtk_window_set_keep_above(GTK_WINDOW(window),TRUE);//not handled in wayland
   	gtk_window_move(GTK_WINDOW(window),0,0);//not handled in wayland
   	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
   	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
   	gtk_container_add(GTK_CONTAINER(window), boxView);

	if(viewMode) //view Mode
	{
		printf("\nView window initialization...");
		buttonCalib = gtk_button_new_with_label("Calibrate");
		buttonRescale = gtk_button_new_with_label("Rescale");
		buttonExit = gtk_button_new_with_label("Exit");
		g_signal_connect(G_OBJECT(buttonCalib), "clicked", G_CALLBACK(autocalibration), NULL);
		g_signal_connect(G_OBJECT(buttonRescale), "clicked", G_CALLBACK(rescale), NULL);
		g_signal_connect(G_OBJECT(buttonExit), "clicked", G_CALLBACK(exitview), NULL);
	   	gtk_box_pack_start(GTK_BOX(boxView), buttonCalib, FALSE, TRUE, 2);
		gtk_box_pack_start(GTK_BOX(boxView), buttonRescale, FALSE, FALSE, 4);
		gtk_box_pack_start(GTK_BOX(boxView), buttonExit, FALSE, FALSE, 4);
		chart = slope_chart_new_detailed("DLYB Oscilloscope",700,350,0);
		x = g_malloc(nSample * sizeof(double));
		y = g_malloc(nSample * sizeof(double));


		clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
		for (iSample = 0; iSample < nSample; iSample++)
		{
			x[iSample] = iSample;
			y[iSample] = (double)(bw_low_pass(lowfilter,(float)(Get_HW(currentDLval,currentCLKval))));
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
		duration = (uint32_t)((endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000000);
		printf("\nDuration: %d ms", duration);

		scaleView = slope_xyscale_new_axis("Sample", "Amplitude", "Average");
		slope_scale_set_layout_rect(scaleView, 0, 0, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleView);

		seriesView = slope_xyseries_new_filled("Average", x, y, nSample, "l-");
		slope_scale_add_item(scaleView, seriesView);

		timeoutView = g_timeout_add(duration+50, (GSourceFunc) view_timer_callback, (gpointer) chart);
		printf("\nView window init done\n");

	}
	else if(AESMode) //AES Mode
	{

		printf("\nAES window initialization...");
		buttonExit = gtk_button_new_with_label("Exit");
		g_signal_connect(G_OBJECT(buttonExit), "clicked", G_CALLBACK(exitview), NULL);
		gtk_box_pack_start(GTK_BOX(boxView), buttonExit, FALSE, FALSE, 4);
		chart = slope_chart_new_detailed("AES Results",700,350,0);

		clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
		for(iTry = 0 ;  iTry < refreshRateAES ;  iTry++)
		{
			 if(Launch_AES()==0){break;}
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
		duration = (uint32_t)((endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000000);
		printf("\nDuration: %d ms for %d traces per timeout\n", duration,refreshRateAES);

		x = g_malloc(nSample * sizeof(double));
		y = g_malloc(nSample * sizeof(double));
		for (iSample = 0; iSample < nSample; iSample++)
		{
			x[iSample] = iSample;
			y[iSample] = GlobalAverage[iSample]/iTrace;
		}

		scaleAvg = slope_xyscale_new_axis("Sample", "Amplitude", "Average");
		slope_scale_set_layout_rect(scaleAvg, 0, 0, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleAvg);

		seriesAvg = slope_xyseries_new_filled("Average", x, y, nSample, "l-");
		slope_scale_add_item(scaleAvg, seriesAvg);

		timeoutAES = g_timeout_add(duration, (GSourceFunc) aes_timer_callback, (gpointer) chart);
		printf("\nAES window init done\n");
	}
	else if(CPAMode)
	{
		printf("\nCPA window initialization...");
		buttonUpdate = gtk_button_new_with_label("Update");
		buttonByte = gtk_button_new_with_label("Byte++");
		buttonExit = gtk_button_new_with_label("Exit");
		g_signal_connect(G_OBJECT(buttonUpdate), "clicked", G_CALLBACK(cpaupdate), NULL);
		g_signal_connect(G_OBJECT(buttonByte), "clicked", G_CALLBACK(byteselect), NULL);
		g_signal_connect(G_OBJECT(buttonExit), "clicked", G_CALLBACK(exitview), NULL);
		gtk_box_pack_start(GTK_BOX(boxView), buttonUpdate, FALSE, FALSE, 4);
		gtk_box_pack_start(GTK_BOX(boxView), buttonByte, FALSE, FALSE, 4);
		gtk_box_pack_start(GTK_BOX(boxView), buttonExit, FALSE, FALSE, 4);
		chart = slope_chart_new_detailed("CPA Results",700,420,0);

		currentByte 	= 0;
		x    	= (double*)g_malloc(nSample * sizeof(double));
		yAvg    = (double*)g_malloc(nSample * sizeof(double));
		yVar    = (double*)g_malloc(nSample * sizeof(double));


		printf("\nCorr Array Init OK");

		clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
		for(iTry = 0 ;  iTry < refreshRateCPA ;  iTry++)
		{
			if(Launch_AES()==0){break;}
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

		duration = (uint32_t)((endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000000);

		printf("\nDuration: %d ms for %d traces per timeout", duration,refreshRateCPA);

		for (iSample = 0; iSample < nSample ; iSample++)
		{
			x[iSample]   = iSample;
			yAvg[iSample] = GlobalAverage[iSample]/ iTrace;
		}

		for (iByte = 0; iByte < NBYTE ; iByte++)
		{
			bestguess[iByte] = 0;
		}

		printf("\nAvg and Var Array Init OK");

		scaleAvg = slope_xyscale_new_axis("Sample", "Amplitude", "Average");
		slope_scale_set_layout_rect(scaleAvg, 0, 0, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleAvg);

		scaleCorr = slope_xyscale_new_axis("Sample", "Correlation", "Correlation ");
		slope_scale_set_layout_rect(scaleCorr, 0, 1, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleCorr);

		seriesAvg = slope_xyseries_new_filled("Average", x, yAvg, nSample, "l-");
		slope_scale_add_item(scaleAvg, seriesAvg);

		seriesCorr = (SlopeItem **)malloc(sizeof(SlopeItem *)*3);

		seriesCorr[0] = slope_xyseries_new_filled("Wrong", x, maxCorrelation[currentByte], nSample, "la");
		slope_scale_add_item(scaleCorr, seriesCorr[0]);

		seriesCorr[1] = slope_xyseries_new_filled("Best", x, Correlation[currentByte][bestguess[currentByte]], nSample, "g-");
		slope_scale_add_item(scaleCorr, seriesCorr[1]);

		seriesCorr[2] = slope_xyseries_new_filled("Right", x, Correlation[currentByte][exKeyArray[currentByte]], nSample, "r-");
		slope_scale_add_item(scaleCorr, seriesCorr[2]);

		timeoutAES = g_timeout_add(duration, (GSourceFunc) aes_timer_callback, (gpointer) chart);

		printf("\nCPA window init done\n");
	}
	else
	{
		printf("\nError in mode!");
		exit(0);
	}

	gtk_widget_show_all(chart);
	gtk_widget_show_all(window);

	printf("\nPlease exit window screen to access the command line\n");
	//gtk_window_maximize(GTK_WINDOW(window));
	gtk_main();


	if(CPAMode)
	{
		g_free(x);
		g_free(yAvg);
		g_free(yVar);
	}
	else
	{
		g_free(x);
		g_free(y);
	}
}

/*
 * Rescale screen window to match with the dataset displayed
 */
static gboolean rescale(GtkWidget *button, gpointer data)
{
	printf("\nRescaling !");
	slope_scale_remove_item(scaleView,seriesView);
	seriesView = slope_xyseries_new_filled("DLYB State", x, y, nSample, "l-");
   	slope_scale_add_item(scaleView, seriesView);
	return TRUE;
}

/*
 * Launch calibration to enhance delay line precision
 */
static gboolean autocalibration(GtkWidget *button, gpointer data)
{
	printf("\nAuto-calibrating !");
	Auto_Find(1, 10,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
	printf("\nPlease exit the graph window on screen to access the command line\n");
	rescale(buttonRescale,NULL);
	return TRUE;
}

/*
 * Exit GTK screen
 */
static gboolean exitview(GtkWidget *button, gpointer data)
{
	printf("Exiting\n");

	if(updateend)
	{
		gtk_widget_destroy(window);
		gtk_widget_destroy(chart);
		gtk_main_quit();
		updateend = 0;
	}
	else
	{
		exitCallback = 1;
	}

	return TRUE;
}


/*
 * Update and display DLYB oscilloscope values
 */
static gboolean view_timer_callback(GtkWidget *widget)
{
	if (timeoutView != 0) {
	g_source_remove(timeoutView);
	}
	timeoutView = 0;

	clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
	for(iSample = 0 ; iSample < nSample ; iSample++)
	{
		if(iSample+shiftSpeed < nSample)
		{
			y[iSample] = y[iSample+shiftSpeed];
		}
		else
		{
			y[iSample] = bw_low_pass(lowfilter,Get_HW(currentDLval,currentCLKval));
		}
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
	duration = (uint32_t)((endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000000);

	if(exitCallback)
	{
		if (timeoutView != 0) {
		g_source_remove(timeoutView);
		}
		timeoutView = 0;
		exitCallback = 0;
		gtk_widget_destroy(window);
		gtk_widget_destroy(chart);
		gtk_main_quit();
	}
	else
	{
	timeoutView = g_timeout_add (duration+50, (GSourceFunc)view_timer_callback, (gpointer) chart);
	slope_chart_redraw(SLOPE_CHART(chart));
	}


	return timeoutView;
}

/*
 * Compute, update and display correlation results
 */
static gboolean cpaupdate(GtkWidget *button, gpointer data)
{
	printf("\nUpdating CPA Results for %d traces...",iTrace);
	printf("\nButton Pushed: %s",gtk_button_get_label(GTK_BUTTON(button)));
	int i = strcmp(gtk_button_get_label(GTK_BUTTON(button)),"Update");
	printf("\ni = %d",i);
	if(i == 0)
	{
		for (iSample = 0; iSample < nSample ; iSample++)
		{
			x[iSample]   = iSample;
			yAvg[iSample] = GlobalAverage[iSample]/ iTrace;
			yVar[iSample] = GlobalVariance[iSample] / iTrace;
			yVar[iSample] -= pow(yAvg[iSample],2);

			if(yVar[iSample]>0)
			{
				yVar[iSample] = sqrt(yVar[iSample]);
			}
		}

		for(iByte = minByte ; iByte < maxByte ; iByte++)
		{
			Correlation[iByte] = CorrelateClasses(yVar,yAvg,ClassAverage[iByte],ClassPopulation[iByte],NCLASS,256,iTrace,nSample,2,iByte,0,1);
			bestguess[iByte] = CPA_Results(Correlation[iByte],maxCorrelation,NCLASS,nSample,exKeyArray[iByte],iByte,NULL);	
		}

		if(verbose)
		{
			printf("\ncurrentByte = %d",currentByte);
			printf("\nClassAvg[0][0][0] = %f",ClassAverage[0][0][0]);
			printf("\nClassPop[0] = %d",ClassPopulation[0][0]);
			printf("\nyCorr[0][0] = %f",Correlation[0][0][0]);
		}
	}

	slope_scale_remove_item(scaleAvg, seriesAvg);
	sprintf(legendStr,"Average, nTrace: %d, byte: %d",iTrace,currentByte);
	seriesAvg = slope_xyseries_new_filled(legendStr, x, yAvg, nSample, "l-");
	slope_scale_add_item(scaleAvg, seriesAvg);

	slope_scale_remove_item(scaleCorr, seriesCorr[0]);
	seriesCorr[0] = slope_xyseries_new_filled("Wrong", x, maxCorrelation[currentByte], nSample, "la");
	slope_scale_add_item(scaleCorr, seriesCorr[0]);

	slope_scale_remove_item(scaleCorr, seriesCorr[1]);
	seriesCorr[1] = slope_xyseries_new_filled("Best", x, Correlation[currentByte][bestguess[currentByte]], nSample, "g-");
	slope_scale_add_item(scaleCorr, seriesCorr[1]);

	slope_scale_remove_item(scaleCorr, seriesCorr[2]);
	seriesCorr[2] = slope_xyseries_new_filled("Right", x, Correlation[currentByte][exKeyArray[currentByte]], nSample, "r-");
	slope_scale_add_item(scaleCorr, seriesCorr[2]);

	slope_chart_redraw(SLOPE_CHART(chart));

	printf("\nSuccess!\n");

	return TRUE;
}

/*
 * Select Byte correlation to display
 */
static gboolean byteselect(GtkWidget *button, gpointer data)
{
	currentByte++;

	if(currentByte == maxByte)
	{
		currentByte = 0;
	}

	printf("\nCurrent Byte is  %d",currentByte);

	cpaupdate(buttonByte,NULL);
	return TRUE;
}


static gboolean aes_timer_callback(GtkWidget *widget)
{
	FILE *fptr;

	if (timeoutAES != 0) {
	g_source_remove(timeoutAES);
	}
	timeoutAES = 0;


	clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);

	if(AESMode)
	{
		for(iTry = 0 ;  iTry < refreshRateAES ;  iTry++)
		{
			 if(Launch_AES()==0){break;}
		}
	}
	else
	{
		for(iTry = 0 ;  iTry < refreshRateCPA ;  iTry++)
		{
			 if(Launch_AES()==0){break;}
		}
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

	duration = (uint32_t)((endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000000);

	if(AESMode)
	{	
		printf("\nHot: %d, Cold: %d, Discard: %d", counterHOT,counterCOLD,counterDiscard);
	}

	//relaunch condition
	if(iTrace < nTrace)
	{
		if(AESMode)
		{
			printf("\nDuration: %d for %d traces per timeout\n", duration,refreshRateAES);

			for(iSample = 0 ; iSample < nSample ; iSample++)
			{
				y[iSample] = GlobalAverage[iSample]/iTrace;
			}

			sprintf(legendStr,"AES acquisition, nTrace: %d",iTrace);
			slope_scale_remove_item(scaleAvg,seriesAvg);
			seriesAvg = slope_xyseries_new_filled(legendStr, x, y, nSample, "l-");
			slope_scale_add_item(scaleAvg, seriesAvg);
			slope_chart_redraw(SLOPE_CHART(chart));
		}
		/*else
		{
			printf("\nDuration: %d for %d traces per timeout\n", duration,refreshRateCPA);
		}*/

		if(!exitCallback)
		{
			if(AESMode)
			{
				timeoutAES = g_timeout_add (duration, (GSourceFunc)aes_timer_callback, (gpointer) chart);
			}
			else
			{
				timeoutAES = g_timeout_add (100, (GSourceFunc)aes_timer_callback, (gpointer) chart);
			}
		}
	}
	else
	{
		updateend = 1;

		if(AESMode)
		{
			for(iSample = 0 ; iSample < nSample ; iSample++)
			{
				y[iSample] = GlobalAverage[iSample]/iTrace;
			}

			sprintf(legendStr,"AES acquisition, nTrace: %d",iTrace);
			slope_scale_remove_item(scaleAvg,seriesAvg);
			seriesAvg = slope_xyseries_new_filled(legendStr, x, y, nSample, "l-");
			slope_scale_add_item(scaleAvg, seriesAvg);
			slope_chart_redraw(SLOPE_CHART(chart));
		}
		else
		{
			printf("\nUpdating CPA Window...");
			cpaupdate(buttonUpdate,NULL);

			if(saveCPA)
			{
				printf("\nSaving Correlation Results...");
				fptr = fopen("CPAresults.log","w");
				for(iByte = 0 ; iByte < NBYTE ; iByte++)
				{
					Correlation[iByte] = CorrelateClasses(yVar,yAvg,ClassAverage[iByte],ClassPopulation[iByte],NCLASS,256,iTrace,nSample,2,iByte,0,1);
					bestguess[iByte] = CPA_Results(Correlation[iByte],maxCorrelation,NCLASS,nSample,exKeyArray[iByte],iByte,fptr);
				}
				fclose(fptr);
			}

		}
		printf("\n\nEnd!");
		printf("\nCounterHOT: %d",counterHOT);
		printf("\nCounterCOLD: %d",counterCOLD);
		printf("\nCounterDISCARD: %d",counterDiscard);
		printf("\n\nPlease exit the graph window on screen to access the command line\n");
	}

	if(exitCallback)
	{
		if (timeoutAES != 0) {
		g_source_remove(timeoutAES);
		}
		timeoutAES = 0;
		exitCallback = 0;
		gtk_widget_destroy(window);
		gtk_widget_destroy(chart);
		gtk_main_quit();
	}

	return timeoutAES;
}

void Print_AES_Type(uint8_t aesType)
{
	switch(aesType)
	{
		case OPENSSL:
			printf("\nOpenSSL AES");
			break;
		case TINYAES:
			printf("\nTiny AES");
			break;
		case MASKEDAES:
			printf("\nMasked AES");
			break;
		case HIGHERORDER:
			printf("\nHigher order masked AES");
			break;
		case ANSSIAES:
			printf("\nANSSI 1st order masked AES");
			break;
		default:
			break;
	}
}





