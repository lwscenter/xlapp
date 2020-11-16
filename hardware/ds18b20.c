#include "sys.h"
#include "ds18b20.h"
DEFINE_DEV_REF(ds18b20_0_ref);
DEFINE_DEV_REF(ds18b20_1_ref);
static uint8_t ds18b20_0_ok = 1, ds18b20_1_ok = 1;
#define DS18B20_0_DQ_IN                 TEMPSENSOR0_IN_PIN
#define DS18B20_0_DQ_OUT                TEMPSENSOR0_OUT_PIN

#define DS18B20_1_DQ_IN                 TEMPSENSOR1_IN_PIN
#define DS18B20_1_DQ_OUT                TEMPSENSOR1_OUT_PIN
#if 0
#define DS18B20_0                       DEV_DS18B20_0
#define DS18B20_1                       DEV_DS18B20_1

#define DS18B20_0_DQ_OUT                TEMPSENSOR0_OUT_PIN
#define DS18B20_1_DQ_OUT                TEMPSENSOR1_OUT_PIN
#define DS18B20_DQ_OUT(_v)              (_v == DS18B20_0) ? DS18B20_0_DQ_OUT : DS18B20_1_DQ_OUT

#define DS18B20_0_DQ_IN                 TEMPSENSOR0_IN_PIN
#define DS18B20_1_DQ_IN                 TEMPSENSOR1_IN_PIN
#define DS18B20_DQ_IN_EX(_v)            (_v == DS18B20_0) ? DS18B20_0_DQ_IN : DS18B20_1_DQ_IN

#define DS18B20_0_IO_IN()               ds18b20_gpio_init(0, 0)
#define DS18B20_1_IO_IN()               ds18b20_gpio_init(1, 0)
#define DS18B20_IO_IN_EX(_v)            {if (_v == DS18B20_0) DS18B20_0_IO_IN(); else DS18B20_1_IO_IN(); }

#define DS18B20_0_IO_OUT()              ds18b20_gpio_init(0, 1)
#define DS18B20_1_IO_OUT()              ds18b20_gpio_init(1, 1)
#define DS18B20_IO_OUT_EX(_v)           {if (_v == DS18B20_0) DS18B20_0_IO_OUT(); else DS18B20_1_IO_OUT(); }

static u8 DS18B20_Check(uint8_t idx);

static uint8_t ds18b20_gpio_init(uint8_t idx, uint8_t dir)
{
/*GPIOC*/
    /*GPIO_InitTypeDef def;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    def.GPIO_Pin = _P(POWER_DQ_PIN_);
    def.GPIO_Speed = GPIO_Speed_50MHz;
    def.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &def);
*/
    GPIO_InitTypeDef  def;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    if (idx == DS18B20_0)
        def.GPIO_Pin = _P(TEMPSENSOR0_PIN_);
    else
        def.GPIO_Pin = _P(TEMPSENSOR1_PIN_);
    if (dir)
        def.GPIO_Mode = GPIO_Mode_Out_PP;
    else
        def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    /*def.GPIO_Mode = GPIO_Mode_Out_OD;*/
    def.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &def);
    return DS18B20_Check(idx);
}


