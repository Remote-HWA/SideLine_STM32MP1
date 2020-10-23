/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main_cortex-M4.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 Thales.
  * All rights reserved.
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <main_cortex-M4.h>


/* Private define ------------------------------------------------------------*/
#define AES_BUFFER_SIZE 400
#define DDR_BASE_ADDR 0xD0000000

//#define SD1_EN
#define SD2_EN
//#define SD3_EN
//#define SQ_EN

#if defined SQ_EN
	#define DLYB_ADDR 0x58004000
	#define MEM_ADDR 0x58003000
#elif defined SD1_EN
	#define DLYB_ADDR 0x58006000
	#define MEM_ADDR 0x58005000
#elif defined SD2_EN
	#define DLYB_ADDR 0x58008000
	#define MEM_ADDR 0x58007000
#else
	#define DLYB_ADDR 0x48005000
	#define MEM_ADDR 0x48004000
#endif


//#define AES_DMA
#define AES


/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_memtomem_dma2_stream0;

/* USER CODE BEGIN PV */
static uint32_t aDST_Buffer[AES_BUFFER_SIZE];
//AES = 4500 DMA = 5500
//#define aDST_Buffer (*(volatile unsigned int (*)[AES_BUFFER_SIZE])(0xD0000040))
//AES = 4500 DMA = 6800
//For DMA transfer
static __IO uint32_t transferErrorDetected; /* Set to 1 if an error transfer is detected */
uint32_t transferComplete; /* Set to 1 if DMA transfer has ended */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void 	MX_GPIO_Init(void);
static void 	MX_USART3_UART_Init(void);
static void 	MX_DMA_Init(void);
static void 	Write_Register(uintptr_t Addr, uint32_t Value);
static uint32_t Read_Register(uintptr_t Addr);
static void 	Modify_Register(uintptr_t Addr, uint32_t Value,uint32_t mask);
uint8_t 		Decode_DelayLine(uint32_t reg,uint8_t DLvalue,uint8_t printvalue);
void 			AES_DMA_Attack_Self(void);
void 			Compute_AES(void);
static void 	TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void 	TransferError(DMA_HandleTypeDef *DmaHandle);
static void 	Init_PerfCounters(void);
void 			printBits(uint8_t size, uint32_t data);
/* USER CODE BEGIN PFP */

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#ifdef __GNUC__
#define GETCHAR_PROTOTYPE int __io_getchar (void)
#else
#define GETCHAR_PROTOTYPE int fgetc(FILE * f)
#endif /* __GNUC__ */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t gdlval = 41;
uint8_t gclkval = 0;
volatile uint32_t *DWT_CONTROL = (uint32_t *) 0xE0001000;
volatile uint32_t *DWT_CYCCNT = (uint32_t *) 0xE0001004;
volatile uint32_t *DEMCR = (uint32_t *) 0xE000EDFC;
volatile uint32_t *LAR  = (uint32_t *) 0xE0001FB0;   // <-- added lock access register
/* USER CODE END 0 */


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/*HW semaphore Clock enable*/
	__HAL_RCC_HSEM_CLK_ENABLE();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	BSP_LED_Init(LED7);
	Init_PerfCounters();

	/* Print Hello Banner */
	printf("\n\r\n\r\n\r\n\r");
	printf(
	"   _____    __    ___                   \n\r"
	"  / __(_)__/ /__ / (_)__  ___           \n\r"
	" _\\ \\/ / _  / -_) / / _ \\/ -_)	     \n\r"
	"/___/_/\\_,_/\\__/_/_/_//_/\\__/  on AES\n\r");


	#if defined AES_DMA || defined AES

		transferErrorDetected = 0;
		/* Init DMA for DLYB to memory data transfers */
		MX_DMA_Init();
		/* Select Callbacks functions called after Transfer complete and Transfer error */
		HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
		HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

	#endif

	#ifdef AES_DMA
	AES_DMA_Attack_Self();
	#elif defined(AES)
	Compute_AES();
	#else
	#endif



	while (1){}

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 921600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

GETCHAR_PROTOTYPE
{
  uint8_t ch = 0;

  /* Clear the Overrun flag just before receiving the first character */
  __HAL_UART_CLEAR_OREFLAG(&huart3);

  HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}


/**
  * @brief Performance Counters Initialization Function
  * @param None
  * @retval None
  */
static void Init_PerfCounters(void)
{
	*DEMCR = *DEMCR | 0x01000000;     // enable trace
	*LAR = 0xC5ACCE55;                // <-- added unlock access to DWT (ITM, etc.)registers
	*DWT_CYCCNT = 0;                  // clear DWT cycle counter
	*DWT_CONTROL = *DWT_CONTROL | 1;  // enable DWT cycle counter

}


