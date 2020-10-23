#define _GNU_SOURCE
#include "main_cortex_A7.h"


//Correlation Power Analysis Variables
double * GlobalVariance;
double * GlobalAverage;
double ** ClassAverage;
double ** Correlation;
uint32_t * ClassPopulation;
uint8_t bnum_selected = 14;
uint8_t localCPA = 0;
uint8_t eFilter = 0;
BWHighPass* filter;

//DLYB calibration variables
uint8_t currentDLval = 41;
uint8_t currentCLKval = 0;
double currentVar = 0;
uint8_t currentMinHW = 0;
uint8_t currentMaxHW = 0;
double currentnTransition = 0;

//Register virtual addresses
int map_file = 0;
uint32_t DLYB_CFGR = 0;
uint32_t DLYB_CR = 0;
uint32_t SDMMC_CLK = 0;
uint32_t v_base_addr = 0;
uint32_t v_ddr_base_addr = 0;

/*static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Hello World\n");
}


static void
activate (GtkApplication *app,
          gpointer        user_data)
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button[4];
    GtkWidget *progress_bar;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 200);
    gtk_window_set_title(GTK_WINDOW(window), "Les GtkTable");
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid =  gtk_grid_new ();


    button[0]= gtk_button_new_with_label("Bouton 1");
    button[1]= gtk_button_new_with_label("Bouton 2");
    button[2]= gtk_button_new_with_label("Bouton 3");
    button[3]= gtk_button_new_with_label("Bouton 4");


    gtk_grid_attach(GTK_GRID(grid), button[0],0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button[1],0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button[2],1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button[3],1, 1, 1, 1);


    progress_bar = gtk_progress_bar_new ();
    gtk_grid_attach_next_to (GTK_GRID (grid),
                               progress_bar,
                               button[3],
                               GTK_POS_RIGHT, 1, 1);

    //add callback on button[3]
    //g_signal_connect (GTK_BUTTON (button[3]), "clicked",
    //                  G_CALLBACK (on_button_click), progress_bar);

    gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (grid));

    gtk_widget_show_all(window);

  GtkWidget *window;
  GtkWidget *viewButton;
  GtkWidget *varButton;
  GtkWidget *autoButton;
  GtkWidget *viewButton_box;


  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 300);

  viewButton_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (window), viewButton_box);

  viewButton = gtk_button_new_with_label ("View");
  g_signal_connect (viewButton, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect_swapped (viewButton, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add (GTK_CONTAINER (viewButton_box), viewButton);

  //varButton_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  //gtk_container_add (GTK_CONTAINER (window), varButton_box);

  varButton = gtk_button_new_with_label ("Var");
  g_signal_connect (varButton, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect_swapped (varButton, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add (GTK_CONTAINER (viewButton_box), varButton);

  //autoButton_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  //gtk_container_add (GTK_CONTAINER (window), autoButton_box);

  autoButton = gtk_button_new_with_label ("Auto");
  g_signal_connect (autoButton, "clicked", G_CALLBACK (GTK_Auto), autoButton);
  //g_signal_connect_swapped (autoButton, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add (GTK_CONTAINER (viewButton_box), autoButton);

  gtk_widget_show_all (window);
}*/

/*
 * Main function
 */