static void DS18B20_Rst(uint8_t idx)
{
    if (idx == DS18B20_0) {
        DS18B20_0_IO_OUT(); //SET PA0 OUTPUT
        DS18B20_0_DQ_OUT=0; //����DQ
        delay_us(750);    //����750us
        DS18B20_0_DQ_OUT=1; //DQ=1
    } else {
        DS18B20_1_IO_OUT(); //SET PA0 OUTPUT
        DS18B20_1_DQ_OUT=0; //����DQ
        delay_us(750);    //����750us
        DS18B20_1_DQ_OUT=1; //DQ=1
    }
    delay_us(15);     //15US
}
static u8 DS18B20_Check(uint8_t idx)
{
    u8 retry = 0;
    return retry;
    if (idx == DS18B20_0)
        DS18B20_0_IO_IN();//SET PA0 INPUT
    else
        DS18B20_1_IO_IN();//SET PA0 INPUT
    if (idx == DS18B20_0) {
        while (DS18B20_0_DQ_IN&&retry<200) {
            retry++;
            delay_us(1);
        };
    } else {
        while (DS18B20_0_DQ_IN&&retry<200) {
            retry++;
            delay_us(1);
        };
    }
    if(retry>=200)return 1;
    else retry=0;
    if (idx == DS18B20_0) {
        while (!DS18B20_0_DQ_IN&&retry<240)
        {
            retry++;
            delay_us(1);
        };
    } else {
        while (!DS18B20_0_DQ_IN&&retry<240)
        {
            retry++;
            delay_us(1);
        };
    }
    if(retry>=240)return 1;
    return 0;
}
//��DS18B20��ȡһ��λ
//����ֵ��1/0
static u8 DS18B20_0_Read_Bit(void) // read one bit
{
    u8 data;
    DS18B20_0_IO_OUT();//SET PA0 OUTPUT
    DS18B20_0_DQ_OUT=0;
    delay_us(2);
    DS18B20_0_DQ_OUT=1;
    DS18B20_0_IO_IN();//SET PA0 INPUT
    delay_us(12);
    if(DS18B20_0_DQ_IN)data=1;
    else data=0;
    delay_us(50);
    return data;
}
static u8 DS18B20_1_Read_Bit(void) // read one bit
{
    u8 data;
    DS18B20_1_IO_OUT();//SET PA0 OUTPUT
    DS18B20_1_DQ_OUT=0;
    delay_us(2);
    DS18B20_1_DQ_OUT=1;
    DS18B20_1_IO_IN();//SET PA0 INPUT
    delay_us(12);
    if(DS18B20_1_DQ_IN)data=1;
    else data=0;
    delay_us(50);
    return data;
}
//��DS18B20��ȡһ���ֽ�
//����ֵ������������
static uint8_t DS18B20_Read_Byte(uint8_t idx)
{
    uint8_t tmp, i, dat;
    dat = 0;
    for (i = 8; i > 0; --i) {
        if (idx == DS18B20_0)
            tmp = DS18B20_0_Read_Bit();
        else
            tmp = DS18B20_1_Read_Bit();
        if (tmp)
            dat |= 1 << (i - 1);
    }
    return dat;
}
//dat��Ҫд����ֽ�
static void DS18B20_Write_Byte(uint8_t idx, u8 dat)
 {
    u8 j;
    u8 testb;
    if (idx == DS18B20_0)
        DS18B20_0_IO_OUT();//SET PA0 OUTPUT;
    else
        DS18B20_1_IO_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) {
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) {
            if (idx == DS18B20_0)
                DS18B20_0_DQ_OUT=0;// Write 1
            else
                DS18B20_1_DQ_OUT=0;// Write 1
            delay_us(2);
            if (idx == DS18B20_0)
                DS18B20_0_DQ_OUT=1;// Write 1
            else
                DS18B20_1_DQ_OUT=1;// Write 1
            delay_us(60);
        } else {
            if (idx == DS18B20_0)
                DS18B20_0_DQ_OUT=0;// Write 0
            else
                DS18B20_1_DQ_OUT=0;// Write 0
            delay_us(60);
            if (idx == DS18B20_0)
                DS18B20_0_DQ_OUT=1;
            else
                DS18B20_1_DQ_OUT=1;
            delay_us(2);
        }
    }
}
//��ʼ�¶�ת��
static void DS18B20_Start(uint8_t idx)// ds1820 start convert
{
    DS18B20_Rst(idx);
    DS18B20_Check(idx);
    DS18B20_Write_Byte(idx, 0xcc);// skip rom
    DS18B20_Write_Byte(idx, 0x44);// convert
}
//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����
uint8_t ds18b20_init(uint8_t idx)
{
    ds18b20_gpio_init(idx, 1);
	/*RCC->APB2ENR|=1<<2;    //ʹ��PORTA��ʱ��
	GPIOA->CRL&=0XFFFFFFF0;//PORTA0 �������
	GPIOA->CRL|=0X00000003;
	GPIOA->ODR|=1<<0;      //���1*/
	DS18B20_Rst(idx);
	return DS18B20_Check(idx);
}
//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250��
short DS18B20_Get_Temp(uint8_t idx)
{
    u8 temp;
    u8 TL,TH;
    short tem;
    DS18B20_Start(idx);                    // ds1820 start convert
    DS18B20_Rst(idx);
    DS18B20_Check(idx);
    DS18B20_Write_Byte(idx, 0xcc);// skip rom
    DS18B20_Write_Byte(idx, 0xbe);// convert
    TL=DS18B20_Read_Byte(idx); // LSB
    TH=DS18B20_Read_Byte(idx); // MSB

    if(TH>7) {
        TH=~TH;
        TL=~TL;
        temp=0;//�¶�Ϊ��
    } else temp=1;//�¶�Ϊ��
    tem=TH; //��ø߰�λ
    tem<<=8;
    tem+=TL;//��õװ�λ
    tem=(float)tem*0.625;//ת��
	if(temp)return tem; //�����¶�ֵ
	else return -tem;
}