/**
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma2_stream0
  */
static void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMAMUX_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma2_stream0 on DMA2_Stream0 */
  hdma_memtomem_dma2_stream0.Instance = DMA2_Stream0;
  hdma_memtomem_dma2_stream0.Init.Request = DMA_REQUEST_MEM2MEM;   /*DMA_REQUEST_GENERATOR0;*/
  hdma_memtomem_dma2_stream0.Init.Direction = DMA_MEMORY_TO_MEMORY;  /*DMA_PERIPH_TO_MEMORY;*/
  hdma_memtomem_dma2_stream0.Init.PeriphInc = /*DMA_PINC_ENABLE;*/ DMA_PINC_DISABLE;
  hdma_memtomem_dma2_stream0.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma2_stream0.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma2_stream0.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma2_stream0.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma2_stream0.Init.Priority = DMA_PRIORITY_VERY_HIGH; /*DMA_PRIORITY_LOW*/;
  hdma_memtomem_dma2_stream0.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_memtomem_dma2_stream0.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_memtomem_dma2_stream0.Init.MemBurst = DMA_MBURST_INC4;
  hdma_memtomem_dma2_stream0.Init.PeriphBurst = DMA_PBURST_INC4;
  if (HAL_DMA_Init(&hdma_memtomem_dma2_stream0) != HAL_OK)
  {
    Error_Handler();
  }

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, DEFAULT_IRQ_PRIO, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief  DMA conversion complete callback
  * @note   This function is executed when the transfer complete interrupt
  *         is generated
  * @retval None
  */
static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
{
  /* Turn LED7 on: Transfer correct */
	transferComplete = 1;
	BSP_LED_On(LED7);
}

/**
  * @brief  DMA conversion error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
static void TransferError(DMA_HandleTypeDef *DmaHandle)
{
	transferErrorDetected = 1;
	Error_Handler();
}

/**
  * @brief Convert DLYB state into voltage related information
  * @param reg, DLvalue, printvalue
  * @retval None
  */
uint8_t Decode_DelayLine(uint32_t reg,uint8_t DLvalue,uint8_t printvalue)
{
	int temp = 0;

	if(DLvalue == 119)
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
	else if(DLvalue == 41)
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
					printf(" ");
					printBits(6,(reg>>(16+itdc*6))&0xF );
					printf(" ");

					break;
			}
		}
	}

	if(printvalue == 1)
	{
		printf("%c",temp+50);
	}

	return temp;
}


void AES_DMA_Attack_Self(void)
{
	AES_KEY_OSSL keydma;
	int cycleAES = 0;
	int cycleDMA = 0;
	uint8_t ptArray[16];
	uint8_t ctArray[16];
	uint8_t keyArray[16];
	uint32_t sample_min = 0;
	uint32_t sample_max = AES_BUFFER_SIZE;
	float mean = 0.0;
	uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};


	printf("****Start Openssl DMA AES attack****\n\r");

	//Force_Variability(119, 1);

	// Generate random key and print it
	printf("\n\rkey : ");
	for(int k_i = 0 ; k_i < 16 ; k_i++){keyArray[k_i] = exKeyArray[k_i];/*((uint8_t)rand())*/printf("%02x",keyArray[k_i]);}
	printf("\n\r");

	//AES key expansion
	AES_set_encrypt_key(keyArray,128,&keydma);

	// loop = number of traces acquired
	for(int num = 0 ; num < 100000000 ; num++)
	{
		// Generate random plain text and print it
		for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();}

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_ADDR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_ADDR,0x3);
		//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
		//*(volatile uint32_t *)(DLYB_ADDR + 0x4) = 0x0000770c;
		*(volatile uint32_t *)(DLYB_ADDR + 0x4) = 0xc + (gdlval << 8);

		/* STart DMA transaction */
		transferComplete = 0;

		// Launch DMA
		if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, AES_BUFFER_SIZE) != HAL_OK){Error_Handler();}

		//clear DWT cycle counter
		*DWT_CYCCNT = 0;

		// Launch encryptions
		//for(int i = 0 ; i < 2; i++)
		//{
		for(int i = 0 ; i < 70 ; i++){}
		AES_encrypt(ptArray,ctArray,&keydma);
		//AES_encrypt(ptArray,ctArray,&keydma);
		for(int i = 0 ; i < 70 ; i++){}

		cycleAES = *DWT_CYCCNT;
		//}

		while(transferComplete == 0){}

		// Print DWT cycle counter value
		cycleDMA = *DWT_CYCCNT;

		// Print Samples
		for(int i = sample_min ; i < sample_max ; i++)
		{
			mean += Decode_DelayLine(aDST_Buffer[i],gdlval,0);
		}

		mean = mean/(sample_max-sample_min);

		//printf("mean: %f\n\r",mean);

		if(mean > 1.5 && mean < 2.5) //for 41
		//if(mean > 21.5 && mean < 23)//for 119
		{
			printf("\n\rplaintext : ");
			for(int u = 0; u<16 ; u++){printf("%02x",ptArray[u]);}
			printf("\n\r");

			for(int i = sample_min ; i < sample_max ; i++)
			{
				 Decode_DelayLine(aDST_Buffer[i],gdlval,1);
			}
		}

		//print time
		//printf ("\n\rtime : %.2f",(double)(1.0/209.0)*(double)cycleAES);
		//printf ("\n\rnb cycles AES : %d",(int)cycleAES);
		//printf ("\n\rnb cycles DMA: %d",(int)cycleDMA);

		// Print cipher text
		//printf("\n\rciphertext : ");
		//for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}

		/*end transaction*/
		Write_Register(DLYB_ADDR,0x1);
	}
	printf("\n\rEnd of the AES sequence!!\n\r");
}


