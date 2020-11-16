#include "sys.h"
#include "main.h"
#include "bsp.h"
#include "usart.h"
#include "rc522.h"
#include "protocol.h"
DEFINE_DEV_REF(rc522_ref);

#if USE_SOFT_SPI2

#define  RC522_SCK_L()               (SPI2_CLK_PIN = LOW)
#define  RC522_SCK_H()               (SPI2_CLK_PIN = HIGH)

#define  RC522_MOSI_L()              (SPI2_MOSI_PIN = LOW)
#define  RC522_MOSI_H()              (SPI2_MOSI_PIN = HIGH)

#define  RC522_MISO_int()            SPI_MISO_PIN
#else

#endif
#define RC522_Reset_Enable()
#define RC522_Reset_Disable()
#define RC522_CS_Enable()                   spi_cs_pin(LOW)
#define RC522_CS_Disable()                  spi_cs_pin(HIGH)

#define DELAY_US(_v)                        delay_us((_v))


static const uint8_t CODE[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};



#if USE_SOFT_SPI2

/********************************************************************************
 *函数功能:SPI发送数据
 *参数:待发送的数据
 *返回值:无
 *
 **********************************************************************************/
static void RC522_Send(uint8_t byte)
{
    uint8_t counter;

    for(counter=0;counter<8;counter++) {
        if ( byte & 0x80 )
            RC522_MOSI_H();
        else
            RC522_MOSI_L();
        DELAY_US(2);
        RC522_SCK_L();
        DELAY_US(2);
        RC522_SCK_H();
        DELAY_US(2);
        byte <<= 1;
    }
}
/*******************************************************************************************
 *函数功能:SPI读函数
 *参数:无
 *返回值:读到的数据
 *
 ********************************************************************************************/
static uint8_t RC522_Read( void )
{
    uint8_t counter;
    uint8_t Data = 0;
    for(counter=0;counter<8;counter++)
    {
        Data <<= 1;
        RC522_SCK_L();
        DELAY_US(2);
        if (RC522_MISO_int() == 1)
            Data |= 0x01;
        DELAY_US(2);
        RC522_SCK_H();
        DELAY_US(2);
    }
    return Data;

}
#else
#define RC522_Send(_V)              spi_write((_V))
#define RC522_Read()                spi_read()


#endif
/******************************************************************************************
 *函数功能:读RC522寄存器
 *参数:address 寄存器的地址
 *返回值:寄存器的当前值
 *
 ******************************************************************************************/
static uint8_t ReadRC(uint8_t address )
{
    uint8_t Addr, Return;
    Return = 1;

    Addr=((address<<1)&0x7E)|0x80;

    RC522_CS_Enable();

    RC522_Send(Addr);

    Return=RC522_Read();

    RC522_CS_Disable();

    return Return;


}


/*******************************************************************************************
 *函数功能:写RC522寄存器
 *参数:Addresss 寄存器的地址
 Value 写入寄存器的值
 *返回值:无
 *******************************************************************************************/
static void WriteRawRC(uint8_t Address,uint8_t Value)
{
    uint8_t Addr;

    Addr =(Address<<1)&0x7E;

    RC522_CS_Enable();

    RC522_Send(Addr);

    RC522_Send(Value);

    RC522_CS_Disable();


}
/*******************************************************************************************
 *函数功能:对RC522寄存器置位
 *参数:Reg 寄存器的纸质
 mask 置位的值
 * 返回值:无
 *******************************************************************************************/
static void SetBitMask(uint8_t Reg,uint8_t Mask )
{
    uint8_t Temp;

    Temp=ReadRC(Reg);

    WriteRawRC(Reg,Temp|Mask);         // set bit mask

}

/*******************************************************************************************
 *函数功能:对RC522寄存器清零
 *参数:Reg RC522的地址
 Mask 清零的数据
 *返回值:无
 *******************************************************************************************/