#else

static void DS18B20_0_IO_IN(void)
{
    GPIO_InitTypeDef  def;
    def.GPIO_Pin = _P(TEMPSENSOR0_PIN_);
    def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    def.GPIO_Mode = GPIO_Mode_IPU;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
 	GPIO_Init(GPIOC, &def);

}
//ͨ���������
static void DS18B20_0_IO_OUT(void)
{
    GPIO_InitTypeDef  def;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

 	def.GPIO_Pin = _P(TEMPSENSOR0_PIN_);
    def.GPIO_Mode = GPIO_Mode_Out_PP;
 	def.GPIO_Speed = GPIO_Speed_10MHz;
 	GPIO_Init(GPIOC, &def);
}
//��λDS18B20
static void DS18B20_0_Rst(void)
{
	 DS18B20_0_IO_OUT(); 	//SET PG11 OUTPUT
     DS18B20_0_DQ_OUT=0; 	//����DQ
     delay_us(750);    	//����750us
     DS18B20_0_DQ_OUT=1; 	//DQ=1
	 delay_us(15);     	//15US
}
//�ȴ�DS18B20�Ļ�Ӧ�����Ӧ������
//����1:δ��⵽DS18B20�Ĵ���
//����0:����
static u8 DS18B20_0_Check(void)
{
	u8 retry=0;
	DS18B20_0_IO_IN();
    while (DS18B20_0_DQ_IN&&retry<200)
	{
		retry++;
		delay_us(1);
	};
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_0_DQ_IN&&retry<240)
	{
		retry++;
		delay_us(1);
	}
	if(retry>=240)return 1;
    return 0;
}

u8 DS18B20_0_self_check(void)
{
    u8 val = 0;
    val = ds18b20_0_ok;
    if (val) {
        mid_send_to_host("Heating1 self-check fail"CRLF);
    } else {
        mid_send_to_host("Heating1 self-check ok"CRLF);
    }
    return val;
}
//��DS18B20��ȡһ��λ
//����ֵ��1/0
static u8 DS18B20_0_Read_Bit(void)
{
    u8 data;
	  DS18B20_0_IO_OUT();	//SET PG11 OUTPUT
    DS18B20_0_DQ_OUT=0;
	  delay_us(2);
    DS18B20_0_DQ_OUT=1;
	  DS18B20_0_IO_IN();	//SET PG11 INPUT
	  delay_us(8);
	  if(DS18B20_0_DQ_IN)data=1;
    else data=0;
    delay_us(50);
    return data;
}

//��DS18B20��ȡһ���ֽ�
//����ֵ������������
static u8 DS18B20_0_Read_Byte(void)
{
  u8 i,j,dat;
  dat=0;
	for (i=1;i<=8;i++)
	{
        j=DS18B20_0_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }
    return dat;
}

