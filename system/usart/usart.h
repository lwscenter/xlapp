#ifndef USART_FZEG172Z
#define USART_FZEG172Z
#ifdef __cplusplus
extern "C" { /*}*/
#endif

typedef int (*fputc_fun_t)(int, FILE *);

void uart_init(int baud);
#define uart_send(_v)              { USART_SendData(USART1, (_v));\
                                            while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);}
void send_to_host_c(uint8_t ch);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: USART_FZEG172Z */



