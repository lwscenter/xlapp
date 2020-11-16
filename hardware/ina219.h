#ifndef _INA219_H_
#define _INA219_H_
// I2C Address Options
/*
#define INA219_I2C_ADDRESS_CONF_0               (u8)(0x40 << 1)     // A0 = GND, A1 = GND
#define INA219_I2C_ADDRESS_CONF_1               (u8)(0x41 << 1)     // A0 = VS+, A1 = GND
#define INA219_I2C_ADDRESS_CONF_2               (u8)(0x42 << 1)     // A0 = SDA, A1 = GND
#define INA219_I2C_ADDRESS_CONF_3               (u8)(0x43 << 1)     // A0 = SCL, A1 = GND
#define INA219_I2C_ADDRESS_CONF_4               (u8)(0x44 << 1)     // A0 = GND, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_5               (u8)(0x45 << 1)     // A0 = VS+, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_6               (u8)(0x46 << 1)     // A0 = SDA, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_7               (u8)(0x47 << 1)     // A0 = SCL, A1 = VS+
#define INA219_I2C_ADDRESS_CONF_8               (u8)(0x48 << 1)     // A0 = GND, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_9               (u8)(0x49 << 1)     // A0 = VS+, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_A               (u8)(0x4A << 1)     // A0 = SDA, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_B               (u8)(0x4B << 1)     // A0 = SCL, A1 = SDA
#define INA219_I2C_ADDRESS_CONF_C               (u8)(0x4C << 1)     // A0 = GND, A1 = SCL
#define INA219_I2C_ADDRESS_CONF_D               (u8)(0x4D << 1)     // A0 = VS+, A1 = SCL
#define INA219_I2C_ADDRESS_CONF_E               (u8)(0x4E << 1)     // A0 = SDA, A1 = SCL
#define INA219_I2C_ADDRESS_CONF_F               (u8)(0x4F << 1)     // A0 = SCL, A1 = SCL
*/
/*#define INA219_ADDRESS  (0x40 << 1)     // A0 = GND, A1 = GND*/
#define INA219_ADDRESS  (0x80)

// 寄存器地址
#define INA219_REG_CONFIG                       (u8)(0x00)      // 模式配置寄存器 (R/W)
#define INA219_REG_SHUNTVOLTAGE                 (u8)(0x01)      // 分流电阻电压寄存器 (R)
#define INA219_REG_BUSVOLTAGE                   (u8)(0x02)      // 总线电压寄存器 (R)
#define INA219_REG_POWER                        (u8)(0x03)      // 功率寄存器 (R)
#define INA219_REG_CURRENT                      (u8)(0x04)      // 电流寄存器 (R)
#define INA219_REG_CALIBRATION                  (u8)(0x05)      // 基准值寄存器 (R/W)

//寄存器（0x01）参数定义
#define INA219_CONFIG_RESET                     (0x8000)  // Reset Bit

#define INA219_CONFIG_BVOLTAGERANGE_MASK        (0x2000)  // Bus Voltage Range Mask
#define INA219_CONFIG_BVOLTAGERANGE_16V         (0x0000)  // 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V         (0x2000)  // 0-32V Range

#define INA219_CONFIG_GAIN_MASK                 (0x1800)  // Gain Mask
#define INA219_CONFIG_GAIN_1_40MV               (0x0000)  // Gain 1, 40mV Range		//配置检测电阻上的最大检测电压
#define INA219_CONFIG_GAIN_2_80MV               (0x0800)  // Gain 2, 80mV Range
#define INA219_CONFIG_GAIN_4_160MV              (0x1000)  // Gain 4, 160mV Range
#define INA219_CONFIG_GAIN_8_320MV              (0x1800)  // Gain 8, 320mV Range

#define INA219_CONFIG_BADCRES_MASK              (0x0780)  // Bus ADC Resolution Mask
#define INA219_CONFIG_BADCRES_9BIT              (0x0080)  // 9-bit bus res = 0..511
#define INA219_CONFIG_BADCRES_10BIT             (0x0100)  // 10-bit bus res = 0..1023
#define INA219_CONFIG_BADCRES_11BIT             (0x0200)  // 11-bit bus res = 0..2047
#define INA219_CONFIG_BADCRES_12BIT             (0x0400)  // 12-bit bus res = 0..4097