static uint8_t rfid_err_count;
#define RFID_ERR_FLAG_VAL           200
static void ClearBitMask(uint8_t Reg,uint8_t Mask )
{
    uint8_t tmp;

    tmp=ReadRC(Reg);
    if (Reg == Status2Reg) {
        if (tmp == 0xff) {
            if (rfid_err_count < RFID_ERR_FLAG_VAL)
                ++rfid_err_count;
        }
    }


    WriteRawRC(Reg,tmp&(~Mask));  // clear bit mask

}
/*******************************************************************************************
 *函数功能:开天线
 *参数:无
 *返回值:无
 *
 ********************************************************************************************/
static void PcdAntennaOn(void)
{
    uint8_t temp;

    temp=ReadRC(TxControlReg);

    if (!(temp&0x03))
        SetBitMask(TxControlReg,0x03);
}
/*******************************************************************************************
 *函数功能:关闭天线
 *参数:无
 *返回值:
 *
 *******************************************************************************************/
static void PcdAntennaOff ( void )
{
    ClearBitMask(TxControlReg,0x03 );

}


/*******************************************************************************************
 *函数功能:复位RC522
 *参数:无
 *返回值:无
 *
 *******************************************************************************************/
static void PcdReset(void)
{
    RC522_Reset_Disable();

    DELAY_US(1);

    RC522_Reset_Enable();

    DELAY_US(1);

    RC522_Reset_Disable();

    DELAY_US(1);

    WriteRawRC(CommandReg,0x0f);

    //while(ReadRC(CommandReg)&0x10);

    DELAY_US(1);

    WriteRawRC(ModeReg, 0x3D );            //定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363

    WriteRawRC(TReloadRegL,30);          //16位定时器低位
    WriteRawRC(TReloadRegH,0);			     //16位定时器高位

    WriteRawRC(TModeReg,0x8D);				   //定义内部定时器的设置

    WriteRawRC(TPrescalerReg,0x3E);			 //设置定时器分频系数

    WriteRawRC(TxAutoReg,0x40);				   //调制发送信号为100%ASK


}

/*******************************************************************************************
 *函数功能:设置RC522的工作方式
 *参数:CardType:工作方式
 *返回值:无
 *
 *******************************************************************************************/
static void M500PcdConfigISOType(uint8_t CardType)
{
    if (CardType== 'A')                     //ISO14443_A
    {
        ClearBitMask(Status2Reg,0x08);

        WriteRawRC(ModeReg,0x3D);//3F

        WriteRawRC(RxSelReg,0x86);//84

        WriteRawRC(RFCfgReg,0x7F);   //4F

        WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)

        WriteRawRC(TReloadRegH,0);

        WriteRawRC(TModeReg,0x8D);

        WriteRawRC(TPrescalerReg,0x3E);

        DELAY_US(2);
        PcdAntennaOn();//开天线

    }


}


/*******************************************************************************************
 *函数功能:通过RC522和ISO14443卡通讯
 *参数:    Command，RC522的命令
 *         intData，RC522发送到卡片的数据
 *         intLen，发送数据的字节长度
 *         outData，接收到的卡片返回数据
 *         outLen，返回数据的长度
 *返回值:
 ********************************************************************************************/
