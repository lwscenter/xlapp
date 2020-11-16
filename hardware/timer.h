#ifndef TIMER_8HLCY5NV
#define TIMER_8HLCY5NV
#ifdef __cplusplus
extern "C" { /*}*/
#endif

void tick_init(void);

void tim3_int_init(uint16_t arr, uint16_t psc);

void delay_ms(uint16_t nms);

void delay_us(uint32_t nms);

void tick_uninit(void);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: TIMER_8HLCY5NV */