#define INA219_CONFIG_SADCRES_MASK              (0x0078)  // Shunt ADC Resolution and Averaging Mask
#define INA219_CONFIG_SADCRES_9BIT_1S_84US      (0x0000)  // 1 x 9-bit shunt sample
#define INA219_CONFIG_SADCRES_10BIT_1S_148US    (0x0008)  // 1 x 10-bit shunt sample
#define INA219_CONFIG_SADCRES_11BIT_1S_276US    (0x0010)  // 1 x 11-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_1S_532US    (0x0018)  // 1 x 12-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_2S_1060US   (0x0048)	 // 2 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_4S_2130US   (0x0050)  // 4 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_8S_4260US   (0x0058)  // 8 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_16S_8510US  (0x0060)  // 16 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_32S_17MS    (0x0068)  // 32 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_64S_34MS    (0x0070)  // 64 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_128S_69MS   (0x0078)  // 128 x 12-bit shunt samples averaged together

#define INA219_CONFIG_MODE_MASK                 (0x0007)  // Operating Mode Mask
#define INA219_CONFIG_MODE_POWERDOWN            (0x0000)
#define INA219_CONFIG_MODE_SVOLT_TRIGGERED      (0x0001)
#define INA219_CONFIG_MODE_BVOLT_TRIGGERED      (0x0002)
#define INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED  (0x0003)
#define INA219_CONFIG_MODE_ADCOFF               (0x0004)
#define INA219_CONFIG_MODE_SVOLT_CONTINUOUS     (0x0005)
#define INA219_CONFIG_MODE_BVOLT_CONTINUOUS     (0x0006)
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)	
#if ___
// By default we use a pretty huge range for the input voltage,
  // which probably isn't the most appropriate choice for system
  // that don't use a lot of power.  But all of the calculations
  // are shown below if you want to change the settings.  You will
  // also need to change any relevant register settings, such as
  // setting the VBUS_MAX to 16V instead of 32V, etc.

  // VBUS_MAX     = 16V   (Assumes 16V, can also be set to 32V)
  // VSHUNT_MAX   = 0.32  (Assumes Gain 8, 320mV, can also be 0.16, 0.08, 0.04)
  // RSHUNT       = 0.04   (Resistor value in ohms)

  // 1. Determine max possible current
  // MaxPossible_I = VSHUNT_MAX / RSHUNT
  // MaxPossible_I = 8A

  // 2. Determine max expected current
  // MaxExpected_I = 8A

  // 3. Calculate possible range of LSBs (Min = 15-bit, Max = 12-bit)
  // MinimumLSB = MaxExpected_I/32767
  // MinimumLSB = 0.00048            (0.48mA per bit)
  // MaximumLSB = MaxExpected_I/4096
  // MaximumLSB = 0,00390            (3.9mA per bit)

  // 4. Choose an LSB between the min and max values
  //    (Preferrably a roundish number close to MinLSB)
  // CurrentLSB = 0.00050            (500uA per bit)

  // 5. Compute the calibration register
  // Cal = trunc (0.04096 / (Current_LSB * RSHUNT))
  // Cal = 4096 (0x1000)

  ina219_calValue = 0x1000;

  // 6. Calculate the power LSB
  // PowerLSB = 20 * CurrentLSB
  // PowerLSB = 0.01 (10mW per bit)

  // 7. Compute the maximum current and shunt voltage values before overflow
  //
  // Max_Current = Current_LSB * 32767
  // Max_Current = 16.3835A before overflow
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
  // MaximumPower = 1.6 * 16V
  // MaximumPower = 256W

  // Set multipliers to convert raw current/power values
  ina219_current_LSB_uA = 500;     // Current LSB = 500uA per bit
  ina219_power_LSB_mW = 10;        // Power LSB = 10mW per bit = 20 * Current LSB

#endif
//INA配置的参数
#define INA_R                                   0.04//阻值大小（单位ohm）
/*#define INA_R                                   4//阻值大小（单位ohm）*/
#define INA_I_MAX                               3//硬件最大检测电流（单位：A）	电压/INA_R
#define IAN_I_LSB                               0.1//uA(0x04)检测出值对应每位电流值	选择接近 = (INA_I_MAX/32767(2^15))*1000
#define INA_Power_LSB                           2//(0x03)检测出值对应每位功率值	= 20 * IAN_I_LSB	(MW)
#define INA_CAL                                 4096//(0x05)寄存器基准值
/*#define ina219_currentDivider_mA                10// Current LSB = 100uA per bit (1000/100 = 10)
#define ina219_powerDivider_mW                  2// Power LSB = 1mW per bit (2/1)*/

//根据上面定义设置 配置寄存器（0x00）的值：0x3C4F
#define INA219_CONFIG_value                     INA219_CONFIG_BVOLTAGERANGE_16V| \
                                                INA219_CONFIG_GAIN_8_320MV | \
                                                INA219_CONFIG_BADCRES_12BIT | \
                                                INA219_CONFIG_SADCRES_12BIT_8S_4260US| \
                                                INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS


uint8_t ina_init(void);

void current_det(void);

uint8_t ina219_self_check(void);


#endif
