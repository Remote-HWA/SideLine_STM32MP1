#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <aes_tiny.h>
#include <aes_openssl.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/sp.h>
#include <wolfssl/openssl/bn.h>
#include <math.h>
#include <wolfssl/options.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/perf_event.h>
#include <linux/types.h>
#include <linux/hw_breakpoint.h>
#include <asm/unistd.h>
#include <main_cortex-A7.h>

#define DLYB_BASE_ADDR 0x58007000 
//#define PERF_DWT_ADDR 0x500D1000

#define DDR_ADDR 0xD8000000

#define DEBUG_BASE_ADDR 0x500D0000
#define BSEC_BASE_ADDR 0x5C005000
#define PERF_STM32_ADDR 0xE0001000
#define GPIO_BSRR_OFFSET 0x18
#define UNIT_VAL 119
#define USEREN 0x1


float Measure_Variability(uint8_t dlval, uint16_t clkval);
float Force_Variability(uint8_t dlval, uint16_t clkval);
int Increase_Temperature(uint32_t attempts);
void Decrease_Temperature(uint32_t duration);
uint8_t Test_Variability(uint8_t dlmin, uint8_t dlmax,uint8_t clkmin, uint8_t clkmax);
void printBits(uint8_t size, uint32_t data);
float Get_Mean(uint32_t nSample,uint8_t dlval);
uint8_t Decode_DelayLine(uint32_t delayvalue);
int enable_counter = 0x80000000;
int map_file = 0;
int DLYB_CFGR = 0;
int DLYB_CR = 0;
int map_file2 = 0;
int custom_unit = 0;
uint32_t v_base_addr = 0;
uint32_t v_ddr_base_addr = 0;
/*uint32_t v_dwt_base_addr = 0;
uint32_t v_debug_dwt_base_addr = 0;
uint32_t v_bsec_base_addr = 0;
uint32_t v_perf_stm32_addr = 0;*/
int inst_fd = 0;
long long count0 = 0;
long long count1 = 0;
int err = 0;
//pthread_t thread;

int perf_event_open(struct perf_event_attr *hw_event,pid_t pid, int cpu, int group_fd,unsigned long flags)
{
	int ret;

	ret = syscall(__NR_perf_event_open,hw_event,pid, cpu, group_fd, flags);

	return ret;
}

/*void* WriteDLYBRegister(void *arg)
{
    //for(int i = 0 ; i < 10000 ; i++)
    //{
    *(volatile uint32_t *)(DLYB_CFGR) = 0x0000770c;
    //}
    return NULL;
}*/