int main(int argc, char *argv[]) {

	//GtkApplication *app;
	//int status;

	double variance = 0.;
	double mean = 0.;

	uint32_t iTrace = 0;
	uint32_t nSample = 0;
	uint32_t nTrace = 0;
	uint32_t regval = 0;


	char * filename = "cortex-M4.elf";
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
    Map_Registers(&map_file,(void **) &core, DRAM_BASE_ADRAM, 0x2fff);
    v_ddr_base_addr = &core->control;
    printf("v_ddr_base_addr = %08x\n\r",v_base_addr);

    Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

    DLYB_CR = v_base_addr + 0x1000;
    DLYB_CFGR = v_base_addr + 0x1000 + 0x4;
    SDMMC_CLK = v_base_addr + 0x4;

    printf("virtual DLYB_CR address: %08x\n\r",DLYB_CR);
    printf("virtual DLYB_CFGR address: %08x\n\r",DLYB_CFGR);
    printf("virtual SDMMC_CLK address: %08x\n\r",SDMMC_CLK);

    //Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);

	/* Print Hello Banner */
	printf("\n\r\n\r");
	printf(
	"   _____    __    ___                   \n\r"
	"  / __(_)__/ /__ / (_)__  ___           \n\r"
	" _\\ \\/ / _  / -_) / / _ \\/ -_)	     \n\r"
	"/___/_/\\_,_/\\__/_/_/_//_/\\__/  on AES\n\r");

	Command_Helper();

	//gtk
	//app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	//g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	//status = g_application_run (G_APPLICATION (app), argc, argv);
	//g_object_unref (app);

	do{

	printf("SideLine>");
	fgets(command,50,stdin);
	command[strcspn(command,"\n")] = '\0';
	user_input = strtok(command," ");

		if(user_input == NULL){
			//do nothing
		}
		else if((strcmp(user_input,"get")==0) || (strcmp(user_input,"GET")==0))
		{
			printf("Get Mode:\n\r");
			printf("the current dlval is %d\n\r",currentDLval);
			printf("the current clkval is %d\n\r",currentCLKval);
		}
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
		else if((strcmp(user_input,"view")==0) || (strcmp(user_input,"VIEW")==0))
		{
			printf("View Mode:\n\r");

			//number of DLYB value to print (default 10)
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?10:atoi(user_input);
			printf("nSample: %d\n\r",nSample);
			printf("dlval: %d\n\r",currentDLval);
			printf("clkval: %d\n\r",currentCLKval);
			Print_DL_State(nSample,currentDLval,currentCLKval);
			printf("\n\r");
		}
		else if((strcmp(user_input,"var")==0) || (strcmp(user_input,"VAR")==0))
		{
			printf("Measure Variability Mode:\n\r");

			printf("dlval: %d\n\r",currentDLval);
			printf("clkval: %d\n\r",currentCLKval);
			variance = Measure_Variability(currentDLval, currentCLKval);
			Print_DL_State(3,currentDLval,currentCLKval);
			printf("Computed Variance: %f\n\r",variance);
		}
		else if((strcmp(user_input,"find")==0) || (strcmp(user_input,"FIND")==0))
		{
			//Searched value (default 3640 for 0b111000111000)
			user_input = strtok(NULL," ");
			regval = (user_input == NULL)?3640:atoi(user_input);

			Find_Clock_Delay_Pair(0,127,0,3,regval);
		}
		else if((strcmp(user_input,"auto")==0) || (strcmp(user_input,"AUTO")==0))
		{
			printf("Auto configuration Mode:\n\r");
			user_input = strtok(NULL," ");
			Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
			printf("\n\r*******************");
			printf("\n\rAuto configuration gave: \n\rdlval: %d\n\rclkval: %d\n\rVariance: %f\n\rminHW: %d\n\rmaxHW: %d\n\rnTransition: %f\n\r",currentDLval,currentCLKval,currentVar,currentMinHW,currentMaxHW,currentnTransition);
			Print_DL_State(10,currentDLval,currentCLKval);
			printf("*******************\n\r");
		}
		else if((strcmp(user_input,"help")==0) || (strcmp(user_input,"?")==0) || (strcmp(user_input,"man")==0))
		{
			Command_Helper();
		}
		else if((strcmp(user_input,"control")==0) || (strcmp(user_input,"CONTROL")==0))
		{
			while(1)
			{
				mean = Get_Mean_HW(100,currentDLval,currentCLKval);

				if(mean >= currentMaxHW-0.2)
				{
					printf("Too hot ! - mean : %f\n\r",mean);
					usleep(10000);
				}
				else if((mean > currentMinHW+0.2) && (mean < currentMaxHW-0.2))
				{
					printf("ok\n\r");
				}
				else
				{
					printf("Too cold ! - mean : %f\n\r",mean);
					Increase_Temperature(5000);
				}
			}
		}
		else if((strcmp(user_input,"aescustom")==0) || (strcmp(user_input,"AESCUSTOM")==0))
		{
			uint8_t custom_dl = 41;
			uint8_t custom_clk = 0;
			double custom_max = 2.5;
			double custom_min = 1.5;

			uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};

			iTrace = 0;

			//Reset communication with CM4
			Write_Register(v_ddr_base_addr,0x0);

			// Number of DLYB samples to acquire per trace (default 400)
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?400:atoi(user_input);
			printf("nSample: %d\n\r",nSample);

			// Number of trace to acquire (default 100)
			user_input = strtok(NULL," ");
			nTrace = (user_input == NULL)?100:atoi(user_input);
			printf("nTrace: %d\n\r",nTrace);

			//Send DMA parameters to CM4
			Write_Register(v_ddr_base_addr+0x30,nSample);

			// Print key used by CM4 for the example
			printf("\n\rkey : ");
			for(int k_i = 0 ; k_i < 16 ; k_i++){printf("%02x",exKeyArray[k_i]);}
			printf("\n\r");

			//Modify SDMMC2 frequency to clkval
			Modify_Register(SDMMC_CLK,currentCLKval,0x2ff);

			while(1)
			{
				mean = Get_Mean_Custom(100,custom_dl,custom_clk);

				if(mean >= custom_max)
				{
					printf("Too hot ! - mean : %f\n\r",mean);
					usleep(10000);
				}
				else if((mean > custom_min) && (mean < custom_max))
				{
					printf("attack\n\r");
					AES_DMA_Attack_Custom(currentDLval,nSample);
					iTrace++;
				}
				else
				{
					printf("\n\rToo cold ! - mean : %f",mean);
					Increase_Temperature(1000);
				}

				if(iTrace == nTrace)
				{
					printf("\n\rEnd acquisition!\n\r");
					break;
				}
			}

		}
		else if((strcmp(user_input,"aes")==0) || (strcmp(user_input,"AES")==0))
		{
			uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};
			uint32_t counterTemp = 0;

			iTrace = 0;
			Write_Register(v_ddr_base_addr,0x0); //Reset communication with CM4

			// Number of DLYB samples to acquire per trace (default 400)
			user_input = strtok(NULL," ");
			nSample = (user_input == NULL)?100:atoi(user_input);
			printf("\n\rnSample: %d",nSample);

			// Number of trace to acquire (default 100)
			user_input = strtok(NULL," ");
			nTrace = (user_input == NULL)?10:atoi(user_input);
			printf("\n\rnTrace: %d",nTrace);

			// Enable CPA calculation (default 1 enable)
			user_input = strtok(NULL," ");
			localCPA = (user_input == NULL)?1:atoi(user_input);
			printf("\n\rnlocalCPA: %d",localCPA);

			// Enable Butterworth Filter (default 1 enable)
			user_input = strtok(NULL," ");
			eFilter = (user_input == NULL)?1:atoi(user_input);
			printf("\n\rFilter: %d",eFilter);

			// Print info
			printf("\n\rDLval: %d",currentDLval);
			printf("\n\rCLKval: %d",currentCLKval);
			printf("\n\rMinHW: %d",currentMinHW);
			printf("\n\rMaxHW: %d",currentMaxHW);
			printf("\n\rnTransition: %f",currentnTransition);

			// Init Local CPA
			if( localCPA )
			{
				GlobalVariance = (double*)malloc(sizeof(double)*nSample);
				GlobalAverage = (double*)malloc(sizeof(double)*nSample);

				for(int iSample = 0 ; iSample < nSample ; iSample++)
				{
					GlobalVariance[iSample] = 0.;
					GlobalAverage[iSample] = 0.;
				}

				ClassPopulation = (uint32_t*)malloc(sizeof(uint32_t)*NCLASS);

				for(int iClass = 0 ; iClass < NCLASS ; iClass++)
				{
					ClassPopulation[iClass] = 0;
				}

				ClassAverage = (double**)malloc(sizeof(double*)*NCLASS);
				Correlation = (double**)malloc(sizeof(double*)*NCLASS);

				for(int iClass = 0 ; iClass < NCLASS ; iClass++)
				{
					ClassAverage[iClass] = (double*)malloc(sizeof(double)*nSample);
					Correlation[iClass] = (double*)malloc(sizeof(double)*nSample);

					for(int iSample = 0 ; iSample < nSample ; iSample++)
					{
						ClassAverage[iClass][iSample] = 0.;
						Correlation[iClass][iSample] = 0.;
					}
				}

			}

			// Init Filter
			if(eFilter)
			{
				filter = create_bw_high_pass_filter(4, 16000, 900);
			}

			//Send DMA parameters to CM4
			Write_Register(v_ddr_base_addr+0x30,nSample);

			// Print key used by CM4 for the example
			printf("\n\rkey : ");
			for(int k_i = 0 ; k_i < 16 ; k_i++){printf("%02x",exKeyArray[k_i]);}
			printf("\n\r");

			//Modify SDMMC2 frequency to clkval
			Modify_Register(SDMMC_CLK,currentCLKval,0x2ff);

			sleep(1);

			while(1)
			{
				mean = Get_Mean_HW(100,currentDLval,currentCLKval);

				if(mean >= currentMaxHW-0.1)
				{
					//printf("Too hot ! - mean : %f\n\r",mean);
					//printf("\n\rToo hot, countertemp: %d\n\r",counterTemp);
					usleep(10000);
					counterTemp++;
				}
				else if((mean > currentMinHW+0.1) && (mean < currentMaxHW-0.1))
				{
					//printf("ok ! - mean : %f\n\r",mean);
					AES_DMA_attack_Auto(nSample);
					iTrace++;

					if(counterTemp > 0)
					{
						counterTemp--;
					}
				}
				else
				{
					//printf("\n\rcountertemp: %d, mean = %f, currentMinHW = %d, currentMaxHW = %d",counterTemp,mean,currentMinHW,currentMaxHW);
					//printf("\n\rToo cold, countertemp: %d\n\r",counterTemp);
					Increase_Temperature(500);
					counterTemp++;
				}

				if(iTrace%1000==0)
				{
					printf("Traces processed: %d/%d\n\r",iTrace,nTrace);
				}

				if(iTrace == nTrace)
				{
					printf("\n\rEnd acquisition!\n\r");
					break;
				}

				if(counterTemp == 10000)
				{
					Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
					counterTemp = 0;
				}
			}

			if(localCPA)
			{
				printf("Compute Correlation...");
				Correlation = CorrelateClasses(GlobalVariance,GlobalAverage,ClassAverage,ClassPopulation,NCLASS,256,nTrace,nSample,2,0,0,1);
				CPA_Results(Correlation,NCLASS,nSample,exKeyArray[bnum_selected]);


			}

			free(GlobalVariance);
			free(GlobalAverage);
			free(ClassAverage);
			free(ClassPopulation);
			free(Correlation);

		}
		else
		{
			printf("Unknown Command %s\n\r",user_input);
		}


	}
	while(1);


}

