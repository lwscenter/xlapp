#include "sys.h"
#include "miic.h"

#if 0
#define MPU_IIC_Delay()     delay_us(2)


#define MPU_SDA_OUT()       /*MPU_set_gpio_dir(GPIO_Mode_Out_OD)*/
#define MPU_SDA_IN()        /*MPU_set_gpio_dir(GPIO_Mode_IN_FLOATING)*/

#define MPU_IIC_SCL         IIC_SCL
#define MPU_IIC_SDA         IIC_SDA
#define MPU_READ_SDA        READ_SDA

//��ʼ��IIC
void MPU_IIC_Init(void)
{
    GPIO_InitTypeDef def;
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_I2C_PORT | RCC_APB2Periph_AFIO,ENABLE);
    def.GPIO_Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
    /*def.GPIO_Mode = GPIO_Mode_Out_OD;*/
    /*def.GPIO_Mode = GPIO_Mode_Out_PP;*/
    def.GPIO_Mode = GPIO_Mode_Out_OD;
    def.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_PORT_I2C, &def);
    /**     @arg GPIO_Remap_SWJ_NoJTRST      : Full SWJ Enabled (JTAG-DP + SW-DP) but without JTRST
     *     @arg GPIO_Remap_SWJ_JTAGDisable  : JTAG-DP Disabled and SW-DP Enabled
     *     @arg GPIO_Remap_SWJ_Disable      : Full SWJ Disabled (JTAG-DP + SW-DP)*/
    GPIO_SetBits(GPIO_PORT_I2C, I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN);
}
//����IIC��ʼ�ź�
void MPU_IIC_Start(void)
{
    MPU_SDA_OUT();     //sda�����
    MPU_IIC_SDA=1;
    MPU_IIC_SCL=1;
    MPU_IIC_Delay();
    MPU_IIC_SDA=0;//START:when CLK is high,DATA change form high to low
    MPU_IIC_Delay();
    MPU_IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ��������
}
//����IICֹͣ�ź�
void MPU_IIC_Stop(void)
{
    MPU_SDA_OUT();//sda�����
    MPU_IIC_SCL=0;
    MPU_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
    MPU_IIC_Delay();
    MPU_IIC_SCL=1;
    MPU_IIC_SDA=1;//����I2C���߽����ź�
    MPU_IIC_Delay();
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t MPU_IIC_Wait_Ack(void)
{
    uint8_t ucErrTime=0;
    MPU_SDA_IN();      //SDA����Ϊ����
    MPU_IIC_SDA=1;MPU_IIC_Delay();
    MPU_IIC_SCL=1;MPU_IIC_Delay();
    while(MPU_READ_SDA) {
        ucErrTime++;
        if(ucErrTime>250)
        {
            MPU_IIC_Stop();
            return 1;
        }
    }
    MPU_IIC_SCL=0;//ʱ�����0
    return 0;
}
//����ACKӦ��
void MPU_IIC_Ack(void)
{
    MPU_IIC_SCL=0;
    MPU_SDA_OUT();
    MPU_IIC_SDA=0;
    MPU_IIC_Delay();
    MPU_IIC_SCL=1;
    MPU_IIC_Delay();
    MPU_IIC_SCL=0;
}
//������ACKӦ��
void MPU_IIC_NAck(void)
{
    MPU_IIC_SCL=0;
    MPU_SDA_OUT();
    MPU_IIC_SDA=1;
    MPU_IIC_Delay();
    MPU_IIC_SCL=1;
    MPU_IIC_Delay();
    MPU_IIC_SCL=0;
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void MPU_IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    MPU_SDA_OUT();
    MPU_IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {
        MPU_IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        MPU_IIC_SCL=1;
        MPU_IIC_Delay();
        MPU_IIC_SCL=0;
        MPU_IIC_Delay();
    }
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
uint8_t MPU_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    MPU_SDA_IN();//SDA����Ϊ����
    for(i=0; i < 8;i++) {
        MPU_IIC_SCL=0;
        MPU_IIC_Delay();
        MPU_IIC_SCL=1;
        receive<<=1;
        if(MPU_READ_SDA)receive++;
        MPU_IIC_Delay();
    }
    if (!ack)
        MPU_IIC_NAck();//����nACK
    else
        MPU_IIC_Ack(); //����ACK
    return receive;
}
#endif
