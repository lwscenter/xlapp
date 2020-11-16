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
 *��������:SPI��������
 *����:�����͵�����
 *����ֵ:��
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
 *��������:SPI������
 *����:��
 *����ֵ:����������
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
 *��������:��RC522�Ĵ���
 *����:address �Ĵ����ĵ�ַ
 *����ֵ:�Ĵ����ĵ�ǰֵ
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
 *��������:дRC522�Ĵ���
 *����:Addresss �Ĵ����ĵ�ַ
 Value д��Ĵ�����ֵ
 *����ֵ:��
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
 *��������:��RC522�Ĵ�����λ
 *����:Reg �Ĵ�����ֽ��
 mask ��λ��ֵ
 * ����ֵ:��
 *******************************************************************************************/
static void SetBitMask(uint8_t Reg,uint8_t Mask )
{
    uint8_t Temp;

    Temp=ReadRC(Reg);

    WriteRawRC(Reg,Temp|Mask);         // set bit mask

}

/*******************************************************************************************
 *��������:��RC522�Ĵ�������
 *����:Reg RC522�ĵ�ַ
 Mask ���������
 *����ֵ:��
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
 *��������:������
 *����:��
 *����ֵ:��
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
 *��������:�ر�����
 *����:��
 *����ֵ:
 *
 *******************************************************************************************/
static void PcdAntennaOff ( void )
{
    ClearBitMask(TxControlReg,0x03 );

}


/*******************************************************************************************
 *��������:��λRC522
 *����:��
 *����ֵ:��
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

    WriteRawRC(ModeReg, 0x3D );            //���巢�ͺͽ��ճ���ģʽ ��Mifare��ͨѶ��CRC��ʼֵ0x6363

    WriteRawRC(TReloadRegL,30);          //16λ��ʱ����λ
    WriteRawRC(TReloadRegH,0);			     //16λ��ʱ����λ

    WriteRawRC(TModeReg,0x8D);				   //�����ڲ���ʱ��������

    WriteRawRC(TPrescalerReg,0x3E);			 //���ö�ʱ����Ƶϵ��

    WriteRawRC(TxAutoReg,0x40);				   //���Ʒ����ź�Ϊ100%ASK


}

/*******************************************************************************************
 *��������:����RC522�Ĺ�����ʽ
 *����:CardType:������ʽ
 *����ֵ:��
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
        PcdAntennaOn();//������

    }


}


/*******************************************************************************************
 *��������:ͨ��RC522��ISO14443��ͨѶ
 *����:    Command��RC522������
 *         intData��RC522���͵���Ƭ������
 *         intLen���������ݵ��ֽڳ���
 *         outData�����յ��Ŀ�Ƭ��������
 *         outLen���������ݵĳ���
 *����ֵ:
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
    case PCD_AUTHENT:		//Mifare��֤
        IrqEn=0x12;		//��������ж�����ErrIEn  ��������ж�IdleIEn
        WaitFor=0x10;		//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
        break;

    case PCD_TRANSCEIVE:		//���շ��� ���ͽ���
        IrqEn   = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        WaitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ�� �����жϱ�־λ
        break;

    default:
        break;

    }

    WriteRawRC( ComIEnReg,IrqEn|0x80);		//IRqInv��λ�ܽ�IRQ��Status1Reg��IRqλ��ֵ�෴
    ClearBitMask( ComIrqReg,0x80 );			//Set1��λ����ʱ��CommIRqReg������λ����
    WriteRawRC( CommandReg,PCD_IDLE );		//д��������
    SetBitMask( FIFOLevelReg,0x80);			//��λFlushBuffer����ڲ�FIFO�Ķ���дָ���Լ�ErrReg��BufferOvfl��־λ�����

    for (L= 0; L<intLen;L++ )
        WriteRawRC(FIFODataReg,Data [L]);    		//д���ݽ�FIFOdata

    WriteRawRC(CommandReg,Command);					//д����


    if (Command==PCD_TRANSCEIVE )
        SetBitMask(BitFramingReg,0x80);  				//StartSend��λ�������ݷ��� ��λ���շ�����ʹ��ʱ����Ч

    L= 1000;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms

    do 														//��֤ ��Ѱ���ȴ�ʱ��
    {
        N = ReadRC ( ComIrqReg );							//��ѯ�¼��ж�
        L--;
    } while ((L!=0)&&(!(N&0x01))&&(!(N&WaitFor ) ) );		//�˳�����i=0,��ʱ���жϣ���д��������

    ClearBitMask ( BitFramingReg, 0x80 );					//��������StartSendλ

    if (L!= 0 )
    {
        if ( ! ( ReadRC ( ErrorReg ) & 0x1B ) )			//�������־�Ĵ���BufferOfI CollErr ParityErr ProtocolErr
        {
            Status =MI_OK ;

            if ( N & IrqEn & 0x01 )					//�Ƿ�����ʱ���ж�
                Status =Irq_falg ;

            if (Command ==PCD_TRANSCEIVE )
            {
                N = ReadRC ( FIFOLevelReg );			//��FIFO�б�����ֽ���

                LastBits = ReadRC(ControlReg ) & 0x07;	//�����յ����ֽڵ���Чλ��

                if(LastBits )
                    * outLen=(N - 1 )*8+LastBits;   	//N���ֽ�����ȥ1�����һ���ֽڣ�+���һλ��λ�� ��ȡ����������λ��
                else
                    *outLen=N * 8;   					//�����յ����ֽ������ֽ���Ч

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
 *��������:Ѱ��
 *������Card_code��Ѱ����ʽ
 *                    = 0x52��Ѱ��Ӧ�������з���14443A��׼�Ŀ�
 *                     = 0x26��Ѱδ��������״̬�Ŀ�
 *        CardType����Ƭ���ʹ���
 *                   = 0x4400��Mifare_UltraLight
 *                  = 0x0400��Mifare_One(S50)
 *                   = 0x0200��Mifare_One(S70)
 *                  = 0x0800��Mifare_Pro(X))
 *                   = 0x4403��Mifare_DESFire
 * ����ֵ:1 ,�ɹ� 0��ʧ��
 *******************************************************************************************/
