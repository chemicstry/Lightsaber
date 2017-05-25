
//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "sdcard.h"
#include "ff.h"
#include "audio/WAV.h"

#define DMA_MAX_SIZE 0xFFFF
#define DMA_MAX(x)           (((x) <= DMA_MAX_SIZE)? (x):DMA_MAX_SIZE)

// ----------------------------------------------------------------------------
//
// Semihosting STM32F1 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup DAC_DualModeDMA_SineWave
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DAC_DHR12RD_Address      0x40007420
#define DAC_DHR8R1_Address      0x40007410

/* Init Structure definition */
DAC_InitTypeDef            DAC_InitStructure;
DMA_InitTypeDef            DMA_InitStructure;
TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
uint32_t Idx = 0;

const uint8_t Escalator8bit[6] = {0x55, 0x77, 0x99, 0xBB, 0xDD, 0xFF};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const uint16_t Sine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909,
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};

uint32_t DualSine12bit[32];

uint32_t CurrentPos;
int32_t AudioRemSize = 0;
#define AudioBufSize 0x100
uint8_t AudioBuf[2][AudioBufSize];
uint8_t CurrentBuf = 1;

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void GPIO_Configuration(void);
void Delay(__IO uint32_t nCount);

SD_Error sdcard_init(void)
{
    SD_Error status = SD_Init();
    SD_CardInfo SDCardInfo;

    //SD_SetDeviceMode(SD_POLLING_MODE);

    if (status != SD_OK)
    {
        trace_printf("SD_Init failed: %u\n", status);
        return status;
    }

    status = SD_GetCardInfo(&SDCardInfo);
    if (status != SD_OK)
    {
        trace_printf("SD_GetCardInfo failed: %u\n", status);
        return status;
    }

    trace_printf("SD INFO OK!\nCapacity: %u\n", SDCardInfo.CardCapacity);

    return status;
}

WAV* wav;

int
main(int argc, char* argv[])
{
    //RCC_Configuration();
    //GPIO_Configuration();
    // At this stage the system clock should have already been configured
    // at high speed.
    trace_printf("System clock: %u Hz\n", SystemCoreClock);

    FATFS fs;
    FIL fil;       /* File object */
    char line[82]; /* Line buffer */
    FRESULT fr;    /* FatFs return code */

    int status = f_mount(&fs, "0", 1);

    if (status != FR_OK)
    {
        trace_printf("Mount failed! %u\n", status);
        return 0;
    }

    /* Open a text file */
    fr = f_open(&fil, "hello.txt", FA_READ);
    if (fr) return (int)fr;

    /* Read all lines and display it */
    while (f_gets(line, sizeof(line), &fil))
        trace_printf(line);

    /* Close the file */
    f_close(&fil);

    wav = new WAV("bee32k.wav");
    FormatChunk* fmt = wav->GetFormat();

    //trace_printf("ChunkID: %.8s\n", fmt->ChunkID);
    //trace_printf("ChunkSize: %u\n", fmt->ChunkSize);
    trace_printf("AudioFormat: %u\n", fmt->AudioFormat);
    trace_printf("Channels: %u\n", fmt->Channels);
    trace_printf("SampleRate: %u\n", fmt->SampleRate);
    trace_printf("ByteRate: %u\n", fmt->ByteRate);
    trace_printf("BlockAlign: %u\n", fmt->BlockAlign);
    trace_printf("BitsPerSample: %u\n", fmt->BitsPerSample);
    trace_printf("DataSize: %u\n", wav->GetSize());

    CurrentBuf ^= 1;
    wav->Read(AudioBuf[CurrentBuf], AudioBufSize);

    /*for (int i = 0; i < AudioBufSize; ++i)
          trace_printf("%u ", AudioBuf[CurrentBuf][i]);*/

    trace_printf("Ready!\n");

    // System Clocks Configuration
    RCC_Configuration();

    // Once the DAC channel is enabled, the corresponding GPIO pin is automatically connected to the DAC converter. In order to avoid parasitic consumption, the GPIO pin should be configured in analog
    GPIO_Configuration();

    // TIM6 Configuration
    TIM_PrescalerConfig(TIM6, 0, TIM_PSCReloadMode_Update);
    TIM_SetAutoreload(TIM6, /*3271*//*1635*//*182*/SystemCoreClock/32000 - 1);
    // TIM6 TRGO selection
    TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

    // DAC channel1 Configuration
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  #if !defined STM32F10X_LD_VL && !defined STM32F10X_MD_VL
    // DMA2 channel3 configuration
    DMA_DeInit(DMA2_Channel3);
  #else
    // DMA1 channel3 configuration
    DMA_DeInit(DMA1_Channel3);
  #endif

    //CurrentPos = (uint32_t)&sfx;
    //AudioRemSize = sizeof(sfx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR12R1;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AudioBuf[CurrentBuf];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = AudioBufSize/2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    // Update the current pointer position
    //CurrentPos += DMA_MAX(AudioRemSize);

    // Update the remaining number of data to be played
    //AudioRemSize -= DMA_MAX(AudioRemSize);

    DMA_ITConfig(DMA2_Channel3, DMA_IT_TC | DMA_IT_HT, ENABLE);

  #if !defined STM32F10X_LD_VL && !defined STM32F10X_MD_VL
    DMA_Init(DMA2_Channel3, &DMA_InitStructure);
    // Enable DMA2 Channel3
    DMA_Cmd(DMA2_Channel3, ENABLE);
  #else
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    // Enable DMA1 Channel3
    DMA_Cmd(DMA1_Channel3, ENABLE);
  #endif

    // Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is automatically connected to the DAC converter.
    DAC_Cmd(DAC_Channel_1, ENABLE);

    // Enable DMA for DAC Channel1
    DAC_DMACmd(DAC_Channel_1, ENABLE);

    // TIM6 enable counter
    TIM_Cmd(TIM6, ENABLE);


    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*SD_Error status = SD_Init();
    SD_CardInfo SDCardInfo;

    SD_SetDeviceMode(SD_DMA_MODE);

    if (status == SD_OK)
    {
        trace_printf("SD OK!\n");
        if (SD_GetCardInfo(&SDCardInfo) == SD_OK)
        {
            trace_printf("SD INFO OK!\nCapacity: %u\n", SDCardInfo.CardCapacity);
        }
    }
    else
        trace_printf("SD BAD: %u!\n", status);*/



    while (1)
    {
    }
  }


  /**
    * @brief  Configures the different system clocks.
    * @param  None
    * @retval None
    */
  void RCC_Configuration(void)
  {
    /* Enable peripheral clocks ------------------------------------------------*/
  #if !defined STM32F10X_LD_VL && !defined STM32F10X_MD_VL
    /* DMA2 clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
  #else
    /* DMA1 clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  #endif
    /* GPIOA Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* DAC Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    /* TIM6 Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  }

  /**
    * @brief  Configures the different GPIO ports.
    * @param  None
    * @retval None
    */
  void GPIO_Configuration(void)
  {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Once the DAC channel is enabled, the corresponding GPIO pin is automatically
       connected to the DAC converter. In order to avoid parasitic consumption,
       the GPIO pin should be configured in analog */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA, GPIO_Pin_5);
  }

  /**
    * @brief  Inserts a delay time.
    * @param  nCount: specifies the delay time length.
    * @retval None
    */
  void Delay(__IO uint32_t nCount)
  {
    for(; nCount != 0; nCount--);
  }

