#include "sys.h"
#include "system.h"
#include "protocol.h"
#include "motor.h"
#include "ds2781.h"

/*H   L-Lo    L-Hi    C   DATA    CHK 03H*/

static unsigned char get_prot_data(unsigned char *data)
{
    uint8_t tmp;
#if USE_NEW_PROT
    if (!prot_pkt_count)
        return 1;
#endif
    tmp = queue_out(&prot_data, data);/*randv*/
#if USE_NEW_PROT
    if (!tmp)
        tmp = queue_out(&prot_data, data + 1);/*cmd*/
    if (!tmp && prot_pkt_count > 0)
        --prot_pkt_count;
#endif
    return tmp;
}

void mid_send_to_host_ex(uint8_t *data, int size)
{
    while (size--) {
        send_to_host_c(*data++);
    }
    /*set_s_tx_flag(0);*/
}
__attribute__((unused))
static void print_led_pin(void)
{
    dbg_printf("R:%d\t", LED_RED_IN_PIN);
    dbg_printf("G:%d\t", LED_GREEN_IN_PIN);
    dbg_printf("B:%d\n", LED_BLUE_IN_PIN);
}
void handle_led(uint8_t sta)
{
    static uint8_t val;
    if (sta & BIT_LED_RED_ON)
        LED_RED_ON();
    else if (sta & BIT_LED_RED_FLASH)
        LED_RED_PIN = val;
    else
        LED_RED_OFF();
    if (sta & BIT_LED_GREEN_ON)
        LED_GREEN_ON();
    else if (sta & BIT_LED_GREEN_FLASH)
        LED_GREEN_PIN = val;
    else
        LED_GREEN_OFF();
    if (sta & BIT_LED_BLUE_ON)
        LED_BLUE_ON();
    else if (sta & BIT_LED_BLUE_FLASH) {
        LED_BLUE_PIN = val;
        /*LED_BLUE_FLASH();*/
    } else {
        LED_BLUE_OFF();
    }
    val = !val;
#if DEBUG
    /*print_led_pin();*/
#endif
}
/**
命令与对应功能
A1	前进
A2	后退
A3	停止
AA	库仑计电量清零
AE	红灯亮	PA4输出低电平
AF	绿灯亮	PA5输出低电平
B0	蓝灯亮	PA6输出低电平
B1	红灯闪烁
B2	绿灯闪烁
B3	蓝灯闪烁
B5	获取陀螺仪原始数据
B6	各个外设工作状态检测
B9	蜂鸣器发出声音
BA	关闭蜂鸣器
B7	关闭RGB灯
*****/
void ds2781_test_rst(void);
void ds2781_ex_test_rst(void);
void current_det(void);
extern unsigned int Heat_Temp_MAX;		//加热板加热最高温度


#if APP_JUMP_ISP
void reset2isp(void);
#endif

#if USE_NEW_PROT
//extern unsigned int Code_Num;	//记录接收到的指令数

void prot_pkt_ack(uint8_t *cmds)
{
    char buf[16] = { 0 };
    sprintf(buf, "%X%X%X%X\n", PKT_STX, cmds[0], cmds[1], PKT_ETX);
    mid_send_to_host(buf);
//		printf("%d\n",Code_Num);
}
#endif
void disable_app_irq(void)
{
    TIM_DeInit(TIM3);
    USART_DeInit(DBG_UART);
    USART_DeInit(USART2);
    tick_uninit();
}
static void iap_load_app(u32 appxaddr)
{
    static void (*jump)(void);
    /*__set_PRIMASK(1);*/
    if(((*(volatile vu32 *)appxaddr)&0x2FFE0000)==0x20000000) {
        /*dbg_printf("appxaddr:%x\n", (*(uint32_t *)appxaddr));*/
       dbg_printf("boot start"CRLF);
        disable_app_irq(); __disable_irq();
        jump = (void (*)(void))*(vu32*)(appxaddr+4);
        __set_MSP(*(vu32*)appxaddr);
        jump();
    }
}