static void setup_PMU_counters(void)
{


	struct perf_event_attr pe;
	pid_t mypid = getpid();
	memset(&pe,0,sizeof(struct perf_event_attr));



	pe.disabled = 0;
	pe.type = PERF_TYPE_HARDWARE;
	pe.exclude_hv = 1;
	pe.size = sizeof(struct perf_event_attr);
	pe.enable_on_exec = 1;
	pe.inherit = 1;
	pe.exclude_kernel = 0;
	pe.config = PERF_COUNT_HW_CPU_CYCLES;
	inst_fd  = perf_event_open(&pe,mypid,-1,-1,0);

	if(inst_fd < 0)
	{
		printf("perf_event_open for cycle failed\n\r");
	}

	ioctl(inst_fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(inst_fd, PERF_EVENT_IOC_ENABLE, 0);
	read(inst_fd, &count1, sizeof(long long));
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


static void Write_Register(uintptr_t Addr, uint32_t Value)
{
    volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
    *LocalAddr = Value;
}

static uint32_t Read_Register(uintptr_t Addr)
{
	return *(volatile uint32_t *) Addr;
}

static void Modify_Register(uintptr_t Addr, uint32_t Value,uint32_t mask)
{
    uint32_t data = Read_Register(Addr);
    data = (data & (~mask)) | Value;
    Write_Register(Addr,data); 
}


void delay(volatile uint32_t num) { 
    volatile uint32_t index = 0; 
    for(index = (1 * num); index != 0; index--) {} 
}


int main(int argc, char *argv[]) {

	printf("Welcome!\n\r");

	WC_RNG rng;
    RsaKey         rsaPublicKey;
    RsaKey         rsaPrivateKey;
    RsaKey*        privRsaKey = NULL;
    RsaKey*        pubRsaKey = NULL;
    byte enc[256];
    byte plain[256];
    byte plain1024[128];
    word32 outSz = sizeof(plain);
    word32 encSz = sizeof(enc);
    //cpu_set_t cpuset;


	struct AES_ctx ctx;
    uint32_t lng = 0;
    int ret=0;
    int dummy=0;
    int covert=0;
    uint32_t nb_cycles = 0;
    int nbTracesInput = 0;
    int nbSamplesInput = 0;

    //uint32_t newlng = 0;
    //uint16_t lngArray[400000];
    char command[200];
    char filename[100];
    char cmd[100];
    clock_t t;

    setup_PMU_counters();


	uint8_t * ptArray = malloc(16*sizeof(uint8_t));
	uint8_t * pt2Array = malloc(16*sizeof(uint8_t));
	uint8_t * keyArray = malloc(16*sizeof(uint8_t));
    uint8_t * ctArray = malloc(16*sizeof(uint8_t));


    volatile struct led_core{
        volatile uint32_t control;
    } *core;

    volatile struct ddr_core{
        volatile uint32_t control;
    } *core2;

    if (argc-1 < 3) {
        printf("Usage: %s <UNIT>\n\r", argv[0]);
    }
    else if(argc-1 > 3)
    {
    	printf("4 arguments\n\r");
        strcpy(filename,argv[1]);
        strcpy(cmd,argv[2]);
        nbTracesInput = strtoul(argv[3], NULL, 0);
        nbSamplesInput = strtoul(argv[4], NULL, 0);
    }
    else
    {
    	printf("3 arguments\n\r");
    	strcpy(filename,argv[1]);
        strcpy(cmd,argv[2]);
        nbTracesInput = strtoul(argv[3], NULL, 0);
        printf("arg ok\n\r");
    }
    
    Map_Registers(&map_file,(void **) &core, DLYB_BASE_ADDR, 0x1fff);
    v_base_addr = &core->control;
    printf("v_base_addr = %08x\n\r",v_base_addr);

    Map_Registers(&map_file,(void **) &core, DDR_ADDR, 0x1fff);
    v_ddr_base_addr = &core->control;
    printf("v_ddr_base_addr = %08x\n\r",v_ddr_base_addr);

    DLYB_CR = v_base_addr + 0x1000;
    DLYB_CFGR = v_base_addr + 0x1000 + 0x4;

    /*Map_Registers(&map_file,(void **) &core, DEBUG_BASE_ADDR, 0x1fff);
    v_dwt_base_addr = &core->control;
    printf("v_dwt_base_addr = %08x\n\r",v_dwt_base_addr);

    Map_Registers(&map_file,(void **) &core, BSEC_BASE_ADDR, 0x1fff);
    v_bsec_base_addr = &core->control;
    printf("v_dwt_base_addr = %08x\n\r",v_bsec_base_addr);*/

    /*Map_Registers(&map_file,(void **) &core, PERF_STM32_ADDR, 0x1fff);
    v_perf_stm32_addr = &core->control;
    printf("v_debug_dwt_base_addr = %08x\n\r",v_perf_stm32_addr);*/

    system("rm /tmp/info");
    system("echo stop > /sys/class/remoteproc/remoteproc0/state");
    sprintf(command,"rm /lib/firmware/%s",filename);
    printf("%s\n\r",command);
    system(command);
    sprintf(command,"cp /home/root/SideLine/%s /lib/firmware",filename);
    printf("%s\n\r",command);
    system(command);
    sprintf(command,"echo %s > /sys/class/remoteproc/remoteproc0/firmware",filename);
    printf("%s\n\r",command);
    system(command); 
    printf("echo start > /sys/class/remoteproc/remoteproc0/state\n\r");
    system("echo start > /sys/class/remoteproc/remoteproc0/state");

    //system("stty -onlcr -echo -F /dev/ttyRPMSG0");
    //system("cat /dev/ttyRPMSG0 &");




    //printf("UNIT has been set to %d\n\r",custom_unit);

    //Map_Registers(&map_file,(void **) &core, 0x54004000 , 0x3ff);


    //Modify SDMMC2 frequency to max
    //Modify_Register(v_base_addr + 0x4,0x1,0x2ff);


/*
    //Enable the delay block by setting DEN bit to ‘1’.
    //Enable the length sampling by setting SEN bit to ‘1’.
    Write_Register(v_base_addr+ 0x1000 + 0x0,0x3);
    printf("DLYB_CR = %08x\n\r",Read_Register(v_base_addr+ 0x1000 + 0x0));

    //Enable all delay cells by setting SEL bits to 12.
    Write_Register(v_base_addr+ 0x1000 + 0x4,0xC);
    //For UNIT = 0 to 127 (this step must be repeated until the delay line length is configured):
    for(int i = 0 ; i < 128 ; i++){
        //Update the UNIT value 
	Modify_Register(v_base_addr+ 0x1000 + 0x4,i << 8,0x7f<<8);
	//and wait till the length flag LNGF is set to ‘1'
	while(!(Read_Register(v_base_addr+ 0x1000 + 0x4)>>31));
	//Read LNG bits.
	lng = (Read_Register(v_base_addr+ 0x1000 + 0x4)>>16)&0xfff;
    	//printf("LGN_%d = %03x ",i,lng);
	//If (LNG[10:0] > 0) and (LNG[11] or LNG[10] = 0),
	//if(((lng&0x3ff) > 0) && ((((lng>>10)&1) == 0) || (((lng>>11)&1) == 0))){
           // unit_value = i;
           // break;}

    }

    //Determine how many unit delays (N) span one input clock period.
    for(int i = 10 ; i >= 0 ; i--){
    	if(((lng >> i)&1) == 1){
	    printf("the number of unit delays spanning the input clock period = %d\n\r",i);
            break;}
    }*/
	srand(time(NULL));

    //for(int n = 0 ; n < 1 ; n++)
    //{
	//pt0 =(uint32_t)rand();
	//pt1 =(uint32_t)rand();
	//pt2 =(uint32_t)rand();
	//pt3 =(uint32_t)rand();
	//sprintf(command,"echo '%08x%08x%08x%08x' > /dev/ttyRPMSG0",pt0,pt1,pt2,pt3);
    //launch AES
	//system(command);
	//launch Measurement
 	//t = clock();  
    //printf("time: %f\n\r",(double)(clock()-t));

    //printf("key : 00000000000000000000000000000000\n\r");
    //printf("plaintext : 00000000000000000000000000000000\n\r");

    //Read_Register(v_base_addr);

    if(strstr(cmd,"tiny") || strstr(cmd,"TINY"))
    {

		//AES KEY GENERATION
	    printf("key : ");

		for(int kt_i = 0 ; kt_i < 4 ; kt_i++)
		{
			for(int kt_j = 0 ; kt_j < 4 ; kt_j++)
			{
			keyArray[kt_i *4 + kt_j] = (uint8_t)rand();
			printf("%02x",keyArray[kt_i*4 +kt_j]);
			}
			Write_Register(v_ddr_base_addr + kt_i*4 + 4*8, (((uint32_t)keyArray[kt_i*4] << 24) & 0xff000000) | (((uint32_t)keyArray[kt_i*4+1] << 16) & 0xff0000) | (((uint32_t)keyArray[kt_i*4+2] << 8)& 0xff00) | ((keyArray[kt_i*4+3] << 0)&0xff));
		}
		printf("\n\r");

		//AES KEY EXPENSION
		AES_init_ctx(&ctx, keyArray);

    	for(int nbTraces = 0 ; nbTraces  < nbTracesInput ; nbTraces ++)
    	{

			//Reset PMU Cycle counter (to its initial value)
			ioctl(inst_fd, PERF_EVENT_IOC_RESET, 0);

			//AES PLAINTEXT GENERATION
			//printf("plaintext : ");

			for(int pt_i = 0 ; pt_i < 4 ; pt_i++)
			{
				for(int pt_j = 0 ; pt_j < 4 ; pt_j++)
				{
					ptArray[pt_i*4 + pt_j] = (uint8_t)rand();
					pt2Array[pt_i*4 + pt_j] = ptArray[pt_i*4 + pt_j];
					//printf("%02x",ptArray[pt_i*4 + pt_j]);
				}

				Write_Register(v_ddr_base_addr + pt_i*4, (((uint32_t)ptArray[pt_i*4] << 24) & 0xff000000) | (((uint32_t)ptArray[pt_i*4+1] << 16) & 0xff0000) | (((uint32_t)ptArray[pt_i*4+2] << 8)& 0xff00) | ((ptArray[pt_i*4+3] << 0)&0xff));
			}
			printf("\n\r");


   

            //Read PMU Cycle counter and store it into "count0"
            //read(inst_fd, &count0, sizeof(long long));
            //Read PMU Cycle counter and store it into "count0"
            //read(inst_fd, &count1, sizeof(long long));
    		
    		//wait for CM4 to be ready
            while(((Read_Register(DLYB_CR)>>1)&0x1)==0){}  

            //Launch AES
			AES_ECB_encrypt(&ctx, ptArray);
			AES_ECB_encrypt(&ctx, pt2Array);

		    //end transaction
	   		Write_Register(DLYB_CR,0x1);

            //Read PMU Cycle counter and store it into "count0"
            //read(inst_fd, &count1, sizeof(long long));

	   		//print number cycle
    		//printf("nb cycles : %08lld\n\r",count1-count0);

			//AES PLAINTEXT GENERATION
			/*printf("ciphertext : ");
			for(int pt_i = 0 ; pt_i < 16 ; pt_i++)
			{

				ptArray[pt_i] = (uint8_t)rand();
				printf("%02x",ptArray[pt_i]);
			}
			printf("\n\r");*/

			//usleep(10);
			for(int pt_i = 0 ; pt_i < 4 ; pt_i++)
			{
			Write_Register(v_ddr_base_addr + pt_i*4 + 4*4, (((uint32_t)ptArray[pt_i*4] << 24) & 0xff000000) | (((uint32_t)ptArray[pt_i*4+1] << 16) & 0xff0000) | (((uint32_t)ptArray[pt_i*4+2] << 8)& 0xff00) | ((ptArray[pt_i*4+3] << 0)&0xff));
			}
			//printf("nb_cycles: %08d\n\r",Read_Register(v_ddr_base_addr+ 0x4 * 4));

		}
    }
    else if(strstr(cmd,"ssl") || strstr(cmd,"SSL"))
    {

        AES_KEY key;

        //AES KEY GENERATION
        printf("key : ");

        for(int kt_i = 0 ; kt_i < 4 ; kt_i++)
        {
            for(int kt_j = 0 ; kt_j < 4 ; kt_j++)
            {
            keyArray[kt_i *4 + kt_j] = (uint8_t)rand();
            printf("%02x",keyArray[kt_i*4 +kt_j]);
            }
            Write_Register(v_ddr_base_addr + kt_i*4 + 4*8, (((uint32_t)keyArray[kt_i*4] << 24) & 0xff000000) | (((uint32_t)keyArray[kt_i*4+1] << 16) & 0xff0000) | (((uint32_t)keyArray[kt_i*4+2] << 8)& 0xff00) | ((keyArray[kt_i*4+3] << 0)&0xff));
        }
        printf("\n\r");

        //AES KEY EXPENSION
        AES_set_encrypt_key(keyArray,128,&key);

        for(int nbTraces = 0 ; nbTraces  < nbTracesInput ; nbTraces ++)
        {

            //Reset PMU Cycle counter (to its initial value)
            ioctl(inst_fd, PERF_EVENT_IOC_RESET, 0);

            //AES PLAINTEXT GENERATION
            //printf("plaintext : ");

            for(int pt_i = 0 ; pt_i < 4 ; pt_i++)
            {
                for(int pt_j = 0 ; pt_j < 4 ; pt_j++)
                {
                    ptArray[pt_i*4 + pt_j] = (uint8_t)rand();
                    //printf("%02x",ptArray[pt_i*4 +pt_j]);
                }
                Write_Register(v_ddr_base_addr + pt_i*4, (((uint32_t)ptArray[pt_i*4] << 24) & 0xff000000) | (((uint32_t)ptArray[pt_i*4+1] << 16) & 0xff0000) | (((uint32_t)ptArray[pt_i*4+2] << 8)& 0xff00) | ((ptArray[pt_i*4+3] << 0)&0xff));
            }
            //printf("\n\r");


   

            //Read PMU Cycle counter and store it into "count0"
            //read(inst_fd, &count0, sizeof(long long));
            //Read PMU Cycle counter and store it into "count0"
            //read(inst_fd, &count1, sizeof(long long));
            
            //wait for CM4 to be ready
            while(((Read_Register(DLYB_CR)>>1)&0x1)==0){}  

            //Launch AES
            AES_encrypt(ptArray,ctArray,&key);
            AES_encrypt(ptArray,ctArray,&key);

            //end transaction
            Write_Register(DLYB_CR,0x1);

            //Read PMU Cycle counter and store it into "count0"
            //read(inst_fd, &count1, sizeof(long long));

            //print number cycle
            //printf("nb cycles : %08lld\n\r",count1-count0);

            //AES PLAINTEXT GENERATION
            /*printf("ciphertext : ");
            for(int pt_i = 0 ; pt_i < 16 ; pt_i++)
            {

                ptArray[pt_i] = (uint8_t)rand();
                printf("%02x",ptArray[pt_i]);
            }
            printf("\n\r");*/

            //usleep(10);
            //printf("ciphertext : ");

            for(int pt_i = 0 ; pt_i < 4 ; pt_i++)
            {
                for(int pt_j = 0 ; pt_j < 4 ; pt_j++)
                {
                    //printf("%02x",ctArray[pt_i*4 +pt_j]);
                }
               Write_Register(v_ddr_base_addr + pt_i*4 + 4*4, (((uint32_t)ctArray[pt_i*4] << 24) & 0xff000000) | (((uint32_t)ctArray[pt_i*4+1] << 16) & 0xff0000) | (((uint32_t)ctArray[pt_i*4+2] << 8)& 0xff00) | ((ctArray[pt_i*4+3] << 0)&0xff));
            }
            //printf("\n\r");

        }
            //printf("nb_cycles: %08d\n\r",Read_Register(v_ddr_base_addr+ 0x4 * 4));

        
    }
	/********RSA 1024 NAIVE IMPLEM*********/
	else if(strstr(cmd,"naive") || strstr(cmd,"NAIVE"))
	{
		printf("naive RSA Attack, %d traces\n\r",nbTracesInput);

		/****** INIT RSA ******/
		ret = wc_InitRsaKey(&rsaPrivateKey, NULL);
		privRsaKey = &rsaPrivateKey;
		ret = mp_read_unsigned_bin(&rsaPrivateKey.n, n1024, sizeof(n1024));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.d, d1024, sizeof(d1024));
		rsaPrivateKey.type = RSA_PRIVATE_DECRYPT;



    	for(int nbTraces = 0 ; nbTraces  < nbTracesInput ; nbTraces ++)
    	{
			//Reset PMU Cycle counter (to its initial value)
			ioctl(inst_fd, PERF_EVENT_IOC_RESET, 0);

		    //wait for CM4 to be ready
            while(((Read_Register(DLYB_CR)>>1)&0x1)==0){}

			//printf("size of enc: %d\n\r",sizeof(enc1024));
			//printf("size of plain: %d\n\r",sizeof(plain1024));

		    /*printf(("\n\renc:\n\r"));
			for(int i = 0 ; i < 128 ; i++)
			{
				printf("%02x ",enc1024[i]);
			}
			printf("\n\r");*/

            //Read PMU Cycle counter and store it into "count0"
            read(inst_fd, &count0, sizeof(long long));
                
            //launch RSA
			ret = sp_RsaPrivate_1024_custom_naive(enc1024,sizeof(enc1024),&rsaPrivateKey.d,&rsaPrivateKey.n,plain1024,sizeof(plain1024));

            //Read PMU Cycle counter and store it into "count0"
            read(inst_fd, &count1, sizeof(long long));

			/*printf(("\n\rplain:\n\r"));
			for(int i = 0 ; i < 128 ; i++)
			{
				printf("%02x",plain1024[i]); 
			}
			printf("\n\r");*/

		    /*end transaction*/
	   		Write_Register(DLYB_CR,0x1);

    		printf("nbTraces: %d\n\r",nbTraces);
	   	}

		// RELEASE RSA KEYS
		if (privRsaKey != NULL)
			wc_FreeRsaKey(privRsaKey);
	}
	/********RSA 1024 NAIVE IMPLEM*********/
	else if(strstr(cmd,"else") || strstr(cmd,"ELSE"))
	{
		printf("constant time RSA Attack, %d traces\n\r",nbTracesInput);

		/****** INIT RSA ******/
		ret = wc_InitRsaKey(&rsaPrivateKey, NULL);
		privRsaKey = &rsaPrivateKey;
		ret = mp_read_unsigned_bin(&rsaPrivateKey.n, n1024, sizeof(n1024));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.d, d1024, sizeof(d1024));
		rsaPrivateKey.type = RSA_PRIVATE_DECRYPT;

    	for(int nbTraces = 0 ; nbTraces  < nbTracesInput ; nbTraces ++)
    	{
		    //wait for CM4 to be ready
	   		while((Read_Register(v_base_addr+0x1000) & 0x2) >> 1 == 0)
	   		{
	   			//printf("wait");
	   		}

			//printf("size of enc: %d\n\r",sizeof(enc1024));
			//printf("size of plain: %d\n\r",sizeof(plain1024));

		    /*printf(("\n\renc:\n\r"));
			for(int i = 0 ; i < 128 ; i++)
			{
				printf("%02x ",enc1024[i]);
			}
			printf("\n\r");*/



			ret = sp_RsaPrivate_1024_custom_constanttime(enc1024,sizeof(enc1024),&rsaPrivateKey.d,&rsaPrivateKey.n,plain1024,sizeof(plain1024));

			/*printf(("\n\rplain:\n\r"));
			for(int i = 0 ; i < 128 ; i++)
			{
				printf("%02x",plain1024[i]); 
			}
			printf("\n\r");*/

		    /*end transaction*/
	   		Write_Register(v_base_addr+0x1000,0x1);

    		printf("nbTraces: %d - ",nbTraces);
    		printf("nb cycles : %08lld\n\r",count1-count0);
	   	}

		// RELEASE RSA KEYS
		if (privRsaKey != NULL)
			wc_FreeRsaKey(privRsaKey);
	}
	else if(strstr(cmd,"rsa") || strstr(cmd,"RSA"))
	{

		printf("WolfSSL RSA Attack, %d traces\n\r",nbTracesInput);
		/******  INIT RSA ******/
		ret = wc_InitRsaKey(&rsaPublicKey, NULL);
		//xil_printf("\n\rInit Public key : %d\n\r",ret);
		pubRsaKey = &rsaPublicKey;
		ret = mp_read_unsigned_bin(&rsaPublicKey.n, public_key_2048_n,sizeof(public_key_2048_n));
		//xil_printf("Import modulus n alice : %d\n\r",ret);
		ret = mp_set_int(&rsaPublicKey.e, public_key_2048_e);
		//xil_printf("Import private key e alice : %d\n\r",ret);
		//ret = wc_RsaPublicEncrypt(msg, sizeof(msg),enc,sizeof(enc),&rsaPublicKey,&rng);
		ret = wc_RsaDirect(msg, sizeof(msg), enc, &encSz , &rsaPublicKey, RSA_PUBLIC_ENCRYPT, &rng);
		//xil_printf("Alice public encrypt : %d\n\r",ret);

		/*printf("encrypted message: \n\r");

		for(int i = 0 ; i < 256 ; i++)
		{
		printf("%02x ",enc[i]);
		}

		printf("\n\r");*/

		ret = wc_InitRsaKey(&rsaPrivateKey, NULL);

		//xil_printf("\n\rInit Private key : %d\n\r",ret);

		privRsaKey = &rsaPrivateKey;

		ret = mp_read_unsigned_bin(&rsaPrivateKey.n, public_key_2048_n,sizeof(public_key_2048_n));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.p, prime_number_p,sizeof(prime_number_p));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.q, prime_number_q,sizeof(prime_number_q));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.dP, prime_exponent_dp,sizeof(prime_exponent_dp));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.dQ, prime_exponent_dq,sizeof(prime_exponent_dq));
		ret = mp_read_unsigned_bin(&rsaPrivateKey.u, coefficient_qinv,sizeof(coefficient_qinv));
		ret = mp_set_int(&rsaPrivateKey.e, public_key_2048_e);
		ret = mp_read_unsigned_bin(&rsaPrivateKey.d, private_key_2048_d,sizeof(private_key_2048_d));
		rsaPrivateKey.type = RSA_PRIVATE_DECRYPT;




    	for(int nbTraces = 0 ; nbTraces  < nbTracesInput ; nbTraces ++)
    	{


	   		while((Read_Register(v_base_addr+0x1000) & 0x2) >> 1 == 0)
	   		{
	   			
	   		}

			ret = wc_RsaDirect(enc, sizeof(enc), plain, &outSz , &rsaPrivateKey, RSA_PRIVATE_DECRYPT, &rng);

			/*printf("plain message: \n\r");

	    	for(int i = 0 ; i < 256 ; i++)
			{
			printf("%02x ",plain[i]);
			}

	    	printf("\n\r");*/


	   		//covert = 0;

	   		/*for(int i = 0 ; i < 5 ; i++)
	   		{
				ret = strcmp(str1,str2);
				delay(1800);   			
			}		*/
	   		/*** Delay ***/
	   		//delay(4000);

	   		/*** AES ***/
			//AES_ECB_encrypt(&ctx, ptArray);

	   		/*end transaction*/
	   		Write_Register(v_base_addr+0x1000,0x1);
	   		//pritnf("%08x\n\r",Read_Register(v_base_addr+0x1000));

					//dummy =

					/**** EXPERIMENT 1 ***/
				    /*if(covert%2==0){
						usleep(100);
					}

					else{
						ret = strcmp(str1,str2);
						ret = strcmp(str2,str1);
						ret = strcmp(str1,str2);
						ret = strcmp(str2,str1);
					}*/


					/**** EXPERIMENT 2 ***/
	   				/*t = clock();
				    if(covert%2==0){
				    	//t = clock();
						ret = strcmp(str1,str2);
						//printf("timecompare: %f, ret: %d\n\r",(double)(clock()-t),ret);
						//t = clock();
						delay(1800);
						//printf("timesleep: %f, ret: %d\n\r",(double)(clock()-t),ret);
						ret = strcmp(str2,str1);
						delay(1800);
						ret = strcmp(str1,str2);
						delay(1800);
						ret = strcmp(str2,str1);
						delay(1800);
						//printf("time1: %f, ret: %d\n\r",(double)(clock()-t),ret);
						
					}
					else{
						//t = clock();
						ret = strcmp(str3,str4);
						//printf("timecompare: %f, ret: %d\n\r",(double)(clock()-t),ret);
						//t = clock();
						delay(900);
						//printf("timesleep: %f, ret: %d\n\r",(double)(clock()-t),ret);
						ret = strcmp(str4,str3);
						delay(900);
						ret = strcmp(str3,str4);
						delay(900);
						ret = strcmp(str4,str3);
						delay(900);
						ret = strcmp(str3,str4);
						delay(900);
						ret = strcmp(str4,str3);
						delay(900);
						ret = strcmp(str3,str4);
						delay(900);
						ret = strcmp(str4,str3);
						delay(900);
				        //printf("time2: %f, ret: %d dummy:%d\n\r",(double)(clock()-t),ret,dummy);
					}					
					covert++;	*/  

	   			    /**** EXPERIMENT 3 ***/

	   		/*while((Read_Register(v_base_addr+0x1000) & 0x2) >> 1 == 1)
	   		{
	    	}*/
    		printf("nbTraces: %d ret:%d covert:%d\n\r",nbTraces,ret,covert);
		}


	}
	else if(strstr(cmd,"monitor") || strstr(cmd,"MONITOR"))
	{
		usleep(1000000);
		int lngArray[nbSamplesInput+1];
        int s;


	    printf("key : 00000000000000000000000000000000\n\r");

        /*thread = pthread_self();
        CPU_ZERO(&cpuset);
        CPU_SET(1, &cpuset);

        printf("CPU set ok\n\r");

        s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
        if (s != 0)
            printf("pthread_setaffinity_np error\n\r");

        printf("set affinity ok\n\r");

        s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
        if (s != 0)
            printf("pthread_getaffinity_np\n\r");

        printf("Set returned by pthread_getaffinity_np() contained:\n");

        for (int j = 0; j < 2; j++)
            if (CPU_ISSET(j, &cpuset))
                printf("    CPU %d\n", j);*/

	    //printf("counter STM32: %08x\n\r",Read_Register(v_perf_stm32_addr));
	    //printf("counter STM32: %08x\n\r",Read_Register(v_perf_stm32_addr+0x4));
	    //enable cycle counter
	    //Write_Register(v_dwt_base_addr+0xc00,0x80000000);

	    //asm volatile("MSR PM_USERENR, %0" : : "r" ((uint32_t)USEREN));  

	    //printf("BSEC: %08x\n\r",Read_Register(v_bsec_base_addr+0x14));
	    //printf("counter enabled? : %08x\n\r",Read_Register(v_dwt_base_addr+0x1c00));
		//Write_Register(v_dwt_base_addr+0xc00,0x80000000);
		//Modify_Register(v_dwt_base_addr+0xci00,0x80000000,0x80000000);
	    //printf("cycle enable: %08x\n\r",Read_Register(v_dwt_base_addr+0x1c00));
	     //cycle counter init
		//Write_Register(v_dwt_base_addr+0xe00,0x80000000);
		//printf("DBG OSLAR: %08x\n\r",Read_Register(v_dwt_base_addr+0x0300));
		//printf("DBG DSCR: %08x\n\r",Read_Register(v_dwt_base_addr+0x0088));
	    //printf("DBG OSLSR: %08x\n\r",Read_Register(v_dwt_base_addr+0x0304));

		//printf("PMU CFGR: %08x\n\r",Read_Register(v_dwt_base_addr+0x1e00));
		//printf("PMU CR: %08x\n\r",Read_Register(v_dwt_base_addr+0x1e04));
		//Write_Register(v_dwt_base_addr+0x1e08,0x00000001);
	    //printf("PMU USERENR: %08x\n\r",Read_Register(v_dwt_base_addr+0x1e08));
	    //Write_Register(v_dwt_base_addr+0e08,0x00000001);
		//Modify SDMMC2 frequency to max

		//Modify_Register(v_base_addr+0x4,0x1,0x2ff);


	    for(int num = 0 ; num < nbTracesInput ; num++)
	    {
			
		    //Enable all delay cells by setting SEL bits to 12.
		    Write_Register(DLYB_CFGR, 0xC);

			//Reset PMU Cycle counter (to its initial value)
			ioctl(inst_fd, PERF_EVENT_IOC_RESET, 0);

            //err = pthread_create(&(thread), NULL, &WriteDLYBRegister, NULL);
            //if(err != 0){ printf("\ncan't create thread :[%d]", err);}
            
            //Enable the length sampling by setting SEN bit to ‘1’.
            Write_Register(DLYB_CR, 0x3);

            //write unit register (required to update read value)
            *(volatile uint32_t *)(DLYB_CFGR) = 0x0000770c;

            //Read PMU Cycle counter and store it into "count0"
            read(inst_fd, &count0, sizeof(long long));

			//Start the acquisition of the delay-line
			for(int i = 0 ; i < nbSamplesInput+1 ; i++)
			{
				//write unit register (required to update read value)
				//*(volatile uint32_t *)(DLYB_CFGR) = 0x0000770c;

				//Modify_Register(DLYB_CFGR,(UNIT_VAL)<<8,0x7f<<8);
				//read data
				lngArray[i] = *(volatile uint32_t *)(DLYB_CFGR);
			}

			 //Read PMU Cycle counter and store it into "count1"
			read(inst_fd, &count1, sizeof(long long));

			//Wait for CM4 to end AES/RSA computations
			while(((Read_Register(DLYB_CR)>>1)&0x1)==1){}

			//Start Data treatment
			printf("\n\rplaintext : 000000000000000000000000%08x\n\r",num);

			for(int i = 0 ; i < nbSamplesInput ; i++)
			{
				switch(UNIT_VAL)
				{

				}
			}

			//print the number of cycles taken by the acquisition
		    printf("\n\rnb cycles : %08lld",count1-count0);
		    printf("\n\rtime : %.02f us",(double)(1.0/650.0)*(double)(count1-count0));
			printf("\n\rciphertext : 00000000000000000000000000000000");
		}
	}
	else if(strstr(cmd,"stress") || strstr(cmd,"STRESS"))
	{
		//Force_Variability(119,1);
		//Test_Variability(115,125,1,2);
		float mean = 0.0;
		while(1)
		{
			mean = Get_Mean(100,119);

			//usleep(100000);


			if(mean < 16.5)
			{
				printf("Too hot ! - mean : %f\n\r",mean);
				usleep(200);
				//exit(0);
			}
			else if((mean > 16.2) && (mean < 16.9))
			{
				printf("ok\n\r");
			}
			else
			{
				printf("Too cold ! - mean : %f\n\r",mean);
			}
		}


		
	}
	else
	{
		printf("command not found\n\r");
	}

	printf("\n\rEND!!\n\r");
	return 0;
}