static uint8_t PcdComMF522(uint8_t Command,uint8_t *intData,uint8_t intLen,uint8_t * Data,uint32_t * outLen )
{
    uint8_t Status =MI_FAIL;
    uint8_t IrqEn  = 0x00;
    uint8_t WaitFor = 0x00;
    uint8_t LastBits;
    uint8_t N;
    uint32_t L;

    switch (Command)
    {
    case PCD_AUTHENT:		//Mifare认证
        IrqEn=0x12;		//允许错误中断请求ErrIEn  允许空闲中断IdleIEn
        WaitFor=0x10;		//认证寻卡等待时候 查询空闲中断标志位
        break;

    case PCD_TRANSCEIVE:		//接收发送 发送接收
        IrqEn   = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        WaitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;

    default:
        break;

    }

    WriteRawRC( ComIEnReg,IrqEn|0x80);		//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
    ClearBitMask( ComIrqReg,0x80 );			//Set1该位清零时，CommIRqReg的屏蔽位清零
    WriteRawRC( CommandReg,PCD_IDLE );		//写空闲命令
    SetBitMask( FIFOLevelReg,0x80);			//置位FlushBuffer清除内部FIFO的读和写指针以及ErrReg的BufferOvfl标志位被清除

    for (L= 0; L<intLen;L++ )
        WriteRawRC(FIFODataReg,Data [L]);    		//写数据进FIFOdata

    WriteRawRC(CommandReg,Command);					//写命令


    if (Command==PCD_TRANSCEIVE )
        SetBitMask(BitFramingReg,0x80);  				//StartSend置位启动数据发送 该位与收发命令使用时才有效

    L= 1000;//根据时钟频率调整，操作M1卡最大等待时间25ms

    do 														//认证 与寻卡等待时间
    {
        N = ReadRC ( ComIrqReg );							//查询事件中断
        L--;
    } while ((L!=0)&&(!(N&0x01))&&(!(N&WaitFor ) ) );		//退出条件i=0,定时器中断，与写空闲命令

    ClearBitMask ( BitFramingReg, 0x80 );					//清理允许StartSend位

    if (L!= 0 )
    {
        if ( ! ( ReadRC ( ErrorReg ) & 0x1B ) )			//读错误标志寄存器BufferOfI CollErr ParityErr ProtocolErr
        {
            Status =MI_OK ;

            if ( N & IrqEn & 0x01 )					//是否发生定时器中断
                Status =Irq_falg ;

            if (Command ==PCD_TRANSCEIVE )
            {
                N = ReadRC ( FIFOLevelReg );			//读FIFO中保存的字节数

                LastBits = ReadRC(ControlReg ) & 0x07;	//最后接收到得字节的有效位数

                if(LastBits )
                    * outLen=(N - 1 )*8+LastBits;   	//N个字节数减去1（最后一个字节）+最后一位的位数 读取到的数据总位数
                else
                    *outLen=N * 8;   					//最后接收到的字节整个字节有效

                if (N ==0)
                    N = 1;

                if (N >MAXRLEN)
                    N =MAXRLEN;

                for (L=0;L<N;L ++ )
                    Data[L] =ReadRC(FIFODataReg );

            }

        }

        else
            Status =MI_FAIL;

    }

    SetBitMask ( ControlReg, 0x80 );           // stop timer now
    WriteRawRC ( CommandReg, PCD_IDLE );


    return Status;


}


/*******************************************************************************************
 *函数功能:寻卡
 *参数：Card_code，寻卡方式
 *                    = 0x52，寻感应区内所有符合14443A标准的卡
 *                     = 0x26，寻未进入休眠状态的卡
 *        CardType，卡片类型代码
 *                   = 0x4400，Mifare_UltraLight
 *                  = 0x0400，Mifare_One(S50)
 *                   = 0x0200，Mifare_One(S70)
 *                  = 0x0800，Mifare_Pro(X))
 *                   = 0x4403，Mifare_DESFire
 * 返回值:1 ,成功 0，失败
 *******************************************************************************************/
static uint8_t PcdRequest ( uint8_t Req_code, uint8_t * CardType )
{
    uint8_t Status;
    uint8_t buffer[MAXRLEN];
    uint32_t Len;


    ClearBitMask ( Status2Reg, 0x08 );	//清理指示MIFARECyptol单元接通以及所有卡的数据通信被加密的情况
    WriteRawRC ( BitFramingReg, 0x07 );	//	发送的最后一个字节的 七位
    SetBitMask ( TxControlReg, 0x03 );	//TX1,TX2管脚的输出信号传递经发送调制的13.56的能量载波信号

    buffer[0] =Req_code;		//存入 卡片命令字

    Status = PcdComMF522(PCD_TRANSCEIVE,buffer,1,buffer,&Len );	//寻卡

    if ((Status ==MI_OK)&&(Len == 0x10 ) ){ //寻卡成功返回卡类型
        * CardType=buffer[0];
        * (CardType+1)=buffer[1];
    }

    else
        Status =MI_FAIL;

    return Status;

}


