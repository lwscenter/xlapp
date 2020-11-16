#ifndef I2CX_351W2GYX
#define I2CX_351W2GYX
#ifdef __cplusplus
extern "C" { /*}*/
#endif

enum {
    NACK = 0,
    ACK = 1,
};


void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_send_byte(u8 txd);
u8 i2c_read_byte(unsigned char ack);
u8 i2c_wait_ack(void);
void i2c_ack(void);
void i2c_nack(void);



#ifdef __cplusplus
}
#endif
#endif /* end of include guard: I2CX_351W2GYX */