float Get_Mean(uint32_t nSample,uint8_t dlval)
{
		float mean = 0.0;
		int lngArray[nSample];

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_CR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_CR,0x3);

		//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
		*(volatile uint32_t *)(DLYB_CFGR + 0x4) = 0xc + (dlval << 8);

		//Start the acquisition of the delay-line
		for(int iSample = 0 ; iSample < nSample ; iSample ++)
		{
			lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);
			mean += Decode_DelayLine(lngArray[iSample]);
		}

		return mean/nSample;
}


float Force_Variability(uint8_t dlval, uint16_t clkval)
{

	int attempts = 1000;
	uint32_t val1,val2 = 0;
	uint32_t clockcycles = 0;
	float currentVar = 0.0;
	float lastVar = 0.0;
	float BestVar = 10.0; //minimum best var to beat !

	printf("\n\rForce variability");

	//for(int iTest = 0 ; iTest < attempts ; iTest++)
	while(1)
	{
		//printf("\n\r\n\rAttempt: %d",iTest);
		currentVar = Measure_Variability(dlval,clkval);
		printf("\n\r    Current Var is: %f",currentVar);

		if(currentVar < 0.5)
		{
			val1 = clock();
			Decrease_Temperature(100000);
			val2 = clock();
			//printf("\n\r    Increase Temp duration: %d ms",val2-val1);
		}
		else
		{
			val1 = clock();
			//read(inst_fd, &count0, sizeof(long long));
			Increase_Temperature(500000);
			//read(inst_fd, &count1, sizeof(long long));
			val2 = clock();
			//printf("\n\r    Decrease Temp duration: %d ms",val2-val1);
			//printf("\n\r    Decrease Temp duration: %f ms",(1.0/209000000)*(count1-count0));
		}

		if(currentVar >= BestVar)
		{
			BestVar = currentVar;
		}

		lastVar = currentVar;
	}

	return currentVar;
}

