#include "sys.h"
#include "bsp.h"
#include "bsp_i2c.h"
#include "ina219.h"
#include "protocol.h"

#include "system.h"
DEFINE_DEV_REF(ina_ref);
#define i2c_Start                       i2c_Start
#define i2c_SendByte                    i2c_SendByte
#define i2c_WaitAck                     i2c_WaitAck
#define i2c_Stop                        i2c_Stop
#define i2c_ReadByte                    i2c_ReadByte

__attribute__((unused))
static uint16_t ina219_powerDivider_mW = 10;
static uint16_t ina219_currentDivider_uA = 500;
static uint16_t ina219_calValue = 4096;

/*static uint16_t PWM = 1000;*/

uint8_t INA_REG_Write(unsigned char reg,unsigned int data);
void INA_Read_Byte_s(unsigned char reg,unsigned char *data)
{
    i2c_Start();
    i2c_SendByte(INA219_ADDRESS);
    i2c_WaitAck();
    i2c_SendByte(reg);
    i2c_WaitAck();

    i2c_Start();
    i2c_SendByte(INA219_ADDRESS+0x01);	//设置iic为读模式
    i2c_WaitAck();
    *data=i2c_ReadByte(1);
    data++;
    *data=i2c_ReadByte(0);
    i2c_Stop();
    delay_ms(5);
}
uint8_t INA_REG_Write(unsigned char reg,unsigned int data)	//写寄存器		测试成功
{
    unsigned char data_temp[2];
    uint8_t ret;
    data_temp[0]=(unsigned char )(data>>8);
    data_temp[1]=(unsigned char )(data & 0xFF);
    i2c_Start();
    i2c_SendByte(INA219_ADDRESS);	//发送INA219地址
    i2c_WaitAck();
    /*dbg_printf("wait ack:%d\n", ret);<]*/
    i2c_SendByte(reg);							//发送寄存器地址
    i2c_WaitAck();
    i2c_SendByte(data_temp[0]);						//发送高8位数据
    i2c_WaitAck();
      /*data++;<]*/
    i2c_SendByte(data_temp[1])	;					//发送低8位数据
    ret = i2c_WaitAck();
    i2c_Stop();
    delay_ms(5);
    return ret;
}


/**************************************************************************/
/*! 
    @brief  Configures to INA219 to be able to measure up to 32V and 2A
            of current.  Each unit of current corresponds to 100uA, and
            each unit of power corresponds to 2mW. Counter overflow
            occurs at 3.2A.
			
    @note   These calculations assume a 0.1 ohm resistor is present
*/
/**************************************************************************/
void ina219_SetCalibration_32V_640mA(void)
{
    uint16_t config;
    // By default we use a pretty huge range for the input voltage,
    // which probably isn't the most appropriate choice for system
    // that don't use a lot of power.  But all of the calculations
    // are shown below if you want to change the settings.  You will
    // also need to change any relevant register settings, such as
    // setting the VBUS_MAX to 16V instead of 32V, etc.

    // VBUS_MAX = 32             (Assumes 32V, can also be set to 16V)
    // VSHUNT_MAX = 0.32          (Assumes Gain 8, 320mV, can also be 0.16, 0.08, 0.04)
    // RSHUNT = 0.04               (Resistor value in ohms)

    // 1. Determine max possible current
    // MaxPossible_I = VSHUNT_MAX / RSHUNT
    // MaxPossible_I = 640mA

    // 2. Determine max expected current
    // MaxExpected_I = 640mA

    // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
    // MinimumLSB = MaxExpected_I/32767
    // MinimumLSB = 0.0000195318              (19uA per bit)
    // MaximumLSB = MaxExpected_I/4096
    // MaximumLSB = 0,0001563         (156uA per bit)

    // 4. Choose an LSB between the min and max values
    //    (Preferrably a roundish number close to MinLSB)
    // CurrentLSB = 0.00002 (20uA per bit)

    // 5. Compute the calibration register
    // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
    // Cal = 4096 (0x1000)

    ina219_calValue = 4096;

    // 6. Calculate the power LSB
    // PowerLSB = 20 * CurrentLSB
    // PowerLSB = 0.0004 (2mW per bit)

    // 7. Compute the maximum current and shunt voltage values before overflow
    //
    // Max_Current = Current_LSB * 32767
    // Max_Current = 3.2767A before overflow
    //
    // If Max_Current > Max_Possible_I then
    //    Max_Current_Before_Overflow = MaxPossible_I
    // Else
    //    Max_Current_Before_Overflow = Max_Current
    // End If
    //
    // Max_ShuntVoltage = Max_Current_Before_Overflow * RSHUNT
    // Max_ShuntVoltage = 0.32V
    //
    // If Max_ShuntVoltage >= VSHUNT_MAX
    //    Max_ShuntVoltage_Before_Overflow = VSHUNT_MAX
    // Else
    //    Max_ShuntVoltage_Before_Overflow = Max_ShuntVoltage
    // End If

    // 8. Compute the Maximum Power
    // MaximumPower = Max_Current_Before_Overflow * VBUS_MAX
    // MaximumPower = 3.2 * 32V
    // MaximumPower = 102.4W

    // Set multipliers to convert raw current/power values
    ina219_currentDivider_uA = 500;//0.32/0.04/32768  - 0.32/0.04/4096
    ina219_powerDivider_mW = 10;     // Power LSB = 1mW per bit (2/1)

    // Set Calibration register to 'Cal' calculated above	
    INA_REG_Write(INA219_REG_CALIBRATION, ina219_calValue);

    // Set Config register to take into account the settings above
    config = INA219_CONFIG_BVOLTAGERANGE_32V |
        INA219_CONFIG_GAIN_8_320MV |
        INA219_CONFIG_BADCRES_12BIT |
        INA219_CONFIG_SADCRES_12BIT_1S_532US |
        INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    INA_REG_Write(INA219_REG_CONFIG, config);
}