uint32_t AES_DMA_Attack_Custom(uint8_t dlval,uint32_t nSample)
{
		uint32_t ptArray[4];
		uint32_t ctArray[4];
		uint32_t iSample = 0;
		uint32_t dataArray[nSample];
		double mean = 0.;

		Write_Register(v_ddr_base_addr,0x1); //Re-init communication

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (dlval << 8));

		// Generate random plain text and print it
		for(int u = 0; u<4 ; u++){ptArray[u] = (uint32_t)rand();}

		Write_Register(v_ddr_base_addr+0x4,ptArray[0]); //send 32bit pt0 to CM4
		Write_Register(v_ddr_base_addr+0x8,ptArray[1]); //send 32bit pt1 to CM4
		Write_Register(v_ddr_base_addr+0xc,ptArray[2]); //send 32bit pt2 to CM4
		Write_Register(v_ddr_base_addr+0x10,ptArray[3]); //send 32bit pt3 to CM4

		while(Read_Register(v_ddr_base_addr) != 2){} // Wait for CM4 *ready to encrypt*

		//clock1 = clock();
		Write_Register(v_ddr_base_addr,0x3); //send *start* to CM4
		//usleep(200);
		while(Read_Register(v_ddr_base_addr) != 4){} //Wait for CM4 *end of encrypt*
		//clock2 = clock();

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//usleep(1);

		//ctArray[0] = Read_Register(v_ddr_base_addr+0x14); //read 32bit ct0 from CM4
		//ctArray[1] = Read_Register(v_ddr_base_addr+0x18); //read 32bit ct1 from CM4
		//ctArray[2] = Read_Register(v_ddr_base_addr+0x1c); //read 32bit ct2 from CM4
		//ctArray[3] = Read_Register(v_ddr_base_addr+0x20); //read 32bit ct3 from CM4
		for(int i = 0 ; i < nSample ; i++)
		{
			dataArray[i] = Read_Register(v_ddr_base_addr+0x40+i*4);
			mean += Decode_DelayLine_Custom(dataArray[i],dlval);
		}

		mean = mean/nSample;

		if((mean > 1.5) && (mean < 2.5))
		{
			printf("\n\rplaintext : ");
			for(int u = 0; u<4 ; u++){printf("%08x",ptArray[u]);}
			printf("\n\r");

			for(int i = 0 ; i < nSample ; i++)
			{
			printf("%c",Decode_DelayLine_Custom(dataArray[i],dlval)+50);
			}

			//printf("\n\rciphertext : ");
			//for(int u = 0; u<4 ; u++){printf("%08x",ctArray[u]);}
		}
		else
		{
			//printf("\n\rmean: %f",mean);
		}


		return 1;
}

