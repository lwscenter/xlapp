#ifndef DS18B20_SX371MTW
#define DS18B20_SX371MTW
#ifdef __cplusplus
extern "C" { /*}*/
#endif



//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK MiniSTM32������
//DS18B20��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/12
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
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
