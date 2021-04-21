/**
  ******************************************************************************
  * @file           : calibration.c
  * @brief          : An automated way to turn a DLYB block into a voltage sensor
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

#include "calibration.h"
#include "main_cortex_A7.h"

double Measure_Variability(uint8_t dlval, uint16_t clkval)
{
	double mean = 0.0;
	double var = 0.0;
	double globalMean = 0.0;
	double globalVar = 0.0;
	unsigned int nSample = 4000;
	unsigned int nTrace = 100;
	unsigned int count = 0;
	int lngArray[nSample];

	//Modify SDMMC2 frequency to max
	Modify_Register(SDMMC_CLK,clkval,0x2ff);

	globalMean = 0.0;
	globalVar = 0.0;

	for(int iTrace = 0 ; iTrace < nTrace ; iTrace++)
	{
		var = 0.0;
		mean = 0.0;

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (dlval << 8));

		//Start the acquisition of the delay-line
		for(int iSample = 0 ; iSample < nSample ; iSample ++)
		{
			lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);
		}

		// compute mean
		for(int iSample = 0 ; iSample < nSample; iSample++)
		{
			count = 0;
			for(int i = 0 ; i < DLYB_LENGTH ; i++)
			{
				count += (lngArray[iSample] >> (16+i)) & 1;
			}

			mean += (double)count;
		}

		mean /= (double)nSample;
		globalMean += mean;

		//printf("\n\rMean: %f",mean);

		// compute variance
		for(int iSample = 0 ; iSample < nSample; iSample++)
		{
			count = 0;
			for(int i = 0 ; i < DLYB_LENGTH ; i++)
			{
				count += (lngArray[iSample] >> (16+i)) & 1;
			}

			var += pow((double)count - mean,2);
		}

		var /= (double)(nSample-1);
		globalVar += var;


		//printf("\n\rVariance: %f",var);

		/*end transaction*/
		Write_Register(DLYB_CR,0x1);

	}

	globalVar /= nTrace;

	//printf("\n\r    Global Mean: %f",globalMean);

	return globalVar;
};