int Increase_Temperature(uint32_t attempts)
{
	int ret = 0;//,clockcycles = 0;

	printf("\n\r");
	for(int iTest = 0 ; iTest < attempts ; iTest++)
	{
		//*DWT_CYCCNT=0;

		//clockcycles = *DWT_CYCCNT;
		//printf("\n\r    strcmp duration: %f ms",((1.0/209000)*clockcycles));

		//#ifdef strmcp
		//ret = strcmp(str1,str2);
		//#endif

		//#ifdef ssqrt
		uint64_t rnd = rand();
		double r_d = sqrt((double)rnd) * sqrt((double)rnd);
		long double r_ld = sqrtl((long double)rnd) * sqrtl((long double)r_d);
		printf("r_ld: %f\r",r_ld);
		//#endif

	}
	printf("\n\r");

	return ret;
}

uint8_t Decode_DelayLine(uint32_t delayvalue)
{
	uint8_t temp = 0;

	for(uint8_t iNibble = 0 ; iNibble < 3 ; iNibble++)
	{
		switch((delayvalue>>(16+iNibble*4))&0xF)
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

	return temp;
}

void Decrease_Temperature(uint32_t duration)
{
	//printf("clock : %d\n\r",clock());
	int val = clock();
	int cmp = 0;

	while(cmp < duration)
	{
		cmp = clock() - val;

		if(cmp < 0){ break;}
	}
}

float Measure_Variability(uint8_t dlval, uint16_t clkval)
{
	float mean = 0.0;
	float var = 0.0;
	float globalMean = 0.0;
	float globalVar = 0.0;
	unsigned int nSample = 4000;
	unsigned int nTrace = 1;
	unsigned int count = 0;
	int lngArray[nSample];

	//Modify SDMMC2 frequency to max
	//Modify_Register(MEM_ADDR + 0x4,clkval,0x2ff);
	//printf("\n\r    CLK :%d, DLYB: %d",clkval,dlval);



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

		//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
		*(volatile uint32_t *)(DLYB_CFGR + 0x4) = 0xc + (dlval << 8);

		//Start the acquisition of the delay-line
		for(int iSample = 0 ; iSample < nSample ; iSample ++)
		{
			lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);
		}

		// compute mean
		for(int iSample = 0 ; iSample < nSample; iSample++)
		{
			count = 0;
			for(int i = 0 ; i < 12 ; i++)
			{
				count += (lngArray[iSample] >> (16+i)) & 1;
			}

			mean += (float)count;
		}

		mean /= (float)nSample;
		globalMean += mean;

		//printf("\n\rMean: %f",mean);

		// compute variance
		for(int iSample = 0 ; iSample < nSample; iSample++)
		{
			count = 0;
			for(int i = 0 ; i < 12 ; i++)
			{
				count += (lngArray[iSample] >> (16+i)) & 1;
			}

			var += pow((float)count - mean,2);
		}

		var /= (float)(nSample-1);
		globalVar += var;


		//printf("\n\rVariance: %f",var);

		/*end transaction*/
		Write_Register(DLYB_CR,0x1);

	}

	//printf("\n\r    Global Mean: %f",globalMean);

	return globalVar;
};