uint32_t AES_DMA_attack_Auto(uint32_t nSample)
{
		uint8_t ptArray[16];
		uint8_t ctArray[16];
		uint32_t iSample = 0;
		uint32_t dataArray[nSample];
		float filteredDataArray[nSample];
		uint32_t count = 0;
		double mean = 0.;
		uint8_t bnum = bnum_selected;


		Write_Register(v_ddr_base_addr,0x1); //Re-init communication

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (currentDLval << 8));

		// Generate random plain text and print it
		for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();}

		for(int u = 0 ; u < 4 ; u++)
		{
			Write_Register(v_ddr_base_addr+0x4+u*4,((ptArray[u*4]<<24)&0xFF000000)|((ptArray[u*4+1]<<16)&0xFF0000)|((ptArray[u*4+2]<<8)&0xFF00)|((ptArray[u*4+3]<<0)&0xFF)); //send 32bit to CM4
		}

		while(Read_Register(v_ddr_base_addr) != 2){} // Wait for CM4 *ready to encrypt*

		//clock1 = clock();
		Write_Register(v_ddr_base_addr,0x3); //send *start* to CM4
		//usleep(200);
		while(Read_Register(v_ddr_base_addr) != 4){} //Wait for CM4 *end of encrypt*
		//clock2 = clock();

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//usleep(1);

		for(int u = 0 ; u < 4 ; u++)
		{
			ctArray[0+u*4] = (Read_Register(v_ddr_base_addr+0x14+u*4) >> 24) & 0xff;
			ctArray[1+u*4] = (Read_Register(v_ddr_base_addr+0x14+u*4) >> 16) & 0xff;
			ctArray[2+u*4] = (Read_Register(v_ddr_base_addr+0x14+u*4) >> 8) & 0xff;
			ctArray[3+u*4] = (Read_Register(v_ddr_base_addr+0x14+u*4) >> 0) & 0xff;
		}

		for(int iSample = 0 ; iSample  < nSample ; iSample ++)
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
			if(!localCPA)
			{
				printf("\n\rplaintext : ");
				for(int u = 0; u<16 ; u++){printf("%02x",ptArray[u]);}
				printf("\n\r");

				for(int iSample = 0 ; iSample < nSample ; iSample++)
				{
					printf("%c",dataArray[iSample]+50);
				}
			}
			else
			{

				/*printf("\n\rplaintext : ");
				for(int u = 0; u<16 ; u++){printf("%02x",ptArray[u]);}
				printf("\n\r");*/

				/*filter*/
				if(eFilter)
				{

					//printf("filtered value: \n\r");
					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						filteredDataArray[iSample] = bw_high_pass(filter,dataArray[iSample]);
						//printf("%f ",filteredDataArray[iSample]);
					}
					//printf("\n\r");

					/*profiling*/
					ClassPopulation[ptArray[bnum]] += 1;
					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						ClassAverage[ptArray[bnum]][iSample] += (double)filteredDataArray[iSample];
					}


					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						GlobalVariance[iSample] += pow((double)filteredDataArray[iSample],2);
						GlobalAverage[iSample] += (double)filteredDataArray[iSample];
					}

				}
				else
				{
					/*profiling*/
					ClassPopulation[ptArray[bnum]] += 1;
					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						ClassAverage[ptArray[bnum]][iSample] += (float)dataArray[iSample];
					}


					for(iSample = 0 ; iSample < nSample ; iSample++)
					{
						GlobalVariance[iSample] += (float)pow(dataArray[iSample],2);
						GlobalAverage[iSample] += (float)dataArray[iSample];
					}
				}

				/*printf("\n\rciphertext : ");
				for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}*/
			}
		}
		else
		{
			//printf("\n\rmean: %f",mean);
		}


		return 1;
}