void Compute_AES(void)
{
	AES_KEY_OSSL keydma;
	uint8_t ptArray[16];
	uint8_t ctArray[16];
	uint8_t keyArray[16];
	uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};
	uint32_t temp = 0;
	uint32_t cycleAES = 0;
	uint32_t cycleDMA = 0;
	uint32_t nSample = 0;

	printf("****Start Openssl AES encryption****\n\r");

	// Generate key and print it
	printf("\n\rkey : ");
	for(int k_i = 0 ; k_i < 16 ; k_i++){keyArray[k_i] = exKeyArray[k_i];/*((uint8_t)rand())*/printf("%02x",keyArray[k_i]);}
	printf("\n\r");

	//AES key expansion
	AES_set_encrypt_key(keyArray,128,&keydma);

	while(1)
	{
		while(Read_Register(DDR_BASE_ADDR) != 1){
			nSample = Read_Register(DDR_BASE_ADDR+0x30);
		} // Wait for CA7 *init*

		//printf("\n\rnSample: %d",nSample);

		//import plaintext
		for(int i = 0 ; i < 4 ; i++)
		{
			temp = Read_Register(DDR_BASE_ADDR+0x4+i*4);

			for(int j = 0 ; j < 4 ; j++)
			{
				ptArray[i*4+j] = temp >> (24-j*8);
			}
		}
		//printf("\n\rplaintext : ");
		//for(int u = 0; u<16 ; u++){printf("%02x",ptArray[u]);}
		//printf("\n\r");

		Write_Register(DDR_BASE_ADDR,0x2); //send *ready* to CA7

		while(Read_Register(DDR_BASE_ADDR) != 3){} // Wait for CA7 *start*

		/* STart DMA transaction */
		transferComplete = 0;

		// Launch DMA
		if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, nSample) != HAL_OK){Error_Handler();}

		//clear DWT cycle counter
		*DWT_CYCCNT = 0;

		for(int i = 0 ; i < 30 ; i++);
		AES_encrypt(ptArray,ctArray,&keydma);
		for(int i = 0 ; i < 30 ; i++);

		// Get DWT cycle counter value
		cycleAES = *DWT_CYCCNT;

		while(transferComplete == 0){}

		// Get DWT cycle counter value
		cycleDMA = *DWT_CYCCNT;

		//export local DMA samples to DRAM memory for CA7 to pick up
		for(int i = 0 ; i < nSample ; i++)
		{
			//printf("test");
			Write_Register(DDR_BASE_ADDR+0x40+i*4,aDST_Buffer[i]);
		}

		Write_Register(DDR_BASE_ADDR,0x4); //send *end* to CA7

		//export ciphertext
		for(int i = 0 ; i < 4 ; i++)
		{
			Write_Register(DDR_BASE_ADDR+0x14+i*4,((ctArray[i*4+0]<<24)&0xff000000) | ((ctArray[i*4+1]<<16)&0xff0000) | ((ctArray[i*4+2]<<8)&0xff00) | (ctArray[i*4+3]&0xff));
		}

		//printf("\n\rciphertext : ");
		//for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}
		//printf("\n\r");
		//printf ("\n\rnb cycles AES : %d",(int)cycleAES);
		//printf ("\n\rnb cycles DMA: %d",(int)cycleDMA);
	}

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

void printBits(uint8_t size, uint32_t data)
{
    for (int i = size-1; i >= 0; i--) {
            printf("%lu", (data>>i)  & 1);
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
	  BSP_LED_Toggle(LED7);
	  HAL_Delay(100);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