//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
static void DS18B20_0_Write_Byte(u8 dat)
 {
    u8 j;
    u8 testb;
	DS18B20_0_IO_OUT();	//SET PG11 OUTPUT;
    for (j=1;j<=8;j++)
	 {
        testb=dat&0x01;
        dat=dat>>1;
        if (testb)
        {
            DS18B20_0_DQ_OUT=0;	// Write 1
            delay_us(5);
            DS18B20_0_DQ_OUT=1;
            delay_us(60);
        }
        else
        {
            DS18B20_0_DQ_OUT=0;	// Write 0
            delay_us(60);
            DS18B20_0_DQ_OUT=1;
            delay_us(5);
        }
    }
 }

//��ʼ�¶�ת��
static void DS18B20_0_Start(void)
{
    DS18B20_0_Rst();
	DS18B20_0_Check();
    DS18B20_0_Write_Byte(0xcc);	// ����ROM
    DS18B20_0_Write_Byte(0x44);	// ��ʼADת��
}

//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����
u8 ds18b20_0_init(void)
{

     /*GPIO_InitTypeDef  GPIO_InitStructure;

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

     GPIO_SetBits(GPIOA,GPIO_Pin_9);    */
    DS18B20_0_IO_OUT();
    DS18B20_0_DQ_OUT = 1;
    DS18B20_0_Rst();
    return DS18B20_0_Check();
}


//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250��
short ds18b20_0_get_temp(void)
{
    u8 temp;
    u8 TL,TH;
    short tem;
    DEV_GET_REF(ds18b20_0_ref);
    DS18B20_0_Start();  			// ds1820 start convert
    DS18B20_0_Rst();
    ds18b20_0_ok = DS18B20_0_Check();
    DS18B20_0_Write_Byte(0xcc);	// ����ROM
    DS18B20_0_Write_Byte(0xbe);	// ���¶ȼĴ���
    TL=DS18B20_0_Read_Byte(); 	// ��LSB
    TH=DS18B20_0_Read_Byte(); 	// ��MSB
    DEV_PUT_REF(ds18b20_0_ref);

    if(TH>7) {
        TH=~TH;
        TL=~TL;
        temp=0;					//�¶�Ϊ��
    }
    else temp=1;				//�¶�Ϊ��
    tem=TH; 					//��ø߰�λ
    tem<<=8;
    tem+=TL;					//��õװ�λ
    tem=(float)tem*0.625;		//ת��
	if(temp)return tem; 		//�����¶�ֵ
	else return -tem;
}
static void DS18B20_1_IO_IN(void)
{
    GPIO_InitTypeDef  def;
    def.GPIO_Pin = _P(TEMPSENSOR1_PIN_);
    def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    def.GPIO_Mode = GPIO_Mode_IPU;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
 	GPIO_Init(GPIOC, &def);

}
//ͨ���������
static void DS18B20_1_IO_OUT(void)
{
    GPIO_InitTypeDef  def;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

 	def.GPIO_Pin = _P(TEMPSENSOR1_PIN_);
    def.GPIO_Mode = GPIO_Mode_Out_PP;
 	def.GPIO_Speed = GPIO_Speed_10MHz;
 	GPIO_Init(GPIOC, &def);
}
//��λDS18B20
static void DS18B20_1_Rst(void)
{
	 DS18B20_1_IO_OUT(); 	//SET PG11 OUTPUT
     DS18B20_1_DQ_OUT=0; 	//����DQ
     delay_us(750);    	//����750us
     DS18B20_1_DQ_OUT=1; 	//DQ=1
	 delay_us(15);     	//15US
}
//�ȴ�DS18B20�Ļ�Ӧ�����Ӧ������
//����1:δ��⵽DS18B20�Ĵ���
//����0:����
static u8 DS18B20_1_Check(void)
{
	u8 retry=0;
	DS18B20_1_IO_IN();
    while (DS18B20_1_DQ_IN&&retry<200)
	{
		retry++;
		delay_us(1);
	};
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_1_DQ_IN&&retry<240)
	{
		retry++;
		delay_us(1);
	}
	if(retry>=240)return 1;
    return 0;
}

