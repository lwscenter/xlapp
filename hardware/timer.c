#include "sys.h"
#include "system.h"
#include "timer.h"
void delay_us(uint32_t nus)
{
    u32 temp;
    SysTick->LOAD = 9*nus;
    SysTick->VAL=0X00;//清空计数器
    SysTick->CTRL=0X01;//使能，减到零是无动作，采用外部时钟源
    do
    {
        temp=SysTick->CTRL;//读取当前倒计数值

    }while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达
    SysTick->CTRL=0x00; //关闭计数器
    SysTick->VAL =0X00; //清空计数器
}

void delay_ms(uint16_t nms)
{
    u32 temp;
    SysTick->LOAD = 9000*nms;
    SysTick->VAL=0X00;//清空计数器
    SysTick->CTRL=0X01;//使能，减到零是无动作，采用外部时钟源
    do
    {
        temp=SysTick->CTRL;//读取当前倒计数值
    }while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达
    SysTick->CTRL=0x00; //关闭计数器
    SysTick->VAL =0X00; //清空计数器
}
void tim3_int_init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}
/*
static void one_us_init(uint8_t SYSCLK)
{
    SysTick->CTRL &= ~(1<<2);外部时钟源
    fac_us = SYSCLK / 8; 1/8
}
*/
void tick_init(void)
{
    if (SysTick_Config(SystemCoreClock/1000)) {
        while (1);
    }
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
    /*one_us_init(SystemCoreClock / 1000000);*/
}
void tick_uninit(void)
{
  SysTick->CTRL  = 0;
}

void key_ticks_run(void);
void sched_task_ticks(void);
void motor_pwm_count(void);
void TIM3_IRQHandler(void)
{
    static uint16_t ticks = 0;
    if(TIM3->SR & 1) {
        motor_pwm_count();
        TIM3->SR &= ~(1 << 0);//clear interrupt
        sched_task_ticks();
        if (++ticks > 1000) {
            get_sys_ticks++;
            ticks = 0;
        }
    }
}