uint32_t AES_attack_Custom(uint8_t dlval, uint32_t sumSample, uint32_t iTrace)
{
		uint32_t ptArray[4];
		uint32_t ctArray[4];
		uint32_t lngArray[100];
		uint32_t iSample = 0;
		double meanSample=0.;

		Write_Register(v_ddr_base_addr,0x1); //Re-init communication

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to DLYB_LENGTH and set UNIT to dlval and re-launch LENGTH SAMPLING
		Write_Register(DLYB_CFGR,0xc + (dlval << 8));

		// Generate random plain text and print it
		for(int u = 0; u<4 ; u++){ptArray[u] = (uint32_t)rand();}

		Write_Register(v_ddr_base_addr+0x4,ptArray[0]); //send 32bit pt0 to CM4
		Write_Register(v_ddr_base_addr+0x8,ptArray[1]); //send 32bit pt1 to CM4
		Write_Register(v_ddr_base_addr+0xc,ptArray[2]); //send 32bit pt2 to CM4
		Write_Register(v_ddr_base_addr+0x10,ptArray[3]); //send 32bit pt3 to CM4

		while(Read_Register(v_ddr_base_addr) != 2){} // Wait for CM4 *ready to encrypt*
		//clock1 = clock();

		Write_Register(v_ddr_base_addr,0x3); //send start to CM4


		while(Read_Register(v_ddr_base_addr) != 4){//Wait for CM4 *end of encrypt*

			lngArray[iSample++] = Read_Register(DLYB_CFGR);
		}
		//clock2 = clock();

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		usleep(1);

		ctArray[0] = Read_Register(v_ddr_base_addr+0x14); //read 32bit ct0 from CM4
		ctArray[1] = Read_Register(v_ddr_base_addr+0x18); //read 32bit ct1 from CM4
		ctArray[2] = Read_Register(v_ddr_base_addr+0x1c); //read 32bit ct2 from CM4
		ctArray[3] = Read_Register(v_ddr_base_addr+0x20); //read 32bit ct3 from CM4

		if(iTrace > 1000)
		{
			meanSample = sumSample / iTrace;
		}
		//printf("sumSample: %d\n\r",sumSample);

		if((iSample <= (int)meanSample+1) && (iSample >= (int)meanSample-1))
		{
			printf("\n\rplaintext : ");
			for(int u = 0; u<4 ; u++){printf("%08x",ptArray[u]);}
			printf("\n\r");
			for(int i = 0 ; i < iSample ; i++)
			{
				printf("%c",Decode_DelayLine_Custom(Read_Register(v_ddr_base_addr+0x40+i*4),dlval)+50);
			}

			printf("\n\rciphertext : ");
			for(int u = 0; u<4 ; u++){printf("%08x",ctArray[u]);}
			//printf("nb_cycles: %d",clock2-clock1);
		}

		return iSample;
}