uint8_t Test_Variability(uint8_t dlmin, uint8_t dlmax,uint8_t clkmin, uint8_t clkmax)
{
	printf("\n\rTesting variability");

	int bestVal = 0;
	float bestVar = 0.0;
	float mean = 0.0;
	float var = 0.0;
	float globalMean = 0.0;
	float globalVar = 0.0;
	unsigned int nSample = 400;
	unsigned int nTrace = 1000;
	unsigned int count = 0;
	unsigned int regVal = 0;
	int lngArray[nSample];

	for(int clkvalue = clkmin ; clkvalue <= clkmax ; clkvalue++)
	{
		//Modify SDMMC2 frequency to max
		//Modify_Register(MEM_ADDR + 0x4,clkvalue,0x2ff);

		for(int DLvalue = dlmin ; DLvalue <= dlmax ; DLvalue++)
		{
			globalMean = 0.0;
			globalVar = 0.0;
			printf("\n\rDLvalue : %d DLYB_CFGR = %08x",DLvalue, 0xc + (DLvalue << 8));

			for(int iTrace = 0 ; iTrace < nTrace ; iTrace++)
			{
				var = 0.0;
				mean = 0.0;

				//Disable the length sampling by setting SEN bit to ‘0’.
				Write_Register(DLYB_CR,0x1);

				//Enable the length sampling by setting SEN bit to ‘1’.
				Write_Register(DLYB_CR,0x3);

				//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
				*(volatile uint32_t *)(DLYB_CFGR + 0x4) = 0xc + (DLvalue << 8);


				//Start the acquisition of the delay-line
				for(int iSample = 0 ; iSample < nSample ; iSample ++)
				{
					lngArray[iSample] = *(volatile uint32_t *)(DLYB_CFGR);
				}


				// compute mean
				for(int iSample = 0 ; iSample < nSample; iSample++)
				{
					count = 0;
					for(int i = 0 ; i < 12 ; i++)
					{
						count += (lngArray[iSample] >> (16+i)) & 1;
					}

					mean += (float)count;
				}

				mean /= (float)nSample;
				globalMean += mean;

				//printf("\n\rMean: %f",mean);

				// compute variance
				for(int iSample = 0 ; iSample < nSample; iSample++)
				{
					count = 0;
					for(int i = 0 ; i < 12 ; i++)
					{
						count += (lngArray[iSample] >> (16+i)) & 1;
					}

					var += pow((float)count - mean,2);
				}

				var /= (float)(nSample-1);
				globalVar += var;


				//printf("\n\rVariance: %f",var);

				/*end transaction*/
				Write_Register(DLYB_CR,0x1);


			}
			regVal = (lngArray[50] >> 16) & 0xFFF;
			printf("\n\rRegisterView: ");
			printBits(12,regVal);
			printf("\n\rGlobal Mean: %f",globalMean);
			printf("\n\rGlobal Var: %f",globalVar);

			if(globalVar > bestVar)
			{
				bestVar = globalVar;
				bestVal = DLvalue;

			}

		}
	}

	printf("\n\r\n\rThe Best DL value found is %d\n\r",bestVal);
	usleep(2000);

	return bestVal;
}


void printBits(uint8_t size, uint32_t data)
{
    for (int i = size-1; i >= 0; i--) {
            printf("%lu", (data>>i)  & 1);
    }
}



