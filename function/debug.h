#ifndef DEBUG_HISUSEHS
#define DEBUG_HISUSEHS
#ifdef __cplusplus
extern "C" { /*}*/
#endif
#define APP_JUMP_ISP        0
#define DBG_UART            USART1
void printf2(char *fmt, ...);
#if DEBUG
#define dbg_printf(...)     printf2( __VA_ARGS__)
#define dbg_putc(_v)        do {\
    while((DBG_UART->SR&0X40)==0);\
    DBG_UART->DR = (unsigned char)_v;\
} while (0)
#else
#define dbg_printf(...)
#define dbg_putc(_v)
#endif

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: DEBUG_HISUSEHS */