/*******************************************************************************************
 *函数功能:防冲撞
 *参数:接收ID号缓冲区
 *返回值:1,成功，0，失败
 *
 ********************************************************************************************/
static uint8_t PcdAnticoll ( uint8_t * Snr )
{
    uint8_t Status;
    uint8_t uc, Snr_check = 0;
    uint8_t buffer[MAXRLEN];
    uint32_t Len;

    ClearBitMask ( Status2Reg, 0x08 );		//清MFCryptol On位 只有成功执行MFAuthent命令后，该位才能置位
    WriteRawRC ( BitFramingReg, 0x00);		//清理寄存器 停止收发
    ClearBitMask ( CollReg, 0x80 );			//清ValuesAfterColl所有接收的位在冲突后被清除

    buffer[ 0 ] = 0x93;	//卡片防冲突命令
    buffer[ 1 ] = 0x20;

    Status =PcdComMF522(PCD_TRANSCEIVE,buffer, 2,buffer,&Len);//与卡片通信

    if (Status==MI_OK )		//通信成功
    {
        for ( uc = 0; uc < 4; uc ++ )
        {
            * ( Snr + uc )  = buffer[ uc ];			//读出UID
            Snr_check ^= buffer[ uc ];
        }

        if ( Snr_check !=buffer[ uc ] )
            Status =MI_FAIL;

    }

    SetBitMask ( CollReg, 0x80 );


    return Status;


}


/*******************************************************************************************
  函数功能:用RC522计算CRC16
  参数:intdata，计算CRC16的数组
  Len，计算CRC16的数组字节长度
  0utData，存放计算结果存放的首地址
 *返回值:无
 *******************************************************************************************/
static void CalulateCRC ( uint8_t * intData, uint8_t Len, uint8_t * outData )
{
    uint8_t k,j;


    ClearBitMask(DivIrqReg,0x04);

    WriteRawRC(CommandReg,PCD_IDLE);

    SetBitMask(FIFOLevelReg,0x80);

    for (k= 0;k<Len;k++)
        WriteRawRC ( FIFODataReg, * (intData +k) );

    WriteRawRC ( CommandReg, PCD_CALCCRC );

    k= 0xFF;

    do
    {
        j= ReadRC(DivIrqReg );
        k--;
    } while ( (k!= 0) && ! (j& 0x04 ) );

    outData[0] = ReadRC(CRCResultRegL);
    outData[1] = ReadRC(CRCResultRegM);


}


/*******************************************************************************************
  函数功能:选定卡片
  参数:pSnr，卡片序列号，4字节
 *返回值: 1,成功,0，失败
 *
 ********************************************************************************************/
static uint8_t PcdSelect ( uint8_t * pSnr )
{
    uint8_t ucN;
    uint8_t uc;
    uint8_t buffer[MAXRLEN];
    uint32_t  ulLen;


    buffer[0] = PICC_ANTICOLL1;
    buffer[1] = 0x70;
    buffer[6] = 0;

    for ( uc = 0; uc < 4; uc ++ )
    {
        buffer[ uc + 2 ] = * ( pSnr + uc );
        buffer[ 6 ] ^= * ( pSnr + uc );
    }

    CalulateCRC (buffer, 7, &buffer[ 7 ] );

    ClearBitMask ( Status2Reg, 0x08 );

    ucN = PcdComMF522 ( PCD_TRANSCEIVE,buffer, 9,buffer, & ulLen );

    if ( ( ucN ==MI_OK ) && ( ulLen == 0x18 ) )
        ucN =MI_OK ;
    else
        ucN =MI_FAIL;


    return ucN;

}
/*******************************************************************************************
 *函数功能:验证卡片密码
 *参数:mode，密码验证模式
 *        0x60，验证A密钥
 *        0x61，验证B密钥
 *        uint8_t Addr，块地址
 *        Key，密码
 *        Str，卡片序列号，4字节
 *返回值: 1,成功,0，失败
 *******************************************************************************************/