/**************************************************************************/
void ina219_SetCalibration_32V_320mA(void)
{
    uint16_t config;
    ina219_calValue = 8192;
    /*ina219_currentDivider_mA = 100;      // Current LSB = 40uA per bit (1000/40 = 25)*/
    ina219_powerDivider_mW = 2;         // Power LSB = 800?W per bit

    // Set Calibration register to 'Cal' calculated above	
    INA_REG_Write(INA219_REG_CALIBRATION, ina219_calValue);

    // Set Config register to take into account the settings above
    config = INA219_CONFIG_BVOLTAGERANGE_32V |
        INA219_CONFIG_GAIN_4_160MV |
        INA219_CONFIG_BADCRES_12BIT |
        INA219_CONFIG_SADCRES_12BIT_1S_532US |
        INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    INA_REG_Write(INA219_REG_CONFIG, config);
}
void ina219_SetCalibration_16V_8A(void)
{
    ina219_calValue = 4194;
    ina219_currentDivider_uA = 244;  // Current LSB = 2uA per bit (1000/50 = 20)
    ina219_powerDivider_mW = 4;     // Power LSB = 1mW per bit

    // Set Calibration register to 'Cal' calculated above 
    INA_REG_Write(INA219_REG_CALIBRATION, ina219_calValue);
    // Set Config register to take into account the settings above
}
/**************************************************************************/
/*! 
    @brief  Gets the raw bus voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
float ina219_getBusVoltage_V(void)
{
    uint16_t tmp;
    float tmp2;
    INA_Read_Byte_s(INA219_REG_BUSVOLTAGE, (unsigned char *)&tmp);
    tmp2 = (float)tmp * 0.01f;
    return tmp2;
}

/**************************************************************************/
/*! 
    @brief  Gets the raw shunt voltage (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
/*
float ina219_getShuntVoltage_mV(void)
{
    uint16_t tmp;
    float tmp2;
    INA_Read_Byte_s(INA219_REG_SHUNTVOLTAGE,  (unsigned char *)&tmp);
    tmp2 = (float)tmp * 0.001f;
    return tmp2;
}
*/
/**************************************************************************/
/*! 
    @brief  Gets the raw current value (16-bit signed integer, so +-32767)
*/
/**************************************************************************/
/*float ina219_getCurrent_mA(void)
{
    uint16_t tmp;
    float tmp2;
    INA_Read_Byte_s(INA219_REG_CURRENT , (unsigned char *)&tmp);
    tmp2 = (float)tmp / ina219_currentDivider_mA;
    return tmp2;
}
*/
uint32_t INA_GET_ShuntVoltage_uV(void)
{
    uint32_t volt;
    unsigned char data[2];
    INA_Read_Byte_s(INA219_REG_SHUNTVOLTAGE, data);
    volt = (data[0]<<8) + data[1];
    if (volt == 0xffff)
        volt = 0;
    volt *= 10;
    dbg_printf("shuntvolate:%d\n"CRLF, volt);
    return volt;
}
unsigned int INA_GET_Voltage_MV(void)	//获取电压（单位：mv）
{
    unsigned char data_temp[2];
    INA_Read_Byte_s(INA219_REG_BUSVOLTAGE, data_temp);
    dbg_printf("INA_GET_Voltage_MV:%d(%x), %d(%x)mV\r\n",data_temp[0], data_temp[0],data_temp[1], data_temp[1]);
    return (int)((((data_temp[0]<<8)+data_temp[1]) >> 3));	//右移3为去掉：bit2，CNVR，OVF三位，再乘以 4MV (官方文档规定)，得到当前总线的电压值
}
unsigned int INA_GET_Current_MA(void)
{
    unsigned char data_temp[2] = { 0 };
    unsigned int tmp;
    /*INA_REG_Write(INA219_REG_CALIBRATION, INA_CAL);*/
    INA_Read_Byte_s(INA219_REG_CURRENT, data_temp);
    /*mid_send_to_host("INA_GET_Current_MA:%x, %x"CRLF,data_temp[0], data_temp[1]);*/

    tmp = ((data_temp[0] << 8) | data_temp[1]) * ina219_currentDivider_uA;
    // Now we can safely read the CURRENT register!
    /*ina219_getCurrent_mA();*/
    /*INA_Read_Byte_s(INA219_REG_SHUNTVOLTAGE, data_temp);
    dbg_printf("INA219_REG_SHUNTVOLTAGE:%d(%x), %d(%x)\r\n",data_temp[0], data_temp[0],data_temp[1], data_temp[1]);
    INA_Read_Byte_s(INA219_REG_CALIBRATION,data_temp);
    dbg_printf("INA219_REG_CALIBRATION:%d(%x), %d(%x)"CRLF,data_temp[0], data_temp[0],data_temp[1], data_temp[1]);*/
    return tmp;

}
#if 0
uint16_t INA219_GetVal(uint8 mode)
{
    unsigned char data_temp[2];
    if (INA_REG_Write(INA219_REG_CONFIG,INA219_CONFIG_value | INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED))
        return 0;

    _voltage = INA219_readreg16(INA219_REG_BUSVOLTAGE);
    _voltage = (_voltage >> 3) * 4;
    _current = INA219_readreg16(INA219_REG_CURRENT);
    if (((_current) >> (15)) & 0x01) { //nagative
        _current = 0;
        _shuntvoltage = 0;
        _power = 0;
    } else {
        _current = _current / 100;
        _shuntvoltage = INA219_readreg16(INA219_REG_SHUNTVOLTAGE) / 1000;
        _power = INA219_readreg16(INA219_REG_POWER) / 10;
        _power = _power * 2;
    }
    if (!INA219_writereg16(INA219_REG_CONFIG, (INA219_CONFIG_VALUE | INA219_MODE_POWER_DOWN)))
        return 0;
    return 1;
    //os_printf("-%s-%s GET[%d]=[%d] \r\n", __FILE__, __func__, val,res);   
    return 0;
}
#endif
unsigned int INA_GET_Power_MW(void)		//获取当前功率（单位：mw）
{
    unsigned char data_temp[2];
    INA_Read_Byte_s(INA219_REG_POWER,data_temp);
    dbg_printf("INA_GET_Power_MW:%x, %xmw"CRLF,data_temp[0], data_temp[1]);
    dbg_printf("INA_GET_Power_MW:%dmw"CRLF, ((data_temp[0] << 8) | data_temp[1]));
    /*return (int)((((data_temp[0]<<8)+data_temp[1]))*IAN_I_LSB);		//得到寄存器的值在乘以每位对应的值（IAN_I_LSB）得到实际的电流*/
    return (int)(((data_temp[0]<<8)+data_temp[1])*ina219_powerDivider_mW);	//得到寄存器的值在乘以每位对应的值（INA_Power_LSB）得到实际的功率
}
static uint8_t ina_ok = 0;

