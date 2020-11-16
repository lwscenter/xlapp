#include "sys.h"
#include "bsp.h"
#include "i2cx.h"
void delay_us(uint32_t time);

#define TIME                20
#define IICDelay()          delay_us(TIME)

#define SDA_IN()                i2c_sda_gpio(GPIO_Mode_IPU)
#define SDA_OUT()               i2c_sda_gpio(GPIO_Mode_Out_PP)

static void i2c_sda_gpio(uint8_t dir)
{
    GPIO_InitTypeDef  def;
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_I2C_PORT, ENABLE);

    def.GPIO_Pin = I2C_SDA_GPIO_PIN;
    def.GPIO_Mode = (GPIOMode_TypeDef)dir;
    def.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIO_PORT_I2C, &def);
}

/* 初始化I2C */
void i2c_init(void)
{
	GPIO_InitTypeDef def;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_I2C_PORT | RCC_APB2Periph_AFIO,ENABLE);
	def.GPIO_Pin = I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN;
	/*def.GPIO_Mode = GPIO_Mode_Out_OD;*/
    def.GPIO_Mode = GPIO_Mode_Out_PP;
	def.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PORT_I2C, &def);
  /**     @arg GPIO_Remap_SWJ_NoJTRST      : Full SWJ Enabled (JTAG-DP + SW-DP) but without JTRST
  *     @arg GPIO_Remap_SWJ_JTAGDisable  : JTAG-DP Disabled and SW-DP Enabled
  *     @arg GPIO_Remap_SWJ_Disable      : Full SWJ Disabled (JTAG-DP + SW-DP)*/
	GPIO_SetBits(GPIO_PORT_I2C, I2C_SCL_GPIO_PIN | I2C_SDA_GPIO_PIN);

	/* 在开始传输数据之前, 先让SDA和SCL都拉高 */
	IIC_SCL = HIGH;
	IIC_SDA = HIGH;
}

/* 产生I2C的起始信号 */
void i2c_start(void)
{
	/* 设置SDA线为输出 */
	SDA_OUT();

	/* 在开始传输数据之前, 先让SDA和SCL都拉高 */
	IIC_SDA = HIGH;
	IIC_SCL = HIGH;

    IICDelay();

	/* 根据I2C总线定义: SCL为高时, 数据由高跳变至低表示开始信号 */
	IIC_SDA = LOW;
    IICDelay();
	/* 
	   SCL在低的时候, 是不传送任何数据, 也不作为开始和结束的条件, 
	   所以这样我们可以开始数据的发送而不会导致产生开始或者结束信号 
	   这个就是所谓的钳住I2C总线
	   */
	IIC_SCL = LOW;
    IICDelay();
}

/* 产生I2C停止信号 */
void i2c_stop(void)
{
	SDA_OUT();//sda线输出

	/* 
	   先让SCL拉低这样才能将SDA切换至低的状态而不导致重复产生开始信号 
	   还记得前面对I2C Start的注释吗?就是SCL为高的时候, SDA由高变低
	   */
	IIC_SCL = LOW;

	/* 
	   前面已经将SCL拉低了, 所以这里我们就可以肆无忌惮的将SDA拉低, 
	   为产生结束信号做好准备
	   */
	IIC_SDA = LOW;

    IICDelay();
	/* Okay, 我们开始产生结束信号: 首先需要将SCL拉高(为什么要拉高? 因为I2C总线规定了
	   只有在SCL为高的时候才是传输数据或者产生开始/结束信号的有效时间) */
	IIC_SCL = HIGH; 

	/* 好了, 前面已经提前将SDA拉低了, 所以这里我们只要简单的将SDA置为高就完成了 */
	IIC_SDA = HIGH;
    IICDelay();
}

/*
   应答信号的意义:
   Master每发送完8bit数据后需要等待Slave的ACK
   也就是在第9个Clock, 如果从(Slave)IC发出ACK, 那么SDA会被拉低
   如果没有ACK, SDA会被置高,这样会导致Master发出Restart或者Stop流程
   */