//��DS18B20��ȡһ��λ
//����ֵ��1/0
static u8 DS18B20_1_Read_Bit(void)
{
    u8 data;
	  DS18B20_1_IO_OUT();	//SET PG11 OUTPUT
    DS18B20_1_DQ_OUT=0;
	  delay_us(2);
    DS18B20_1_DQ_OUT=1;
	  DS18B20_1_IO_IN();	//SET PG11 INPUT
	  delay_us(8);
	  if(DS18B20_1_DQ_IN)data=1;
    else data=0;
    delay_us(50);
    return data;
}

//��DS18B20��ȡһ���ֽ�
//����ֵ������������
static u8 DS18B20_1_Read_Byte(void)
{
  u8 i,j,dat;
  dat=0;
	for (i=1;i<=8;i++)
	{
        j=DS18B20_1_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }
    return dat;
}

//дһ���ֽڵ�DS18B20
//dat��Ҫд����ֽ�
static void DS18B20_1_Write_Byte(u8 dat)
 {
    u8 j;
    u8 testb;
	DS18B20_1_IO_OUT();	//SET PG11 OUTPUT;
    for (j=1;j<=8;j++)
	 {
        testb=dat&0x01;
        dat=dat>>1;
        if (testb)
        {
            DS18B20_1_DQ_OUT=0;	// Write 1
            delay_us(5);
            DS18B20_1_DQ_OUT=1;
            delay_us(60);
        }
        else
        {
            DS18B20_1_DQ_OUT=0;	// Write 0
            delay_us(60);
            DS18B20_1_DQ_OUT=1;
            delay_us(5);
        }
    }
 }

//��ʼ�¶�ת��
static void DS18B20_1_Start(void)
{
    DS18B20_1_Rst();
	DS18B20_1_Check();
    DS18B20_1_Write_Byte(0xcc);	// ����ROM
    DS18B20_1_Write_Byte(0x44);	// ��ʼADת��
}

//��ʼ��DS18B20��IO�� DQ ͬʱ���DS�Ĵ���
//����1:������
//����0:����
u8 ds18b20_1_init(void)
{

     /*GPIO_InitTypeDef  GPIO_InitStructure;

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

     GPIO_SetBits(GPIOA,GPIO_Pin_9);    */
    DS18B20_1_IO_OUT();
    DS18B20_1_DQ_OUT = 1;
    DS18B20_1_Rst();
	return DS18B20_1_Check();
}

u8 DS18B20_1_self_check(void)
{
    u8 val = 0;
    val = ds18b20_1_ok;
    if (val) {
        mid_send_to_host("Heating2 self-check fail"CRLF);
    } else {
        mid_send_to_host("Heating2 self-check ok"CRLF);
    }
    return val;
}

//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250��
short ds18b20_1_get_temp(void)
{
    u8 temp;
    u8 TL,TH;
	short tem;
    DEV_GET_REF(ds18b20_1_ref);
    DS18B20_1_Start();  			// ds1820 start convert
    DS18B20_1_Rst();
    ds18b20_1_ok = DS18B20_1_Check();
    DS18B20_1_Write_Byte(0xcc);	// ����ROM
    DS18B20_1_Write_Byte(0xbe);	// ���¶ȼĴ���
    TL=DS18B20_1_Read_Byte(); 	// ��LSB
    TH=DS18B20_1_Read_Byte(); 	// ��MSB
    DEV_PUT_REF(ds18b20_1_ref);

    if(TH>7)
    {
        TH=~TH;
        TL=~TL;
        temp=0;					//�¶�Ϊ��
    }
    else temp=1;				//�¶�Ϊ��
    tem=TH; 					//��ø߰�λ
    tem<<=8;
    tem+=TL;					//��õװ�λ
    tem=(float)tem*0.625;		//ת��
	if(temp)return tem; 		//�����¶�ֵ
	else return -tem;
}
#endif