void Auto_Find(uint8_t nTmin, uint8_t nTmax, uint8_t * odlval,uint8_t * oclkval,uint8_t * ominHW,uint8_t * omaxHW,double * oNTransition,double * oVar)
{
	uint32_t count = 0;
	double var = 0.;
	double mean = 0.;
	uint16_t nTest = 5;
	uint8_t iFound = 0;
	uint32_t nSample = 500;
	uint16_t nData = 100;

	uint32_t lngArray[nSample];

	double bestvar[nTest];
	uint8_t bestdl[nTest];
	uint8_t bestclk[nTest];
	uint32_t bestval[nTest][nData];

	double tempvar[nTest];
	uint8_t tempclk[nTest];
	uint8_t tempdl[nTest];
	uint32_t tempbestval[nTest][nData];

	double nTransition = 0.;
	double nOnes = 0.;
	double maxOnes = 10.;//12.;//3.;
	double minOnes = 2.;
	uint8_t minHW = 0xff;
	uint8_t tempHW = 0;
	uint8_t maxHW =0;
	uint8_t orientation = 0;

	uint8_t clkmin = 0;
	uint8_t clkmax = 7;
	uint8_t dlmin = 0;
	uint8_t dlmax = 128;

	printf("\n\r *** Autocalibration ***\n\r");
	printf("Starting at dlval = %d, ending at dlval = %d\n\r",dlmin,dlmax);
	printf("Starting at clkval = %d, ending at clkval = %d\n\r",clkmin,clkmax);
	printf("Transitions starting at = %d, ending at %d\n\r",nTmin,nTmax);

	for(int iTest = 0 ; iTest < nTest ; iTest++)
	{
		bestvar[iTest]= 0;
		bestdl[iTest]= 0;
		bestclk[iTest]= 0;


		tempvar[iTest]= 0;
		tempclk[iTest]= 0;
		tempdl[iTest]= 0;

		for(int iData = 0 ; iData < nData ; iData++)
		{
			bestval[iTest][iData] = 0;
			tempbestval[iTest][iData] = 0;
		}
	}

	for(uint8_t clkval = clkmin ; clkval < clkmax ; clkval++)
	{
		//Modify SDMMC2 frequency to max
		Modify_Register(SDMMC_CLK,clkval,0x2ff);

		for(uint8_t dlval = dlmin ; dlval < dlmax ; dlval++)
		{
			orientation = 0;
			mean = 0.;
			var = 0.;

			//Disable the length sampling by setting SEN bit to ‘0’.
			Write_Register(DLYB_CR,0x1);

			//Enable the length sampling by setting SEN bit to ‘1’.
			Write_Register(DLYB_CR,0x3);

			//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval  and re-launch LENGTH SAMPLING
			Write_Register(DLYB_CFGR, 0xc + (dlval << 8));


			//Start the acquisition of the delay-line
			for(int iSample = 0 ; iSample < nSample ; iSample ++)
			{
				lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);

			}

			// Compute Mean (Hamming weight)
			for(int iSample = 0 ; iSample < nSample; iSample++)
			{
				count = 0;
				for(int i = 0 ; i < DLYB_LENGTH ; i++)
				{
					count += (lngArray[iSample] >> (16+i)) & 1;
				}
				//printf("count: %d\n\r",count);
				mean += count;

			}


			mean /= (double)nSample;


			// compute variance
			for(int iSample = 0 ; iSample < nSample; iSample++)
			{
				count = 0;
				for(int i = 0 ; i < DLYB_LENGTH ; i++)
				{
					count += (lngArray[iSample] >> (16+i)) & 1;
				}

				var += pow((double)count - mean,2);
			}

			var /= (double)(nSample-1);

			nTransition = 0; //reset transition counter for test

			//count transition
			for(int j = 0 ; j < 10 ; j++)
			{
				nTransition += Count_Transitions((lngArray[50+j] >> 16) & 0xFFF,DLYB_LENGTH);
				nOnes += Count_Ones((lngArray[50+j] >> 16) & 0xFFF,DLYB_LENGTH);
			}

			nTransition/=10;
			nOnes /= 10;

			if(((lngArray[50] >> 16) & 0xFFF)>>11 == 1)
			{
				orientation = 1;
			}

			if((nTransition >= nTmin) && (nTransition <= nTmax) && (orientation == 1) && (nOnes <= maxOnes) && (nOnes >= minOnes)) //if transition count is ok, check var
			{
				for(int i = 0 ; i < nTest ; i++)
				{

					if(var > bestvar[i]) //check if var is superior to current bestvar
					{
						iFound++;
						printf("\n\rNew var found, at %d place, clkval: %d, dlval: %d, var: %f!",i,clkval,dlval,var);

						for(int j = 0 ; j < nTest ; j++)
						{
							//printf("\n\ri+j+1: %d",i+j+1);
							if(j+i+1 == nTest)
							{
								break;
							}

							tempvar[j+i] = bestvar[j+i];
							tempclk[j+i] = bestclk[j+i];
							tempdl[j+i] = bestdl[j+i];

							//bestdl[j+i] = temp;

							for(int k = 0 ; k < nData ; k++)
							{
								tempbestval[i+j+1][k] = bestval[i+j][k];
							}
						}

						//save DLYB values for this setup (to print them after)
						bestvar[i] = var;
						bestclk[i] = clkval;
						bestdl[i] = dlval;
						for(int k = 0 ; k < nData ; k++)
						{
							bestval[i][k] = lngArray[50+k];
						}

						for(int j = 0 ; j < nTest ; j++)
						{
							if(j+i+1 == nTest)
							{
								break;
							}

							bestvar[j+i+1] = tempvar[j+i];
							bestclk[j+i+1] = tempclk[j+i];
							bestdl[j+i+1] = tempdl[j+i];

							//bestdl[j+i] = temp;

							for(int k = 0 ; k < nData ; k++)
							{
								bestval[i+j+1][k] = tempbestval[i+j][k];
							}
						}


						/*for(int j = 0 ; j < nTest ; j++)
						{
							printf("\n\rbestvar[%d]=%f",j,bestvar[j]);
						}*/

						break;
					}


				}
			}

			Write_Register(DLYB_CR,0x1);
		}


	}

	printf("\n\r");

	for(int i = 0 ; i < 5 ; i++)
	{
		nTransition=0;
		nOnes=0;
		minHW = 0xff;
		maxHW = 0;

		printf("\n\rConfig %d: ",i);

		printf("\n\rvariance: %f",bestvar[i]);
		printf("\n\rdlval: %d",bestdl[i]);
		printf("\n\rclkval: %d",bestclk[i]);
		printf("\n\rreg:");

		for(int j = 0 ; j < nData ; j++)
		{
			if(j < 10)
			{
			printf("\n\r");
			printBits(DLYB_LENGTH, (bestval[i][j] >> 16) & 0xFFF);
			}
			nTransition += Count_Transitions((bestval[i][j] >> 16) & 0xFFF,DLYB_LENGTH);
			tempHW = Count_Ones((bestval[i][j] >> 16) & 0xFFF,DLYB_LENGTH);

			if(tempHW > maxHW)
			{
				maxHW = tempHW;
			}

			if(tempHW < minHW)
			{
				minHW = tempHW;
			}

			nOnes += tempHW;
		}

		nTransition/=nData;
		nOnes/=nData;

		printf("\n\rnOnes: %f",nOnes);
		printf("\n\rminHW: %d",minHW);
		printf("\n\rmaxHW: %d",maxHW);
		printf("\n\rnTransition: %f",nTransition);
		printf("\n\r");

		if(i == 0)
		{
			*odlval = bestdl[i];
			*oclkval = bestclk[i];
			*ominHW = minHW;
			*omaxHW = maxHW;
			*oNTransition = nTransition;
			*oVar = bestvar[i];
		}
	}

	printf("\n\r*******************");
	printf("\n\rAuto configuration gave: \n\rdlval: %d\n\rclkval: %d\n\rVariance: %f\n\rminHW: %d\n\rmaxHW: %d\n\rnTransition: %f\n\r",*odlval,*oclkval,*oVar,*ominHW,*omaxHW,*oNTransition);
	Print_DL_State(10,*odlval,*oclkval);
	printf("*******************\n\r");
}