void jump_to_boot(uint32_t addr)
{
    if(((*(volatile uint32_t *)(addr + 4)) & 0xFF000000) == FLASH_BASE) {
        iap_load_app(addr);
    } else {
        dbg_printf("jng"CRLF);
    }
}
int handle_port_cmd(void)
{
    #define PROT_LED_FREQ               2
    uint8_t ret;
#if USE_NEW_PROT
    uint8_t cmds[2] = { 0 };
    #define cmd                         cmds[1]
    ret = get_prot_data(cmds);
#else
    uint8_t cmds;
    #define cmd                         cmds
    ret = get_prot_data(&cmds);
#endif
	
//    PROT_CMD_Temp_Low = 0x23,							//设置加热板温度最低值
//    PROT_CMD_Temp_High = 0x50,						//设置加热板温度最高值
	
    if (ret)
        return 1;
    dbg_printf("cmd:%X\n", cmd);

    switch(cmd) {
    case PORT_CMD_MOTOR_FOWARD:
    case PROT_CMD_MOTOR_FORWARD_EX:
        if (get_sys_bat_low_alarm() > BAT_LOW_ALARM_WARNING) {
            mid_send_to_host("Very low battery\n");
        } else {
            motor_foward();
        }
        if (cmd == PROT_CMD_MOTOR_FORWARD_EX)
            motor_foward();
        break;
    case PORT_CMD_INTER_BOOT:
        dbg_printf("Now reboot..."CRLF);
		/*jump_to_boot(FLASH_BASE);*/
        REBOOT();
        /*break;*/
    case PORT_CMD_MOTOR_BACKWARD:
    case PROT_CMD_MOTOR_BACKWARD_EX:
        if (get_sys_bat_low_alarm() > BAT_LOW_ALARM_WARNING) {
            mid_send_to_host("Very low battery\n");
        } else {
            motor_backward();
        }
        if (cmd == PROT_CMD_MOTOR_BACKWARD_EX)
            motor_backward();
        break;
    case PORT_CMD_BATTERY_CHECK:
        break;
    case PORT_CMD_BATTERY_PERCENT:
        /*ds2781_default();*/
        /*get_battery_percentage();*/
        break;
    case PORT_CMD_MOTOR_STOP:
        motor_stop();
        break;
    case PORT_CMD_MOTOR_CTL_TURN:
        MOTOR_CTL_TURN();
        break;
    case PORT_CMD_TEST1:
        set_buzzer_freq(5);
#if APP_JUMP_ISP
        reset2isp();
#endif
        /*ds2781_test_rst();*/
        /*ds2781_ex_test_rst();*/
        /*get_coulomb_data(1);*/
        break;
    case PORT_CMD_TEST2:
        set_buzzer_freq(10);
        /*get_coulomb_data();*/
        /*dbg_printf("temp:%d", DS2781_EX_Get_Temp());*/
        break;
    case PORT_CMD_TEST3:
        current_det();
        /*dbg_printf("volate:%f", voltage());*/
        break;
    case 0xab:
        /*ds2781_default();*/
        break;
    case 0xac:
        /*kulunji();*/
        break;
    case 0xad:
        motor_ad();
    case PORT_CMD_COULOMB_CLEAR:
        /*get_coulomb_data(1);*/
        /*acr_clear();*/
        break;
    case PORT_CMD_LEDR_ON:
        set_led_freq(PROT_LED_FREQ);
        SET_LED_RED_STATUS(BIT_LED_RED_ON);
        break;
    case PORT_CMD_LEDG_ON:
        set_led_freq(PROT_LED_FREQ);
        SET_LED_GREEN_STATUS(BIT_LED_GREEN_ON);
        break;
    case PORT_CMD_LEDB_ON:
        set_led_freq(PROT_LED_FREQ);
        SET_LED_BLUE_STATUS(BIT_LED_BLUE_ON);
        break;
    case PORT_CMD_LEDR_FLASH:
        set_led_freq(PROT_LED_FREQ);
        SET_LED_RED_STATUS(BIT_LED_RED_FLASH);
        break;
    case PORT_CMD_LEDG_FLASH:
        set_led_freq(PROT_LED_FREQ);
        SET_LED_GREEN_STATUS(BIT_LED_GREEN_FLASH);
        break;
    case PORT_CMD_LEDB_FLASH:
        set_led_freq(PROT_LED_FREQ);
        SET_LED_BLUE_STATUS(BIT_LED_BLUE_FLASH);
        break;
    case PORT_CMD_GYRODATA:
        break;
    case PORT_CMD_SELF_TEST:
        dev_self_check();
        break;
    case PORT_CMD_BUZZER_ON:
        set_buzzer_on();
        break;
    case PORT_CMD_BUZZER_OFF:
        set_buzzer_off();
        break;
    case PORT_CMD_LED_OFF:
        CLR_LED_ALL_STATUS();
        break;
    case PORT_CMD_VERSION:
        mid_send_to_host("Version:");
        mid_send_to_host(VERSION);
        mid_send_to_host(CRLF);
        break;
    default:
				if((PROT_CMD_Temp_Low <= cmd)&&(cmd <= PROT_CMD_Temp_High))	//指令在PROT_CMD_Temp_Low，PROT_CMD_Temp_High范围内都认为是设置温度
				{
					dbg_printf("Set Temp :%d.\n", cmd);
					Heat_Temp_MAX = cmd;		//加热板加热最高温度
					
				}else
				{
					return 1;		
				}
    }
#if USE_NEW_PROT
    prot_pkt_ack(cmds);
#endif
    return 0;
#undef PROT_LED_FREQ
#undef cmd
}