static uint8_t PcdRequest ( uint8_t Req_code, uint8_t * CardType )
{
    uint8_t Status;
    uint8_t buffer[MAXRLEN];
    uint32_t Len;


    ClearBitMask ( Status2Reg, 0x08 );	//����ָʾMIFARECyptol��Ԫ��ͨ�Լ����п�������ͨ�ű����ܵ����
    WriteRawRC ( BitFramingReg, 0x07 );	//	���͵����һ���ֽڵ� ��λ
    SetBitMask ( TxControlReg, 0x03 );	//TX1,TX2�ܽŵ�����źŴ��ݾ����͵��Ƶ�13.56�������ز��ź�

    buffer[0] =Req_code;		//���� ��Ƭ������

    Status = PcdComMF522(PCD_TRANSCEIVE,buffer,1,buffer,&Len );	//Ѱ��

    if ((Status ==MI_OK)&&(Len == 0x10 ) ){ //Ѱ���ɹ����ؿ�����
        * CardType=buffer[0];
        * (CardType+1)=buffer[1];
    }

    else
        Status =MI_FAIL;

    return Status;

}


/*******************************************************************************************
 *��������:����ײ
 *����:����ID�Ż�����
 *����ֵ:1,�ɹ���0��ʧ��
 *
 ********************************************************************************************/