double Get_Mean_HW(uint32_t sMean,uint8_t dlval,uint8_t clkval)
{
		uint32_t count = 0;
		double mean = 0.0;
		int lngArray[sMean];

		//Modify SDMMC2 frequency to clkval
		Modify_Register(SDMMC_CLK,clkval,0x2ff);

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		while(((Read_Register(DLYB_CR)>>1)&0x1)!=0){}

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		while(((Read_Register(DLYB_CR)>>1)&0x1)==0){}

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (dlval << 8));

		//Start the acquisition of the delay-line
		for(int iSample = 0 ; iSample < sMean ; iSample ++)
		{
			lngArray[iSample] = Read_Register(DLYB_CFGR);
		}

		// Compute Mean (Hamming weight)
		for(int iSample = 0 ; iSample < sMean; iSample++)
		{
			count = 0;
			for(int i = 0 ; i < DLYB_LENGTH ; i++)
			{
				count += (lngArray[iSample] >> (16+i)) & 1;
			}

			mean += count;
		}


		return mean/sMean;
}

uint32_t Get_HW(uint8_t dlval,uint8_t clkval)
{
	uint32_t count = 0;
	uint32_t temp;

	//Modify SDMMC2 frequency to clkval
	Modify_Register(SDMMC_CLK,clkval,0x2ff);

	//Disable the length sampling by setting SEN bit to ‘0’.
	Write_Register(DLYB_CR,0x1);

	//Enable the length sampling by setting SEN bit to ‘1’.
	Write_Register(DLYB_CR,0x3);

	//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
	Write_Register(DLYB_CFGR,0xc + (dlval << 8));

	 *(volatile uint32_t *)(DLYB_CFGR);
	 *(volatile uint32_t *)(DLYB_CFGR);
	temp =  *(volatile uint32_t *)(DLYB_CFGR);

	// Compute Mean (Hamming weight)
	for(int i = 0 ; i < DLYB_LENGTH ; i++)
	{
		count += (temp >> (16+i)) & 1;
	}

	return count;
}