#if 0

void send_to_master(uint8_t len)
{
    uint8_t i = 0;
    while (len--) {
        /*send_byte(SEND_DATA_BUF[i++]);*/
    }
}

void handle_sys_cmd(void)
{

}
uint8_t enc_cdata_check(uint8_t *data, uint16_t len)
{
    uint8_t tmp = 0;
    while (len--) {
        tmp += *data++;
    }
    return (tmp & 0xff);
}
struct enc_protocol_t enc_data;
void enc_cdata_read_cmd_reply(void)
{
#define cmd             get_edata_cmd()
    uint8_t idx;
    SEND_DATA_BUF[0] = PKT_STX;
    SEND_DATA_BUF[1] = 3;
    SEND_DATA_BUF[2] = 0;
    SEND_DATA_BUF[3] = cmd;
    idx = 6;
    if (cmd == ENC_CMD_READ_COUNTER) {
        SEND_DATA_BUF[4] = get_sys_enc_interval();
        SEND_DATA_BUF[5] = get_sys_enc_interval() >> 8;
    } else if (cmd == ENC_CMD_READ_INTERRUPTS_NUM) {
        SEND_DATA_BUF[4] = get_sys_enc_already_s_ints();
        SEND_DATA_BUF[5] = get_sys_enc_already_s_ints() >> 8;
    } else if (cmd == ENC_CMD_READ_IO) {
        SEND_DATA_BUF[4] = get_io_status();
        idx = 5;
    } else if (cmd == ENC_CMD_READ_STATUS) {
        SEND_DATA_BUF[4] = get_sys_status();
        idx = 5;
    }
    SEND_DATA_BUF[idx] = enc_cdata_check(SEND_DATA_BUF, idx);
    SEND_DATA_BUF[idx + 1] = PKT_ETX;
    send_to_master(idx + 2);

#undef cmd
}

void enc_cdata_send_ack(void)
{
#define cmd             get_edata_cmd()
#define ret             get_edata_ret()
    uint8_t tmp = 1;
    uint8_t idx = 0;
    if (ret == ENC_STATUS_UNDEFINE1)
        return;
    SEND_DATA_BUF[0] = PKT_STX;
    if (cmd == ENC_CMD_REPORT_EVENT) {
        SEND_DATA_BUF[1] = 1;
        idx = 6;
    } else {
        SEND_DATA_BUF[1] = 2;
        SEND_DATA_BUF[4] = ret;
        idx = 7;
    }
    SEND_DATA_BUF[2] = 0;
    SEND_DATA_BUF[3] = cmd;
    SEND_DATA_BUF[idx - 2] = enc_cdata_check(SEND_DATA_BUF, idx - 2);
    SEND_DATA_BUF[idx - 1] = PKT_ETX;
    send_to_master(idx);
#undef cmd

#undef ret
}
#endif