/* 
   等待应答信号到来
   */
u8 i2c_wait_ack(void)
{
	u8 ucErrTime = 0;
	IIC_SDA = HIGH;
    IICDelay();
	/* 设置SDA为输入模式, 以便读取SDA上的ACK */
	SDA_IN();
	/* 置SCL为高, 进行读取操作 */
	IIC_SCL = HIGH;
    IICDelay();
	while (READ_SDA) {
		ucErrTime++;
		if(ucErrTime > 250)
		{//指定时间内没有收到从IC的ACK, 进行超时处理
			return 1;
		}
	}
	/*
	   好了, 这里表示没有超时并且读取到了SDA为低, 表示收到ACK确认了 
	   那么就可以收工回家, 设置一下SCL为低电平(为啥? 因为有效操作都是在
	   SCL为高的时候才进行的)
	   */
	IIC_SCL = LOW;
	return 0;
}

/*
   发出ACK确认的操作
   发出这个操作的意义在于让从IC知道我们已经收到数据了
   这样, 从IC就不会进行Restart或者Stop流程
   */
void i2c_ack(void)
{
	/* 第一步先让SCL拉低先, 避免SDA变化影响I2C */
	IIC_SCL = LOW;
	/* 然后我们设置一下SDA为输出模式, 准备输出数据 */
	SDA_OUT();
	/* SDA拉低, 这样就将确认信号放到总线上, 就差时钟信号了 */
	IIC_SDA = LOW;
    IICDelay();
	/* SCL拉高, 产生必备的有效操作时钟信号 */
	IIC_SCL = HIGH;
    IICDelay();
	/*
	   前面延时了一会了, 时钟差不多了, 那么就结束ACK信号
	   总不能一直占着总线不放吧 */
	IIC_SCL = LOW;
}

/*
   对于NACK, I2C总线是这样定义的:
   当在第9个时钟脉冲的时候SDA线保持高电平, 就被定义为NACK.
   Master要么产生Stop条件来放弃此次传输,要么重复Start条件来
   发起一个新的开始
   */
void i2c_nack(void)
{
	IIC_SCL = LOW;
	SDA_OUT();
	/* 根据定义, 拉高SDA, 作为NACK的定义 */
	IIC_SDA = HIGH;
    IICDelay();
	/* 置为高电平, 发送NACK信号出去 */
	IIC_SCL = HIGH;
    IICDelay();
	/* SCL拉低, 发送完毕 */
	IIC_SCL = LOW;
}

void i2c_send_byte(u8 txd)
{
	u8 t;
	/* 既然开始了数据的发送, 那么先将SDA口切换为输出模式先 */
	SDA_OUT();

	/* 时钟信号拉低, 将数据准备好再拉高进行传输 */
	IIC_SCL = LOW;
	for(t = 0; t < 8; t++) {
		/* I2C数据是按照大端传输的, 也就是高位先传输 */
		if((txd&0x80) >> 7)
			IIC_SDA = HIGH;
		else
			IIC_SDA = LOW;
		txd <<= 1; 
		/* 做一下延时是有必要的 */
        IICDelay();
		/* SCL拉高传送数据 */
		IIC_SCL = HIGH;
        IICDelay();
		/* 拉低SCL, 传输完毕 */
		IIC_SCL = LOW;	
        IICDelay();
	}
}

u8 i2c_read_byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	/* 切换SDA口为输入模式, 准备读取数据 */
	SDA_IN();
	for(i = 0; i < 8; i++ ) {
		/* SCL拉低 */
		IIC_SCL = LOW; 
        IICDelay();
		/* 再拉高SCL, 产生一个有效的时钟信号 */
		IIC_SCL = HIGH;
		/* 读取总线上的数据 */
		receive = (receive << 1) | READ_SDA;
        IICDelay();
	}
    if (ack == NACK)
        i2c_nack();
    else
        i2c_ack();
	return receive;
}
#undef SDA_IN()
#undef SDA_OUT()
