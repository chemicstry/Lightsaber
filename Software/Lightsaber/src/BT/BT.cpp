#include "BT.h"
#include "string.h"
#include "diag/Trace.h"

void BT::Init()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(BT_USART_PERIPH, ENABLE);
    RCC_APB2PeriphClockCmd(BT_TX_PERIPH | BT_RX_PERIPH | BT_CMD_PERIPH, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = BT_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(BT_TX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = BT_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BT_RX_PORT, &GPIO_InitStructure);

    // Setup CMD pin
    GPIO_InitStructure.GPIO_Pin = BT_CMD_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BT_CMD_PORT, &GPIO_InitStructure);

    // Enable CMD mode
    SetCMDMode(true);

    // Setup RX DMA
    /*DMA_DeInit(BT_RX_DMA_CHANNEL);
    DMA_InitStructure.DMA_PeripheralBaseAddr = &BT_USART->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = _rxBuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = BT_RX_BUF_SIZE;
    DMA_Init(BT_RX_DMA_CHANNEL, &DMA_InitStructure);*/

    // Setup USART peripheral
    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(BT_USART, &USART_InitStructure);

    // Enable RX interrupt
    USART_ITConfig(BT_USART, USART_IT_RXNE, ENABLE);

    // Enable global interrupt
    NVIC_EnableIRQ(BT_USART_IRQ);

    // Enable USART
    USART_Cmd(BT_USART, ENABLE);

    _rxi = 0;
    _txi = 0;
    _rxCB = NULL;

    //USART_DMACmd(BT_USART, USART_DMAReq_Tx, ENABLE);
}

void BT::SetCMDMode(bool en)
{
    if (en)
        GPIO_SetBits(BT_CMD_PORT, BT_CMD_PIN);
    else
        GPIO_ResetBits(BT_CMD_PORT, BT_CMD_PIN);

    _cmdMode = en;
}

void BT::Send(char* str)
{
    Send((uint8_t*)str, strlen(str));
}

void BT::Send(uint8_t* data, uint32_t len)
{
    trace_printf("BT::Send(): Start %#010x\n", &BT_USART->DR);

    if (!len)
        return;

    // Copy data to tx buffer
    memcpy(_txBuf, data, len);

    DMA_ClearFlag(DMA1_FLAG_TC7 | DMA1_FLAG_TE7 | DMA1_FLAG_HT7 | DMA1_FLAG_GL7);

    // Disable DMA
    DMA_Cmd(BT_TX_DMA_CHANNEL, DISABLE);

    // DMA Struct
    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);

    // Setup TX DMA
    DMA_DeInit(BT_TX_DMA_CHANNEL);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&BT_USART->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_txBuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = len;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(BT_TX_DMA_CHANNEL, &DMA_InitStructure);

    // Enable DMA
    DMA_Cmd(BT_TX_DMA_CHANNEL, ENABLE);

    // Start transfer
    USART_DMACmd(BT_USART, USART_DMAReq_Tx, ENABLE);
    /*for (uint32_t i = 0; i < len; ++i)
    {
        USART_SendData(BT_USART, data[i]);

        // Wait until transmission is complete
        while(USART_GetFlagStatus(BT_USART, USART_FLAG_TXE) == RESET) {}
    }*/

    //trace_printf("BT::Send(): End\n");
}

void BT::Recv(uint8_t data)
{
    _rxBuf[_rxi++] = data;
}

void BT::SetRXCB(BTRXCB rxCB)
{
	_rxCB = rxCB;
}

void BT::Update()
{
    if (_rxi >= 2 && _rxBuf[_rxi-1] == '\n')
    {
        _rxBuf[_rxi-2] = '\0';
        trace_printf("BT: %s\n", _rxBuf);

        if (_rxCB)
        	_rxCB(_rxBuf, _rxi);

        _rxi = 0;
    }
}

extern "C" void BT_IRQ_HANDLER(void)
{
    // RXNE handler
    if(USART_GetITStatus(BT_USART, USART_IT_RXNE) != RESET)
    {
        BT::getInstance().Recv(USART_ReceiveData(BT_USART));
    }
}
