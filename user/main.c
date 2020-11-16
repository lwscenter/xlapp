#include "sys.h"
#include "sched.h"
#include "bsp.h"
#include "system.h"
#define BOOTLOADER_SIZE     (18 * 1024)
void delay_ms_ex(int time);
int main(void)
{
#if USE_BOOT_LOADER
    SCB->VTOR = FLASH_BASE | BOOTLOADER_SIZE;
    __enable_irq();
#endif
    bsp_init();
    sys_init();
    WDG_INIT();
    sched_task_init();
    task_start( ,WDG_FEED());
}