void current_det(void)
{
    uint32_t volt;
    float tmp;
    DEV_GET_REF(ina_ref);
    ina_ok = INA_REG_Write(INA219_REG_CALIBRATION, ina219_calValue);
    /*volt = INA_GET_Voltage_MV() * 4;*/
    volt = INA_GET_ShuntVoltage_uV();
    INA_REG_Write(INA219_REG_CALIBRATION, ina219_calValue);
    set_sys_current_motor((int)INA_GET_Current_MA());	//得到电流（mA）
    tmp = volt / INA_R;/**/
    set_sys_current_motor((uint16_t)(tmp / 1000));
    if (ABS(get_sys_current_total()) <= get_sys_current_motor()) {
        set_sys_current_motor(0);
    }
#if !DEBUG
    volt = volt;
#endif
    DEV_PUT_REF(ina_ref);
    /*INA_REG_Write(INA219_REG_CALIBRATION, ina219_calValue);*/
    /*Power_mw = INA_GET_Power_MW();//得到功率（mW）
    dbg_printf("curr:%dmw\r\n",Power_mw);*/
    /*Power_w_float=(float)Power_mw/1000;//得到浮点型功率（mW）<]*/
    /*dbg_printf("volt:%dmv"CRLF, volt);*/
    mid_send_to_host("Curr:%dmA"CRLF,get_sys_current_motor());
}
uint8_t ina_init(void)	
{
    uint8_t ret = 0;
    ret = INA_REG_Write(INA219_REG_CONFIG,INA219_CONFIG_value);
    ret = INA_REG_Write(INA219_REG_CALIBRATION, INA_CAL);
    /*ina219_SetCalibration_32V_640mA();*/
    return ret;
}
uint8_t ina219_self_check(void)	//读两位数据
{
    uint8_t ret = 0;
    /*ina_init();
    i2c_Start();
    i2c_SendByte(INA219_ADDRESS);
    ret = i2c_WaitAck(); 
    i2c_Stop();*/
    ret = ina_ok;
    if(!ret) {
        mid_send_to_host("Currrent self-check ok"CRLF);
        ret = 0;
    } else {
        mid_send_to_host("Currrent self-check fail"CRLF);
        ret = 1;
    }
    return ret;
}
