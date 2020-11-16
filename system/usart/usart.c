#include "sys.h"
#include "system.h"
#include "protocol.h"
#include "stdarg.h"
#include "usart.h"

#if (COM_SERIAL_PORT == 2)

#define COM_SP                  USART2
#define COM_SP_GPIO             GPIOA
#define COM_SP_TX               GPIO_Pin_2
#define COM_SP_RX               GPIO_Pin_3
#define COM_SP_APB2_PERIPH_GPIO RCC_APB2Periph_GPIOA
#define COM_SP_APB2_PERIPH_COM  RCC_APB1Periph_USART2
#define COM_SP_IRQ              USART2_IRQn


#elif (COM_SERIAL_PORT == 1)
#define COM_SP                  USART1
#define COM_SP_GPIO             GPIOA
#define COM_SP_TX               GPIO_Pin_9
#define COM_SP_RX               GPIO_Pin_10
#define COM_SP_APB2_PERIPH_GPIO RCC_APB2Periph_GPIOA
#define COM_SP_APB2_PERIPH_COM  RCC_APB2Periph_USART1
#define COM_SP_IRQ              USART1_IRQn

#endif
void printf2(char *fmt, ...)
{
    static char buf[128];
    uint8_t i = 0;
    va_list ap;
    for (i = 0; i < sizeof(buf); i++) {
        buf[i] = 0;
    }
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    i = 0;
    while (buf[i]) {
        while((DBG_UART->SR & 0X40) == 0);
        DBG_UART->DR = buf[i];
        i++;
    }
}
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}

int fputc(int ch, FILE *f)
{
    while((COM_SP->SR & 0X40) == 0);
    COM_SP->DR = (unsigned char) ch;

    return ch;
}

#endif


void send_to_host_c(uint8_t ch)
{
    while((COM_SP->SR & 0X40) == 0);
    COM_SP->DR = (unsigned char) ch;
}


static uint8_t rx_buf[PKT_LEN_MAX * 8];


void uart_init(int baud)
{
    //GPIO端口设置
    USART_InitTypeDef uart;
    NVIC_InitTypeDef nvic;
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
    //USART1_TX   GPIOA.9
    gpio.GPIO_Pin = GPIO_Pin_9; //PA.9
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);

    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 3;
    nvic.NVIC_IRQChannelSubPriority = 3;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    uart.USART_BaudRate = baud;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Tx;
    USART_DeInit(DBG_UART);

    USART_Init(DBG_UART, &uart);
    USART_Cmd(DBG_UART, ENABLE);
    /*USART_ITConfig(DBG_UART, USART_IT_RXNE, ENABLE);*/

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    USART_DeInit(COM_SP);

    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Pin = COM_SP_TX;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(COM_SP_GPIO, &gpio);

    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Pin = COM_SP_RX;
    GPIO_Init(COM_SP_GPIO, &gpio);

    uart.USART_BaudRate = baud;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_WordLength = USART_WordLength_8b;
    USART_Init(COM_SP, &uart);
    USART_Cmd(COM_SP, ENABLE);
    USART_ITConfig(COM_SP, USART_IT_RXNE, ENABLE);


    nvic.NVIC_IRQChannel = COM_SP_IRQ;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic);

    queue_init(&prot_data, rx_buf, sizeof(rx_buf));
}
#if 1

enum {
    sta_s_head,
    sta_s_randv,
    sta_s_cmd,
    sta_s_tail,
};

//unsigned int Code_Num = 0;	//记录接收到的指令数


/*#else*/
#if (COM_SERIAL_PORT == 2)
void USART2_IRQHandler(void)
#elif (COM_SERIAL_PORT == 1)
void USART1_IRQHandler(void)
#else
void USART3_IRQHandler(void)
#endif
{
    uint8_t tmp;
#if USE_NEW_PROT
    static uint8_t randno, cmd;
    static uint8_t state = sta_s_head;
#endif
    if(USART_GetITStatus(COM_SP, USART_IT_RXNE) != RESET) {
        tmp = USART_ReceiveData(COM_SP);
//				Code_Num++;	//记录接收到的指令数
#if USE_NEW_PROT
        switch (state) {
        case sta_s_head:
            if (tmp == PKT_STX)
                state = sta_s_randv;
            break;
        case sta_s_randv:
            randno = tmp ;
            state = sta_s_cmd;
            break;
        case sta_s_cmd:
            cmd = tmp;
            state = sta_s_tail;
            break;
        case sta_s_tail:
            if (tmp == PKT_ETX) {
                queue_in(&prot_data, randno);
                queue_in(&prot_data, cmd);
                ++prot_pkt_count;
                if (cmd == PORT_CMD_RFID_ACK) {
                    set_sys_rfid_ack(1);
                }
            }
            state = sta_s_head;
            break;
        }
#else
        queue_in(&prot_data, tmp);
        ++prot_pkt_count;
#endif
    }
}
#endif