static uint8_t PcdAuthState ( uint8_t mode, uint8_t Addr, const uint8_t * Key, uint8_t * str )
{
    uint8_t Status;
    uint8_t uc, buffer[MAXRLEN];
    uint32_t Len;


    buffer[ 0 ] =mode;
    buffer[ 1 ] = Addr;

    for ( uc = 0; uc < 6; uc ++ )
        buffer[uc + 2 ] = * ( Key + uc );

    for ( uc = 0; uc < 6; uc ++ )
        buffer[ uc + 8 ] = * (str + uc );

    Status = PcdComMF522 ( PCD_AUTHENT,buffer, 12,buffer, &Len );

    if ( ( Status !=MI_OK ) || ( ! ( ReadRC ( Status2Reg ) & 0x08 ) ) )
        Status =MI_FAIL;


    return Status;

}


/*******************************************************************************************
 *函数功能:写数据到M1卡一块
 *参数:uint8_t addr，块地址
 *     Data，写入的数据，16字节
 *返回值:1,成功，0，失败
 *******************************************************************************************/
__attribute__((unused))
static uint8_t PcdWrite ( uint8_t addr, uint8_t * Data )
{
    uint8_t Status;
    uint8_t uc,buffer[MAXRLEN];
    uint32_t Len;


    buffer[ 0 ] = PICC_WRITE;
    buffer [ 1 ] =addr;

    CalulateCRC (buffer, 2, &buffer[ 2 ] );

    Status = PcdComMF522 ( PCD_TRANSCEIVE,buffer, 4,buffer, & Len );

    if ((Status != MI_OK  )||(Len!=4)||((buffer[ 0 ] & 0x0F ) != 0x0A ) )
        Status =MI_FAIL;

    if (Status==MI_OK  )
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for ( uc = 0; uc < 16; uc ++ )
            buffer[uc] = * ( Data + uc );

        CalulateCRC ( buffer, 16, &buffer[ 16 ] );

        Status = PcdComMF522 ( PCD_TRANSCEIVE,buffer, 18,buffer,&Len );

        if ( ( Status != MI_OK )||(Len !=4)||((buffer[0]&0x0F)!=0x0A))
            Status =MI_FAIL;

    }


    return Status;


}


/*******************************************************************************************
 *函数功能:读取M1卡一块数据
 *参数:uint8_t Aaddr，块地址
 *         Data，读出的数据，16字节
 *返回值:1,成功,0,失败
 *
 ********************************************************************************************/
static uint8_t PcdRead (uint8_t addr,uint8_t *Data )
{
    uint8_t Status;
    uint8_t L, buffer[MAXRLEN];
    uint32_t Len;

    buffer[0]=PICC_READ;
    buffer[1]=addr;

    CalulateCRC (buffer, 2, &buffer[ 2 ] );

    Status = PcdComMF522 ( PCD_TRANSCEIVE,buffer, 4,buffer,&Len );

    if ((Status==MI_OK  ) && (Len == 0x90 )) {
        for (L= 0; L< 16; L++ )
            * (Data+L) = buffer[L];
    }

    else
        Status =MI_FAIL;


    return Status;

}
/*******************************************************************************************
 *函数功能:命令卡片进入休眠状态
 *参数:无
 *返回值:1，成功，0，失败
 *
 ********************************************************************************************/
__attribute__((unused))
static uint8_t PcdHalt(void)
{
    uint8_t buffer[MAXRLEN];
    uint32_t  Len;


    buffer[0]=PICC_HALT;
    buffer[1]=0;

    CalulateCRC (buffer, 2, &buffer[ 2 ] );
    PcdComMF522(PCD_TRANSCEIVE,buffer, 4,buffer, &Len);

    return MI_OK ;
}
/****************************************************************************************************************
 *函数功能:RC522测试
 *参数:无
 *返回值:无
 ******************************************************************************************************************/
