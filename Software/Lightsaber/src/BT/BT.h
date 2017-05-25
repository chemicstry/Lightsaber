#ifndef _BT_H_
#define _BT_H_

#include "stm32f10x.h"

#define BT_TX_PERIPH RCC_APB2Periph_GPIOA
#define BT_TX_PORT GPIOA
#define BT_TX_PIN GPIO_Pin_2
#define BT_RX_PERIPH RCC_APB2Periph_GPIOA
#define BT_RX_PORT GPIOA
#define BT_RX_PIN GPIO_Pin_3
#define BT_CMD_PERIPH RCC_APB2Periph_GPIOA
#define BT_CMD_PORT GPIOA
#define BT_CMD_PIN GPIO_Pin_1

#define BT_USART_PERIPH RCC_APB1Periph_USART2
#define BT_USART USART2
#define BT_USART_IRQ USART2_IRQn
#define BT_IRQ_HANDLER USART2_IRQHandler

#define BT_TX_DMA_CHANNEL DMA1_Channel7
#define BT_RX_DMA_CHANNEL DMA1_Channel6

#define BT_TX_BUF_SIZE 100
#define BT_RX_BUF_SIZE 100

typedef void (*BTRXCB)(uint8_t* data, uint32_t len);

class BT
{
    // Singleton
public:
    static BT& getInstance()
    {
       static BT instance;

       return instance;
    }

public:
    void Init();
    void SetCMDMode(bool en);
    void Send(char* str);
    void Send(uint8_t* data, uint32_t len);
    void Recv(uint8_t data);
    void SetRXCB(BTRXCB rxCB);
    void Update();

private:
    uint8_t _txBuf[BT_TX_BUF_SIZE];
    uint32_t _txi;
    uint8_t _rxBuf[BT_RX_BUF_SIZE];
    uint32_t _rxi;
    BTRXCB _rxCB;
    bool _cmdMode;
};

#endif