double Get_Mean_Custom(uint32_t nSample,uint8_t dlval,uint8_t clkval)
{
		double mean = 0.0;
		int lngArray[nSample];

		//Modify SDMMC2 frequency to max
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
		for(int iSample = 0 ; iSample < nSample ; iSample ++)
		{
			lngArray[iSample] = Read_Register(DLYB_CFGR);
		}

		for(int iSample = 0 ; iSample < nSample ; iSample ++)
		{
		mean += Decode_DelayLine_Custom(lngArray[iSample],dlval);
		}

		return mean/nSample;
}



uint8_t Decode_DelayLine_Custom(uint32_t reg,uint8_t dlval)
{
	uint8_t temp = 0;

	if(dlval == 119)
	{
		for(uint8_t iNibble = 0 ; iNibble < 3 ; iNibble++)
		{
			switch((reg>>(16+iNibble*4))&0xF)
			{
				case 0x8:
					temp = temp + 9;
					break;
				case 0xc:
					temp = temp + 8;
					break;
				case 0xe:
					temp = temp + 7;
					break;
				case 0x4:
					temp = temp + 6;
					break;
				case 0x6:
					temp = temp + 5;
					break;
				case 0x7:
					temp = temp + 4;
					break;
				case 0x2:
					temp = temp + 3;
					break;
				case 0x3:
					temp = temp + 2;
					break;
				case 0x1:
					temp = temp + 1;
					break;
				case 0x0:
					temp = temp + 0;
					break;
				default:
					printf("error");
					break;
			}
		}
	}
	else if(dlval == 41)
	{

		for(uint8_t itdc = 0 ; itdc < 2 ; itdc++)
		{
			switch((reg>>(16+itdc*6))&0x3F)
			{
				case 0b111100:
					temp = temp + 2;
					break;
				case 0b111000:
					temp = temp + 1;
					break;
				case 0b110000:
					temp = temp + 0;
					break;
				default:
					printBits(6,(reg>>(16+itdc*6))&0x3F);
					printf("\n\r");
					break;
			}
		}
	}
	else
	{
		printf("DLvar error\n\r");
	}

	return temp;
}