/*
   uint8_t RC522_test(void)
   {
   char Str[30];
   uint8_t Card_ID[4];

   while (1)
   {

   PcdRequest (PICC_REQALL, Card_ID); //寻卡		                                                 //若失败再次寻卡

   if (PcdAnticoll(Card_ID)==MI_OK )                                                                   //防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）
   {
//sprintf(Str,"\r\n门禁卡的ID号: %02X%02X%02X%02X",Card_ID[0],Card_ID[1],Card_ID[2],Card_ID[3]);
return Card_ID[0];
//printf ("%s\r\n",Str);

}


}

void loop()
{
  //Search card, return card types
  if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
    Serial.println("Find the card!");
    // Show card type
    ShowCardType(str);
    //防冲突检测,读取卡序列号
    if (rfid.anticoll(str) == MI_OK) {
      Serial.print("The card's number is  : ");
      //显示卡序列号
      for(int i = 0; i < 4; i++){
        Serial.print(0x0F & (str[i] >> 4),HEX);
        Serial.print(0x0F & str[i],HEX);
      }
      Serial.println("");
    }
    //选卡（锁定卡片，防止多数读取，去掉本行将连续读卡）
    rfid.selectTag(str);
  }
  rfid.halt();  //命令卡片进入休眠状态
}

}
*/

static char RfidInfo[20];
void send_rfid_info(void)
{
    mid_send_to_host(RfidInfo);
}

uint8_t read_rfid(void)
{
    uint8_t ret;
    static uint8_t id[4];
    static uint8_t Data[MAXRLEN + 5];
    ret = 1;
    /*

    24524e2b    01
    d93ff4a2    1c
    895a35a4    2c

    */
    if(PcdRequest (PICC_REQIDL, id) == MI_OK) {
        dbg_printf("Card_search_success!\n");
        rfid_err_count = 0;
        if (PcdAnticoll(id) == MI_OK) {
            PcdSelect(id);
            PcdAuthState(0x60, 0x02, CODE, id);
            if(PcdRead(0x02,Data) == MI_OK) {
#if 0
                switch(*(uint32_t *)id) {
                case 0x2b4e5224:
                    Data[15] = 0x1;
                    break;
                case 0xa2f43fd9:
                    Data[15] = 0x1c;
                    break;
                case 0xa4355a89:
                    Data[15] = 0x2c;
                    break;
                default:
                    break;
                }
#endif
                sprintf(RfidInfo,"RfidInfo_%02X%02X%02X!\n",Data[13],Data[14],Data[15]);
                send_rfid_info();
                ret = 0;
            }
        }
        PcdHalt();
    }
    return ret;
}

/****************************************************************************************************
 *函数功能:RC522初始化
 *参数:无
 *返回值:无
 *****************************************************************************************************/
uint8_t rc522_init (void)
{
    spi2_init();

    RC522_Reset_Disable();

    RC522_CS_Disable();

    PcdReset();
    M500PcdConfigISOType( 'A' );//设置工作方式
    if (ReadRC(VersionReg) == 0 ||
            ReadRC(VersionReg) == 255) {
        return 0;
    }
    return 1;
}

void rc522_off(void)
{
    RC522_CS_Disable();
    PcdAntennaOff();
    DELAY_US(200);
    spi2_uninit();
}


uint8_t rc522_self_check(void)
{
    uint8_t ret = 1;
    if (rfid_err_count == RFID_ERR_FLAG_VAL) {
        ret = 1;
    } else {
        ret = 0;
        rfid_err_count = 0;
    }
    if (!ret) {
        mid_send_to_host("Rfid self-check ok"CRLF);
    } else {
        mid_send_to_host("Rfid self-check fail"CRLF);
    }
    return ret;
}