static uint8_t PcdAnticoll ( uint8_t * Snr )
{
    uint8_t Status;
    uint8_t uc, Snr_check = 0;
    uint8_t buffer[MAXRLEN];
    uint32_t Len;

    ClearBitMask ( Status2Reg, 0x08 );		//��MFCryptol Onλ ֻ�гɹ�ִ��MFAuthent����󣬸�λ������λ
    WriteRawRC ( BitFramingReg, 0x00);		//����Ĵ��� ֹͣ�շ�
    ClearBitMask ( CollReg, 0x80 );			//��ValuesAfterColl���н��յ�λ�ڳ�ͻ�����

    buffer[ 0 ] = 0x93;	//��Ƭ����ͻ����
    buffer[ 1 ] = 0x20;

    Status =PcdComMF522(PCD_TRANSCEIVE,buffer, 2,buffer,&Len);//�뿨Ƭͨ��

    if (Status==MI_OK )		//ͨ�ųɹ�
    {
        for ( uc = 0; uc < 4; uc ++ )
        {
            * ( Snr + uc )  = buffer[ uc ];			//����UID
            Snr_check ^= buffer[ uc ];
        }

        if ( Snr_check !=buffer[ uc ] )
            Status =MI_FAIL;

    }

    SetBitMask ( CollReg, 0x80 );


    return Status;


}


/*******************************************************************************************
  ��������:��RC522����CRC16
  ����:intdata������CRC16������
  Len������CRC16�������ֽڳ���
  0utData����ż�������ŵ��׵�ַ
 *����ֵ:��
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
  ��������:ѡ����Ƭ
  ����:pSnr����Ƭ���кţ�4�ֽ�
 *����ֵ: 1,�ɹ�,0��ʧ��
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
 *��������:��֤��Ƭ����
 *����:mode��������֤ģʽ
 *        0x60����֤A��Կ
 *        0x61����֤B��Կ
 *        uint8_t Addr�����ַ
 *        Key������
 *        Str����Ƭ���кţ�4�ֽ�
 *����ֵ: 1,�ɹ�,0��ʧ��
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
 *��������:д���ݵ�M1��һ��
 *����:uint8_t addr�����ַ
 *     Data��д������ݣ�16�ֽ�
 *����ֵ:1,�ɹ���0��ʧ��
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
 *��������:��ȡM1��һ������
 *����:uint8_t Aaddr�����ַ
 *         Data�����������ݣ�16�ֽ�
 *����ֵ:1,�ɹ�,0,ʧ��
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
 *��������:���Ƭ��������״̬
 *����:��
 *����ֵ:1���ɹ���0��ʧ��
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
 *��������:RC522����
 *����:��
 *����ֵ:��
 ******************************************************************************************************************/
/*
   uint8_t RC522_test(void)
   {
   char Str[30];
   uint8_t Card_ID[4];

   while (1)
   {

   PcdRequest (PICC_REQALL, Card_ID); //Ѱ��		                                                 //��ʧ���ٴ�Ѱ��

   if (PcdAnticoll(Card_ID)==MI_OK )                                                                   //����ײ�����ж��ſ������д��������Χʱ������ͻ���ƻ������ѡ��һ�Ž��в�����
   {
//sprintf(Str,"\r\n�Ž�����ID��: %02X%02X%02X%02X",Card_ID[0],Card_ID[1],Card_ID[2],Card_ID[3]);
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
    //����ͻ���,��ȡ�����к�
    if (rfid.anticoll(str) == MI_OK) {
      Serial.print("The card's number is  : ");
      //��ʾ�����к�
      for(int i = 0; i < 4; i++){
        Serial.print(0x0F & (str[i] >> 4),HEX);
        Serial.print(0x0F & str[i],HEX);
      }
      Serial.println("");
    }
    //ѡ����������Ƭ����ֹ������ȡ��ȥ�����н�����������
    rfid.selectTag(str);
  }
  rfid.halt();  //���Ƭ��������״̬
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
 *��������:RC522��ʼ��
 *����:��
 *����ֵ:��
 *****************************************************************************************************/
uint8_t rc522_init (void)
{
    spi2_init();

    RC522_Reset_Disable();

    RC522_CS_Disable();

    PcdReset();
    M500PcdConfigISOType( 'A' );//���ù�����ʽ
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
