/**
******************************************************************************
* @file           : main_cortex_M4.c
* @brief          : CA7-to-CM4 side-channel attack using DLYB blocks in
* 					STM32MP1 SoCs
******************************************************************************
* @attention
*
* Copyright (c) Joseph Gravellier 2020 Thales.
* Email: joseph.gravellier@gmail.com
* All rights reserved.
*
*
******************************************************************************
*/



/* Includes ------------------------------------------------------------------*/
#include <main_cortex-M4.h>

/* Private define ------------------------------------------------------------*/
#define AES_BUFFER_SIZE 1000
#define DDR_BASE_ADDR 0xD0000000
#define DLYB_ADDR 0x58008000

/* Private variables ---------------------------------------------------------*/
DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
static uint32_t aDST_Buffer[AES_BUFFER_SIZE];
static __IO uint32_t transferErrorDetected; /* Set to 1 if an error transfer is detected */
uint32_t transferComplete; /* Set to 1 if DMA transfer has ended */


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
	BSP_LED_Init(LED7);
	BSP_LED_On(LED7); //to check if the program has been successfully launched by CA7


	/* Init DMA for DLYB to memory data transfers */
	transferErrorDetected = 0;
	MX_DMA_Init();
	/* Select Callbacks functions called after Transfer complete and Transfer error */
	HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
	HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

	/* Start AES encryption app */
	Compute_AES();

	while(1){}

}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}


/**
  * @brief Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  * hdma_memtomem_dma2_stream0
  */
void MX_DMA_Init(void)
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
void TransferComplete(DMA_HandleTypeDef *DmaHandle)
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
void TransferError(DMA_HandleTypeDef *DmaHandle)
{
	transferErrorDetected = 1;
	Error_Handler();
}

/*
  * @brief  OpenSSL AES computation
  * @note   This function imports a CA7 plaintext from a shared memory space, it then
  *         computes AES and save the ciphertext inside the memory. It also handles DMA DLYB sampling
  * @retval None
 */
void Compute_AES(void)
{
	AES_KEY_OSSL keydma;
	uint8_t ptArray[16];
	uint8_t ctArray[16];
	uint8_t keyArray[16];
	uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};
	uint32_t temp = 0;
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

		Write_Register(DDR_BASE_ADDR,0x2); //send *ready* to CA7

		while(Read_Register(DDR_BASE_ADDR) != 3){} // Wait for CA7 *start*

		/* STart DMA transaction */
		transferComplete = 0;

		// Launch DMA
		if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, nSample) != HAL_OK){Error_Handler();}

		for(int i = 0 ; i < 30 ; i++); //delay for visualization
		AES_encrypt(ptArray,ctArray,&keydma);
		for(int i = 0 ; i < 30 ; i++);

		//wait for DMA to end transfer
		while(transferComplete == 0){}

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



/************************ (C) COPYRIGHT Thales *****END OF FILE****/
