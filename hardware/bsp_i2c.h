#ifndef BSP_I2C_ETHTLJGF
#define BSP_I2C_ETHTLJGF
#ifdef __cplusplus
extern "C" { /*}*/
#endif




#define I2C_WR	0		/* Ð´¿ØÖÆbit */
#define I2C_RD	1		/* ¶Á¿ØÖÆbit */

void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(uint8_t ack);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);
void i2c_init(void);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: BSP_I2C_ETHTLJGF */
