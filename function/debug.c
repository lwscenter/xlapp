#include "sys.h"
#include "debug.h"
#if APP_JUMP_ISP
#define  IapAdr    0x1FFFF000
typedef  void (*pFunction)(void);

uint8_t JumpToApp(uint32_t Addr)
{
    if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000) {
        __set_MSP(*(__IO uint32_t*) Addr);
        ((pFunction) (*(__IO uint32_t*) (Addr + 4)))();
    }
    return 0;
}


void RCC_ClearFlag(void);
void  isp_boot(void)
{
    if (IS_RCC_FLAG(RCC_FLAG_SFTRST) != RESET) {
        JumpToApp(IapAdr);
        while(1);
    }
    RCC_ClearFlag();
}



void reset2isp(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}
isp_boot();
#endif
