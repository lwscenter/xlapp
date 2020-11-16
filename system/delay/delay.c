#include "sys.h"
#include "delay.h"
void tick_init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick_Config(SystemCoreClock / 1000);
}

void delay_ms(int delay)
{
    int i;
    while (delay--) {
        for (i = 0; i < 5000; ++i);
    }
}