extern "C" void  DMA2_Channel3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA2_IT_HT3) != RESET)
    {
        DMA_ClearITPendingBit(DMA2_IT_HT3);

        //trace_printf("INT HT\n");
    }

    /* Transfer complete interrupt */
    if (DMA_GetITStatus(DMA2_IT_TC3) != RESET)
    {
      //trace_printf("Remaining: %u\n", AudioRemSize);
      /*if (AudioRemSize <= 0)
      {
          CurrentPos = (uint32_t)&sfx;
          AudioRemSize = sizeof(sfx);
      }*/

      /* Check if the end of file has been reached */
      /*if (AudioRemSize > 0)
      {*/
        /* Clear the Interrupt flag */
        DMA_ClearITPendingBit(DMA2_IT_TC3);

        DMA_DeInit(DMA2_Channel3);

        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&AudioBuf[CurrentBuf];

        DMA_ITConfig(DMA2_Channel3, DMA_IT_TC, ENABLE);

        /* Configure the DMA Stream with the new parameters */
        DMA_Init(DMA2_Channel3, &DMA_InitStructure);

        /* Enable the I2S DMA Stream*/
        DMA_Cmd(DMA2_Channel3, ENABLE);

        CurrentBuf ^= 1;
                wav->Read(&AudioBuf[CurrentBuf], AudioBufSize);

        /* Update the current pointer position */
        //CurrentPos += DMA_MAX(AudioRemSize);

        /* Update the remaining number of data to be played */
        //AudioRemSize -= DMA_MAX(AudioRemSize);
      //}
        //trace_printf("INT TC\n");
    }

    /*trace_printf("INT\n");
    DMA_ClearITPendingBit(DMA2_IT_GL3);


    uint32_t addr = sizeof(audioBuffer) / 2;
    if (DMA_GetITStatus(DMA2_IT_HT3))
        addr = 0;

    for (uint32_t i = 0; i < sizeof(audioBuffer) / 2; ++i)
    {
        audioBuffer[addr++] = sfx[sfxPointer++];
        if (sfxPointer >= sizeof(sfx))
            sfxPointer = 0;
    }*/
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
