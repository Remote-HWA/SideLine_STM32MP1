/**
******************************************************************************
* @file           : main_cortex_M4.c
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



/* Includes ------------------------------------------------------------------*/
#include "main_cortex-M4.h"

/* Private define ------------------------------------------------------------*/
#define AES_BUFFER_SIZE 8000
#define DDR_BASE_ADDR 0x10040000
#define DLYB_ADDR 0x58008000

/* Private variables ---------------------------------------------------------*/
DMA_HandleTypeDef hdma_memtomem_dma2_stream0;


//static uint32_t aDST_Buffer[AES_BUFFER_SIZE];
#define aDST_Buffer (*(volatile uint32_t (*)[AES_BUFFER_SIZE])(DDR_BASE_ADDR+0x40))

static __IO uint32_t transferErrorDetected;  /* Set to 1 if an error transfer is detected */
uint32_t transferComplete; /* Set to 1 if DMA transfer has ended */
uint8_t selectCrypto = 0;

/* USER CODE BEGIN PV */
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

	for(int i = 0 ; i < 50 ; i++)
	{
		BSP_LED_Toggle(LED7); //to check if the program has been successfully launched by CA7
		HAL_Delay(40);
	}

	/* Init DMA for DLYB to memory data transfers */
	transferErrorDetected = 0;
	MX_DMA_Init();
	/* Select Callbacks functions called after Transfer complete and Transfer error */
	HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
	HAL_DMA_RegisterCallback(&hdma_memtomem_dma2_stream0, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

	/* Start AES encryption app */
	Compute_AES();

	while(1){	}

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
	struct AES_ctx ctx;
	uint8_t ptArray[16];
	uint8_t ctArray[16];
	uint8_t exKeyArray[16] = {0x2d,0xcf,0x46,0x29,0x04,0xb4,0x78,0xd8,0x68,0xa7,0xff,0x3f,0x2b,0xf1,0xfc,0xd9};
	uint32_t temp32to8 = 0;
	uint32_t temp8to32 = 0;
	uint32_t nSample = 0;
	uint32_t Mode = 0;
	STRUCT_AES aes_struct;
	uint32_t ret = 0;
	uint8_t refreshExpansion = 0;

	//AES key expansion

	if(selectCrypto == OPENSSL){
	AES_set_encrypt_key(exKeyArray,128,&keydma);}
	else if(selectCrypto == TINYAES)
	{AES_init_ctx(&ctx, exKeyArray);}
	else if(selectCrypto == MASKEDAES){
	KeyExpansion(exKeyArray);}
	else if(selectCrypto == HIGHERORDER)
	{}
	else
	{Error_Handler();}

	while(1)
	{
		while(Read_Register(DDR_BASE_ADDR) != 1){
			nSample = Read_Register(DDR_BASE_ADDR+0x30);
			selectCrypto = Read_Register(DDR_BASE_ADDR+0x34);

			if((Read_Register(DDR_BASE_ADDR) == 0x666) && (refreshExpansion == 0))
			{
				if(selectCrypto == OPENSSL){
				AES_set_encrypt_key(exKeyArray,128,&keydma);}
				else if(selectCrypto == TINYAES)
				{AES_init_ctx(&ctx, exKeyArray);}
				else if(selectCrypto == MASKEDAES){
				KeyExpansion(exKeyArray);}
				else if(selectCrypto == HIGHERORDER)
				{}
				else
				{Error_Handler();}
				refreshExpansion = 1;
			}
		} // Wait for CA7 *init*

		refreshExpansion = 0;

		//import plaintext
		for(int u = 0 ; u < 4 ; u++)
		{
			temp32to8 = Read_Register(DDR_BASE_ADDR+0x4+u*4);

			for(int v = 0 ; v < 4 ; v++)
			{
				ptArray[u*4+v] = (temp32to8 >> (24-v*8)) & 0xff;
			}
		}

		Write_Register(DDR_BASE_ADDR,0x2); //send *ready* to CA7

		while(Read_Register(DDR_BASE_ADDR) != 3){} // Wait for CA7 *start*

		/* STart DMA transaction */
		transferComplete = 0;

		// Launch DMA
		if (HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, DLYB_ADDR + 0x4, (uint32_t)&aDST_Buffer, nSample) != HAL_OK){Error_Handler();}

		if(selectCrypto == TINYAES)
		{
			for(int i = 0 ; i < 1000 ; i++); //delay for visualization
			AES_ECB_encrypt(&ctx,ptArray); //tiny AES
			for(int i = 0 ; i < 1000 ; i++);
		}
		else if(selectCrypto == OPENSSL)
		{
			for(int i = 0 ; i < 30 ; i++);
			AES_encrypt(ptArray,ctArray,&keydma); //openSSL AES
			for(int i = 0 ; i < 30 ; i++);
		}
		else if(selectCrypto == MASKEDAES)
		{
			for(int i = 0 ; i < 30 ; i++); //delay for visualization
			aes128(ptArray); //masked AES
			for(int i = 0 ; i < 30 ; i++);
		}
		else if(selectCrypto == HIGHERORDER)
		{
			for(int i = 0 ; i < 1000 ; i++); //delay for visualization
			Encrypt(ctArray,ptArray,exKeyArray); //higher order AES
			for(int i = 0 ; i < 1000 ; i++);
		}
		else if(selectCrypto == ANSSIAES)
		{
			for(int i = 0 ; i < 30 ; i++);
			Mode = MODE_KEYINIT|MODE_AESINIT_ENC|MODE_ENC;
			ret = aes(Mode, &aes_struct,exKeyArray, ptArray, ctArray, NULL, NULL);
			for(int i = 0 ; i < 30 ; i++);
		}

		//wait for DMA to end transfer
		while(transferComplete == 0){}

		//export local DMA samples to DRAM memory for CA7 to pick up
		/*for(int i = 0 ; i < nSample ; i++)
		{
			//printf("test");
			Write_Register(DDR_BASE_ADDR+0x40+i*4,aDST_Buffer[i]);
		}*/

		//export ciphertext
		if((selectCrypto == MASKEDAES) || (selectCrypto == TINYAES))
		{
			for(int u = 0 ; u < 4 ; u++)
			{
				temp8to32=0;
				for(int v = 0 ; v < 4 ; v++){
					temp8to32 |= (uint32_t)(ptArray[u*4+v])<<(24-8*v);
				}
				Write_Register(DDR_BASE_ADDR+0x14+u*4,temp8to32);
			}
		}
		else
		{
			for(int u = 0 ; u < 4 ; u++)
			{
				temp8to32=0;
				for(int v = 0 ; v < 4 ; v++){
					temp8to32 |= (uint32_t)(ctArray[u*4+v])<<(24-8*v);
				}
				Write_Register(DDR_BASE_ADDR+0x14+u*4,temp8to32);
			}
		}
		Write_Register(DDR_BASE_ADDR,0x4); //send *end* to CA7

	
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