void Command_Helper(void)
{
	printf("\n\rCommand Helper:");
	printf("\n\r-------------------------------------------------------------------------");
	printf("\n\r|    cmd    |              Parameters            |      Description     |");
	printf("\n\r|-----------------------------------------------------------------------|");
	printf("\n\r| set       | <dlValue> <clkValue>               | set dlval and clkval |");
	printf("\n\r| get       |                                    | print dlval clkval   |");
	printf("\n\r| view      | <nSample>                          | print DLYB state     |");
	printf("\n\r| var       |                                    | Compute variance     |");
	printf("\n\r| auto      | <regValue>                         | Find adequate value  |");
	printf("\n\r| control   |                                    | Control temperature  |");
	printf("\n\r| aes       | <nSample> <nTrace>                 | Launch AES attack    |");
	printf("\n\r-------------------------------------------------------------------------");
	printf("\n\r\n\rexample : \"view 10\" = print 10 times DLYB state \n\r\n\r");
}

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

/*void GTK_Auto(GtkWidget *widget,gpointer   data)
{
	gtk_widget_set_sensitive (widget, FALSE);
	Auto_Find(1, 1,&currentDLval,&currentCLKval,&currentMinHW,&currentMaxHW,&currentnTransition,&currentVar);
	gtk_widget_set_sensitive (widget, TRUE);
}*/
