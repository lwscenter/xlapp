#ifndef PROTOCOL_V2LE7NLH
#define PROTOCOL_V2LE7NLH
#ifdef __cplusplus
extern "C" { /*}*/
#endif

#define PKT_STX                 0x8A
#define PKT_ETX                 0xA8

#define PKT_LEN_MAX             4


#define prot_data               sys_parm.protda
#define prot_pkt_count          sys_parm.p_cnt
#define rfid_ack                sys_parm.rfid_ack

void mid_send_to_host_ex(uint8_t *data, int size);

int handle_port_cmd(void);

void handle_led(uint8_t sta);

enum {
    BIT_LED_OFF = 0,
    BIT_LED_RED_ON = BIT(0),
    BIT_LED_GREEN_ON = BIT(1),
    BIT_LED_BLUE_ON = BIT(2),
    BIT_LED_RED_FLASH = BIT(3),
    BIT_LED_GREEN_FLASH = BIT(4),
    BIT_LED_BLUE_FLASH = BIT(5),
    BIT_LED_STA_ALL = 0xff,
};
enum {
    BIT_BUZZER_OFF,
    BIT_BUZZER_ON,
};
enum {
    PROT_CMD_Temp_Low = 0x23,							//设置加热板温度最低值
    PROT_CMD_Temp_High = 0x50,						//设置加热板温度最高值
    PROT_CMD_START = 0xA0,
    PORT_CMD_MOTOR_FOWARD = 0xA1,        //前进
    PORT_CMD_MOTOR_BACKWARD = 0xA2,        //后退
    PORT_CMD_MOTOR_STOP = 0xA3,        //停止
    PORT_CMD_COULOMB_CLEAR = 0xAA,        //库仑计电量清零
    PORT_CMD_LEDR_ON = 0xAE,        //红灯亮	PA4输出低电平
    PORT_CMD_LEDG_ON = 0xAF,        //绿灯亮	PA5输出低电平
    PORT_CMD_LEDB_ON = 0xB0,        //蓝灯亮	PA6输出低电平
    PORT_CMD_LEDR_FLASH = 0xB1,        //红灯闪烁
    PORT_CMD_LEDG_FLASH = 0xB2,        //绿灯闪烁
    PORT_CMD_LEDB_FLASH = 0xB3,        //蓝灯闪烁
    PORT_CMD_BATTERY_CHECK = 0xB4,        //
    PORT_CMD_GYRODATA = 0xB5,        //获取陀螺仪原始数据
    PORT_CMD_SELF_TEST = 0xB6,        //各个外设工作状态检测
    PORT_CMD_LED_OFF = 0xB7,        //关闭RGB灯
    PORT_CMD_MOTOR_CTL_TURN = 0xB8,
    PORT_CMD_BUZZER_ON = 0xB9,        //蜂鸣器发出声音
    PORT_CMD_BUZZER_OFF = 0xBA,        //关闭蜂鸣器
    PORT_CMD_BATTERY_PERCENT = 0xBE,        //
    PROT_CMD_END,
    PORT_CMD_INTER_BOOT = 0xda,
    PORT_CMD_VERSION = 0xc0,
    PROT_CMD_MOTOR_FORWARD_EX = 0xc1,
    PROT_CMD_MOTOR_BACKWARD_EX = 0xc2,

    PORT_CMD_RFID_ACK = 0xe0,

    PORT_CMD_TEST1 = 0x71,
    PORT_CMD_TEST2,
    PORT_CMD_TEST3,
    PORT_CMD_TEST4,

};

/*
00H 正常
01H 校验错误
02H 未定义命令
04H 未定义
08H 未定义
10H 内部错误
20H 米轮坏
40H 未定义1
80H 未定义2
*/
enum enc_status_t {
    ENC_STATUS_NORMAL = 0,
    ENC_STATUS_CHK_ERROR = 1,
    ENC_STATUS_UNDEFINE_CMD = 2,
    ENC_STATUS_UNDEFINE1 = 4,
    ENC_STATUS_UNDEFINE2 = 8,
    ENC_STATUS_INTER_ERROR = 16,
    ENC_STATUS_ENC_ERROR = 32,
    ENC_STATUS_UNDEFINE3 = 64,
    ENC_STATUS_UNDEFINE4 = 128,
};
#define CO_SHORT(v1, v2)        ((v2 << 8) | v1)


enum enc_cmd_t {
    /*
       01H 写间隔数
       02H 写密码文本
       03H 计数清零
       04H 清除已发中断
       05H 发中断请求
       06H 下拉保持时间
       07H 设置频率
       */
    ENC_CMD_WRITE_INTERVAL = 0x01,
    ENC_CMD_WRITE_PASSWD = 0x02,
    ENC_CMD_WRITE_PASSWD_EX = 0x02,
    ENC_CMD_WRITE_COUNTER_CLEAR = 0x03,
    ENC_CMD_WRITE_INTERRUPT_CLEAR = 0x04,
    ENC_CMD_WRITE_INTERRUPT_REQUEST = 0x05,
    ENC_CMD_WRITE_LEVEL_LATCH = 0x06,
    ENC_CMD_WRITE_FREQ_SETTING = 0x07,
    ENC_CMD_WRITE_IO_STATUS = 0x08,



    ENC_CMD_WRITE_ALL_CLEAR = 0x1f,



    ENC_CMD_WRITE_END,


/**
 * 读取计数器
 * 读取已发中断数
 * 读密码文本
 * 读状态
 * 上报中断发出事件
 *
 */
    ENC_CMD_READ_COUNTER = 0x20,
    ENC_CMD_READ_INTERRUPTS_NUM = 0x21,
    ENC_CMD_READ_PASSWD = 0x22,
    ENC_CMD_READ_STATUS = 0x23,
    ENC_CMD_REPORT_EVENT = 0x24,
    ENC_CMD_READ_IO = 0x25,



    ENC_CMD_READ_VER = 0x55,


    ENC_CMD_INTER_BOOT = 0xf0,
};

#define SEND_BUF_LEN            8
#define SEND_BUF_LEN_MASK       (SEND_BUF_LEN - 1)
#if (SEND_BUF_LEN & SEND_BUF_LEN_MASK)
#error "snd buf len error"
#endif
#define RECV_BUF_LEN            16
#define RECV_BUF_LEN_MASK       (RECV_BUF_LEN - 1)
#define ENC_PROT_DATA_LEN_MAX   (RECV_BUF_LEN - 5)
#if (RECV_BUF_LEN & RECV_BUF_LEN_MASK)
#error "len is power of 2"
#endif

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: PROTOCOL_V2LE7NLH */
