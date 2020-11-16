#ifndef DS18B20_SX371MTW
#define DS18B20_SX371MTW
#ifdef __cplusplus
extern "C" { /*}*/
#endif



//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK MiniSTM32开发板
//DS18B20驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/12
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

   	
#if 0
uint8_t ds18b20_init(uint8_t idx);

short DS18B20_Get_Temp(uint8_t idx);
#else
uint8_t ds18b20_1_init(void);
short ds18b20_1_get_temp(void);
uint8_t ds18b20_0_init(void);
short ds18b20_0_get_temp(void);
u8 DS18B20_1_self_check(void);
u8 DS18B20_0_self_check(void);
#endif









#ifdef __cplusplus
}
#endif
#endif /* end of include guard: DS18B20_SX371MTW */