void Print_DL_State(uint32_t nSample,uint8_t dlval,uint8_t clkval)
{
		int lngArray[nSample];

		//Modify SDMMC2 frequency to clkval
		Modify_Register(SDMMC_CLK,clkval,0x2ff);

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (dlval << 8));

		//Start the acquisition of the delay-line
		printf("\n\rDLYB Binary State:\n\r");
		for(int iSample = 0 ; iSample < nSample ; iSample ++)
		{
			lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);
			printBits(DLYB_LENGTH,(lngArray[iSample] >> 16) & 0xFFF);
			printf("\n\r");
		}
		Write_Register(DLYB_CR,0x1);
}

void Find_Clock_Delay_Pair(uint8_t dlmin, uint8_t dlmax,uint8_t clkmin, uint8_t clkmax,uint32_t val)
{

	int nSample = 100;
	int lngArray[nSample];

	uint32_t reg =0;
	uint32_t counter = 0;
	uint8_t found = 0;
	double var = 0.;
	int bestclk = 0;
	int bestdl = 0;
	double bestvar = 0.;


	printf("Starting at dlval = %d, ending at dlval = %d\n\r",dlmin,dlmax);
	printf("Starting at clkval = %d, ending at clkval = %d\n\r",clkmin,clkmax);

	printf("Looking for a clock delay pair that match config\n\r");
	printf("The config analyzed is: ");
    printBits(DLYB_LENGTH,val);
    printf("\n\r\n\r");

	for(uint8_t clkval = clkmin ; clkval < clkmax ; clkval++)
	{
		//Modify SDMMC2 frequency to max
		Modify_Register(SDMMC_CLK,clkval,0x2ff);

		for(uint8_t dlval = dlmin ; dlval < dlmax ; dlval++)
		{
			//Disable the length sampling by setting SEN bit to ‘0’.
			Write_Register(DLYB_CR,0x1);

			//Enable the length sampling by setting SEN bit to ‘1’.
			Write_Register(DLYB_CR,0x3);

			//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval  and re-launch LENGTH SAMPLING
			Write_Register(DLYB_CFGR, 0xc + (dlval << 8));

			//Start the acquisition of the delay-line
			for(int iSample = 0 ; iSample < nSample ; iSample ++)
			{
				lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);
			}

			counter = 0;
			found = 0;

			for(int iSample = 0 ; iSample < nSample ; iSample ++)
			{
				reg = (lngArray[iSample] >> 16) & 0xFFF;

				if(reg == val)
				{
					counter++;
					found = 1;
				}
			}

			if(found == 1)
			{
				printf("Potential Configuration Found!\n\r");
				printf("Clock : %d\n\r",clkval);
				printf("Delay : %d\n\r",dlval);
				printf("Ratio: %d/%d",counter,nSample);

				printf("   Checking Variance...\n\r");
				var = Measure_Variability(dlval, clkval);
				printf("Var: %f\n\r",var);
				if(var > bestvar)
				{
					bestvar = var;
					bestclk = clkval;
					bestdl = dlval;
				}
			}

			Write_Register(DLYB_CR,0x1);
		}
	}

	if(bestvar == 0)
	{
		printf("No configuration found for this value\n\r");
	}
	else
	{
		printf("\n\rA Configuration that matches requirements has been found\n\r");
		printf("Clock : %d\n\r",bestclk);
		printf("Delay : %d\n\r",bestdl);
		printf("Variance: %f\n\r",bestvar);
	}

}



