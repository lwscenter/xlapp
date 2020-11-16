#include "sys.h"
#include "bsp.h"
#include "timer.h"
#include "system.h"
#include "motor.h"

#define MOTOR_PWM_START()               TIM_CtrlPWMOutputs(TIM1, ENABLE)
#define MOTOR_PWM_STOP()                TIM_CtrlPWMOutputs(TIM1, DISABLE)
/*0.3A - 0.6A*/

static uint32_t PWM;
__attribute__((unused))
static void print_motor_level(void)
{
    dbg_printf(">>>>>>>>>>  CTL: %d\t", MOTOR_CTL_IN_PIN);
    dbg_printf("TS: %d\t", MOTOR_TS_IN_PIN);
    dbg_printf("ZF: %d\n", MOTOR_ZF_IN_PIN);

}

#define pwm_diff(_a, _b)			(int)((_b) - (_a))
#define MOTOR_CURR_CHK1                         1200
#define MOTOR_CURR_CHK2                         1400
#define MOTOR_PWM_DIFF_ABNORMAL1                500
#define MOTOR_PWM_DIFF_ABNORMAL2                600
void motor_check_abnormal(void)
{
    static uint8_t pcount, ccount;
    static uint32_t last_pwm;
    dbg_printf("motor current:%d\n", get_sys_current_motor());
    dbg_printf("motor pwm:%d - %d\n", last_pwm, PWM);
    if (get_motor_sta() != MOTOR_ACT_STOP) {
        if (get_sys_current_motor() > MOTOR_CURR_CHK1) {
            ++ccount;
        } else {
            ccount = 0;
        }
        if (pwm_diff(last_pwm, PWM) > MOTOR_PWM_DIFF_ABNORMAL1)
            ++pcount;
        else
            pcount = 0;
        if (get_sys_current_motor() > MOTOR_CURR_CHK2)
            ccount += 2;
        if (pwm_diff(last_pwm, PWM) > MOTOR_PWM_DIFF_ABNORMAL2)
            pcount += 2;
        if (pcount > 6) {
            pcount = 0;
            mid_send_to_host("Motor is stalled! (p)"CRLF);
            motor_stop();
        } else if (pcount > 2) {
            mid_send_to_host("Motor seems to be blocked (p)"CRLF);
        }
        if (ccount > 6) {
            ccount = 0;
            mid_send_to_host("Motor is stalled! (c)"CRLF);
            motor_stop();
        } else if (ccount > 2) {
            mid_send_to_host("Motor seems to be blocked (c)"CRLF);
        }
    }
    dbg_printf("pwm:%d, last_pwm:%d (%d)"CRLF, PWM, last_pwm, pwm_diff(last_pwm, PWM));

    last_pwm = PWM;
}
void motor_pwm_count(void)
{
    static uint8_t last_level = 0;
    if (MOTOR_PWM_SPD_IN_PIN != last_level) {
        ++PWM;
    }
    last_level = MOTOR_PWM_SPD_IN_PIN;
}

///////////////////////////////////////////////////////////
#define NO						0				//切换方向标记，不切换
#define YES						1				//切换方向标记，切换
#define DIR_CW				0				//方向标记，正转
#define DIR_CCW				1				//方向标记，反转
#define Speed_Min			AccStep[0]
//#define Speed_Max			1000
#define Repeat_S			1				//S曲线加速单个值重复次数，该值越大，加速越平稳，加速时间越长
#define Repeat_A			1				//匀加速单个值重复次数，值太小会导致电机停机抖动
#define Stop_Step			10			//减速停止步距，值越大停止越快。

struct Motor_Str
{
	unsigned char DIR;					//方向标志
	unsigned char Switch;				//切换标志
	unsigned int 	ReTimes;			//同一脉冲频率重复次数
	unsigned char EN;						//使能标志
	unsigned int 	StepNum;			//S型曲线数组位置
	unsigned int 	Set;					//速度设置值
	unsigned int 	Current;			//速度当前值,定时器自动重装载值和比较值
} Motor;

//#define ACC_STEP_NUM 400
//const unsigned short AccStep[ACC_STEP_NUM] = {					//S曲线数组
//2959,2941,2915,2899,2874,2857,2841,2817,2801,2778,
//2755,2740,2717,2703,2681,2660,2646,2625,2604,2584,
//2564,2551,2532,2513,2494,2475,2457,2439,2421,2404,
//2387,2370,2347,2331,2315,2299,2278,2262,2247,2227,
//2212,2198,2179,2165,2146,2132,2114,2096,2083,2066,
//2049,2033,2020,2004,1988,1972,1957,1942,1927,1912,
//1898,1880,1866,1852,1838,1821,1808,1795,1779,1767,
//1751,1739,1724,1709,1698,1684,1669,1656,1645,1631,
//1618,1605,1592,1580,1565,1553,1541,1529,1515,1504,
//1493,1479,1468,1456,1443,1433,1420,1408,1397,1385,
//1374,1362,1351,1340,1330,1319,1309,1297,1287,1276,
//1266,1255,1245,1235,1224,1214,1205,1195,1185,1175,
//1164,1155,1145,1136,1126,1117,1107,1099,1089,1080,
//1072,1063,1054,1045,1036,1028,1019,1010,1002,994,
//939,929,920,910,900,890,881,871,862,853,
//842,833,824,814,805,796,786,777,768,759,
//750,741,732,723,714,705,696,688,679,670,
//662,653,645,637,628,620,612,604,596,588,
//580,572,564,557,549,542,534,527,519,512,
//505,498,491,484,477,470,464,457,450,444,
//438,431,425,419,413,407,401,395,389,383,
//378,372,367,361,356,351,345,340,335,330,
//325,320,316,311,306,302,297,293,288,284,
//280,275,271,267,263,259,255,251,248,244,
//240,237,233,230,226,223,219,216,213,210,
//207,204,201,198,195,192,189,186,184,181,
//178,176,173,171,168,166,164,161,159,157,
//154,152,150,148,146,144,142,140,138,136,
//134,133,131,129,127,126,124,122,121,119,
//118,116,115,113,112,110,109,108,106,105,
//104,102,101,100,99,98,96,95,94,93,
//92,91,90,89,88,87,86,85,84,83,
//83,82,81,80,79,78,78,77,76,75,
//75,74,73,72,72,71,71,70,69,69,
//68,67,67,66,66,65,65,64,64,63,
//63,62,62,61,61,60,60,59,59,59,
//58,58,57,57,57,56,56,56,55,55,
//55,54,54,54,53,53,53,52,52,52,
//52,51,51,51,51,50,50,50,50,49,
//49,49,49,49,48,48,48,48,48,47,
//};

#define ACC_STEP_NUM 400
const unsigned short AccStep[ACC_STEP_NUM] = {
4630,4630,4587,4587,4587,4587,4545,4545,4545,4545,
4505,4505,4505,4505,4464,4464,4464,4425,4425,4425,
4386,4386,4386,4348,4348,4348,4310,4310,4274,4274,
4274,4237,4237,4202,4202,4167,4167,4132,4132,4098,
4098,4065,4065,4032,4032,4000,3968,3968,3937,3906,
3906,3876,3846,3846,3817,3788,3759,3731,3731,3704,
3676,3650,3623,3597,3571,3546,3521,3497,3472,3448,
3425,3401,3378,3356,3311,3289,3268,3247,3205,3185,
3165,3125,3106,3067,3049,3012,2994,2959,2924,2907,
2874,2841,2809,2793,2762,2732,2703,2674,2646,2618,
2577,2551,2525,2500,2463,2439,2415,2381,2358,2326,
2294,2273,2242,2212,2183,2155,2128,2101,2075,2049,
2024,2000,1969,1946,1923,1894,1866,1845,1818,1792,
1767,1748,1724,1695,1672,1650,1629,1608,1582,1562,
1538,1515,1497,1475,1453,1433,1412,1393,1374,1355,
1333,1316,1299,1279,1259,1244,1225,1208,1190,1174,
1157,1142,1126,1111,1096,1080,1066,1050,1037,1022,
1008,996,982,969,956,943,931,919,907,896,
883,873,862,852,840,831,820,810,800,791,
781,773,763,754,746,737,729,722,713,705,
698,691,683,676,669,662,655,649,643,636,
631,624,619,613,608,602,597,591,586,581,
577,571,567,562,558,554,549,545,541,537,
534,530,526,522,519,515,512,509,506,503,
500,497,494,491,488,485,483,480,477,475,
473,470,468,466,463,461,459,457,455,453,
451,449,448,446,444,442,441,439,437,436,
435,433,432,430,429,428,426,425,424,423,
422,421,419,418,417,416,415,414,413,412,
412,411,410,409,408,407,407,406,405,404,
404,403,402,401,401,400,399,399,398,398,
397,397,396,396,395,395,394,394,393,393,
392,392,392,391,391,390,390,389,389,389,
389,388,388,388,387,387,387,386,386,386,
386,385,385,385,385,384,384,384,384,383,
383,383,383,383,383,382,382,382,382,382,
381,381,381,381,381,381,381,381,380,380,
380,380,380,380,380,379,379,379,379,379,
379,379,379,379,379,379,379,378,378,378,
378,378,378,378,378,378,378,378,378,377,
};

/*******************************************************************************
* Function Name  : OPEN_Motor_DIR_CW
* Description    : 步进电机方向
* Input          : None
* Return         : None
*******************************************************************************/
void OPEN_Motor_DIR_CW()							//步进电机方向
{
	GPIO_ResetBits(Port_CS_DIR, Pin_CS_DIR);		//步进电机方向
}
/*******************************************************************************
* Function Name  : OPEN_Motor_DIR_CCW
* Description    : 步进电机方向
* Input          : None
* Return         : None
*******************************************************************************/
void OPEN_Motor_DIR_CCW()							//步进电机方向
{
	GPIO_SetBits(Port_CS_DIR, Pin_CS_DIR);	//步进电机方向
}

void TIM1_PWM_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;   //声明一个结构体变量，用来初始化GPIO

	NVIC_InitTypeDef NVIC_InitStructure;  

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//声明一个结构体变量，用来初始化定时器

	TIM_OCInitTypeDef TIM_OCInitStructure;//根据TIM_OCInitStruct中指定的参数初始化外设TIMx

	/* 开启时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA,ENABLE);
	
		//步进电机驱动器方向
	RCC_APB2PeriphClockCmd(RCC_APB_CS_DIR, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  Pin_CS_DIR;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(Port_CS_DIR, &GPIO_InitStructure);	
	
//	OPEN_Motor_DIR_CCW();							//步进电机方向
	OPEN_Motor_DIR_CW();							//步进电机方向
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin		=	GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	TIM_DeInit(TIM1);
	//TIM4定时器初始化
	TIM_TimeBaseInitStructure.TIM_Period = 900;	   	//设置了在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 35;	//设置了用来作为 TIMx 时钟频率除数的预分频值,72000000/72=1MHz
	TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x0;    //周期计数器值  不懂得不管
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

	TIM_OCStructInit(&TIM_OCInitStructure);
	//PWM初始化	  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OCInitStructure.TIM_OCMode		=	TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState	=	TIM_OutputState_Enable;//PWM输出使能
//	TIM_OCInitStructure.TIM_OutputState =	TIM_OutputState_Enable;  //正向通道有效
//	TIM_OCInitStructure.TIM_OutputNState=	TIM_OutputNState_Enable; //反向通道也有效
	TIM_OCInitStructure.TIM_Pulse = 450;        //占空时间  144 中有40的时间为高，互补的输出正好相反
	TIM_OCInitStructure.TIM_OCPolarity 	= TIM_OCPolarity_High;   //输出极性
//	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //互补端的极性  
//	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  //空闲状态下的非工作状态 不管
//	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;  //先不管
	TIM_OC4Init(TIM1,&TIM_OCInitStructure);		//注意此处初始化时TIM_OC4Init而不是TIM_OCInit，否则会出错。因为固件库的版本不一样。
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);//使能或者失能TIMx在CCR2上的预装载寄存器
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	TIM_Cmd(TIM1,DISABLE);//使能或者失能TIMx外设
	/* TIM1 Main Output Enable 使能TIM1外设的主输出*/
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	TIM_ClearFlag(TIM1,TIM_IT_Update);
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	TIM_ClearITPendingBit(TIM1,TIM_IT_CC4);			//清除TIMx的中断待处理位:TIM 中断源
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;     		//更新事件
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;     		//响应优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       		//允许中断
	NVIC_Init(&NVIC_InitStructure); 

}

void SET_Pulse(unsigned short Value)	//设置定时器自动重装载值和比较值
{
	Motor.Current = Value;				//保存设置的值

	TIM_SetCounter(TIM1,0);
	TIM_SetAutoreload(TIM1,Value);
	TIM_SetCompare4(TIM1,Value>>1);
}

void TIM1_UP_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM1,TIM_FLAG_Update);
//		TIM_ClearITPendingBit(TIM1,TIM_IT_CC1);					//清除TIMx的中断待处理位:TIM 中断源
//		TIM_ClearITPendingBit(TIM1,TIM_FLAG_Update);
		if(ENABLE == Motor.EN)
		{
			if(Motor.Current != Motor.Set)			
			{
				if(Motor.StepNum < ACC_STEP_NUM-1)						//S型曲线加速
				{
					if(Motor.ReTimes++ > Repeat_S)							//重复次数
					{
						Motor.ReTimes = 0;
						if((AccStep[Motor.StepNum] >= Motor.Set)&&(AccStep[Motor.StepNum+1] <= Motor.Set))
						{
							SET_Pulse(Motor.Set);	//设置定时器自动重装载值和比较值
						}else
						{
							if(AccStep[Motor.StepNum] < Motor.Set)
							{
								Motor.StepNum--;
							}else
							{
								SET_Pulse(AccStep[Motor.StepNum++]);	//设置定时器自动重装载值和比较值
							}
						}
					}	
				}else
				{
					if(Motor.ReTimes++ > Repeat_A)					//重复次数
					{
						Motor.ReTimes = 0;
						if(Motor.Current > Motor.Set)					//匀加速
						{
							Motor.Current--;
						}else
						{
							Motor.Current++;
						}					
						SET_Pulse(Motor.Current);	//设置定时器自动重装载值和比较值
					}
				}				
			}
		}else
		{
			if(Motor.Current < AccStep[ACC_STEP_NUM - 1])		//匀减速
			{
				if(Motor.ReTimes++ > Repeat_A)				//重复次数
				{
					Motor.ReTimes = 0;
					Motor.Current++;
					SET_Pulse(Motor.Current);	//设置定时器自动重装载值和比较值
				}	
			}else
			{
				if(Motor.StepNum > Stop_Step)
				{
					if(Motor.Current > AccStep[Motor.StepNum])
					{
						Motor.StepNum -= Stop_Step;
					}else
					{
						if(Motor.ReTimes++ > Repeat_S)
						{
							Motor.ReTimes = 0;
							Motor.StepNum -= Stop_Step;
							if(Motor.StepNum < ACC_STEP_NUM)
							{
								SET_Pulse(AccStep[Motor.StepNum]);	//设置定时器自动重装载值和比较值
							}else
							{
							}
						}					
					}
				}else
				{
//					DISABLE_Motor_EN();						//关闭步进电机使能	
					TIM_CCxCmd(TIM1,TIM_Channel_4,TIM_CCx_Disable);
					TIM_Cmd(TIM1,DISABLE);				//使能或者失能TIMx外设	
					MOTOR_CTL_DISABLE();					

					if(YES == Motor.Switch)				//标记切换方向
					{
						Motor.Switch = NO;					//标记切换方向
						if(DIR_CW == Motor.DIR)			//方向标志
						{
							Motor.DIR = DIR_CCW;			//方向标志
							OPEN_Motor_DIR_CCW();			//步进电机方向
							
						}else
						{
							Motor.DIR = DIR_CW;				//方向标志
							OPEN_Motor_DIR_CW();			//步进电机方向

						}
						Motor.StepNum = 0;
						Motor.EN = ENABLE;					//使能标志
//						ENABLE_Motor_EN();					//打开步进电机使能
						MOTOR_CTL_ENABLE();
						TIM_CCxCmd(TIM1,TIM_Channel_4,TIM_CCx_Enable);
						TIM_Cmd(TIM1,ENABLE);			//使能或者失能TIMx外设				
					}
				}			
			}
		}
	}
	if(TIM_GetFlagStatus(TIM1, TIM_FLAG_CC4) != RESET)//接收到数据
	{		
		TIM_ClearITPendingBit(TIM1,TIM_IT_CC4);			//清除TIMx的中断待处理位:TIM 中断源
	}		
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void motor_backward(void)
{
	#if USE_STEP_MOTOR      	//步进电机	
    MOTOR_CTL_ENABLE();
    delay_ms(500);
	
		if(DISABLE == Motor.EN)						//使能标志
		{
			if(DIR_CCW == Motor.DIR)		//方向标志
			{
				Motor.EN = ENABLE;			//使能标志
				Motor.Switch = NO;			//标记切换方向
			}else
			{
				Motor.EN = DISABLE;			//使能标志
				Motor.Switch = YES;			//标记切换方向
			}
		}else
		{
			if(DIR_CCW != Motor.DIR)		//方向标志
			{
				Motor.EN = DISABLE;			//使能标志
				Motor.Switch = YES;			//标记切换方向
			}
		}			
		TIM_CCxCmd(TIM1,TIM_Channel_4,TIM_CCx_Enable);		//传送带
		TIM_Cmd(TIM1,ENABLE);				//使能或者失能TIMx外设
	#elif USE_BRUSHLESS_MOTOR    	//无刷电机
		/*ccw*/
    if (get_motor_sta() != MOTOR_ACT_BACKWARD) {
        motor_stop();
        delay_ms(200);
    }
    MOTOR_CTL_ENABLE();
    delay_ms(20);
    MOTOR_TS_START();
    MOTOR_BACKWARD_EX();
    set_motor_sta(MOTOR_ACT_BACKWARD);
    dbg_printf("motor backward"CRLF);
    print_motor_level();
    /*motor_ac();*/
    /*MOTOR_PWM_START();*/	
	#endif	

}

void motor_foward(void)
{
	#if USE_STEP_MOTOR      	//步进电机
    MOTOR_CTL_ENABLE();
    delay_ms(500);

		if(DISABLE == Motor.EN)						//使能标志
			{
				if(DIR_CW == Motor.DIR)		//方向标志
				{
					Motor.EN = ENABLE;			//使能标志
					Motor.Switch = NO;			//标记切换方向
				}else
				{
					Motor.EN = DISABLE;			//使能标志
					Motor.Switch = YES;			//标记切换方向
				}
			}else
			{
				if(DIR_CW != Motor.DIR)		//方向标志
				{
					Motor.EN = DISABLE;			//使能标志
					Motor.Switch = YES;			//标记切换方向
				}
			}			
			TIM_CCxCmd(TIM1,TIM_Channel_4,TIM_CCx_Enable);		//传送带
			TIM_Cmd(TIM1,ENABLE);				//使能或者失能TIMx外设
	#elif USE_BRUSHLESS_MOTOR    	//无刷电机
    /*clockwise*/
    if (get_motor_sta() != MOTOR_ACT_FOWARD) {
        motor_stop();
        delay_ms(200);
    }
    MOTOR_CTL_ENABLE();
    delay_ms(20);

    MOTOR_TS_START();
    MOTOR_FORWARD_EX();
    set_motor_sta(MOTOR_ACT_FOWARD);
    dbg_printf("motor foward"CRLF);
    print_motor_level();
    /*motor_ad();*/
    /*MOTOR_PWM_START();*/	
	#endif		
		
}

void motor_stop(void)
{
	#if USE_STEP_MOTOR      	//步进电机	
		Motor.EN = DISABLE;			//使能标志
	#elif USE_BRUSHLESS_MOTOR    	//无刷电机
	/*MOTOR_PWM_STOP();*/
    dbg_printf("pwm:%d\n", PWM);
    MOTOR_CTL_DISABLE();
    MOTOR_TS_STOP();
    MOTOR_ZF_STOP();
    set_motor_sta(MOTOR_ACT_STOP);
    dbg_printf("motor stop"CRLF);
    print_motor_level();
    PWM = 0;	
	#endif
}

void motor_ab(void)
{
	#if USE_STEP_MOTOR      	//步进电机		
	#elif USE_BRUSHLESS_MOTOR    	//无刷电机
    MOTOR_CTL_ENABLE();
    TIM_SetCompare4(TIM1,7000);
    PWM = 0;
    TIM_SetCompare4(TIM1,PWM);		
	#endif
}

void motor_ac(void)
{
	#if USE_STEP_MOTOR      	//步进电机		
	#elif USE_BRUSHLESS_MOTOR    	//无刷电机
    MOTOR_CTL_ENABLE();
    TIM_SetCompare4(TIM1,7000);
    PWM=2800;
    TIM_SetCompare4(TIM1,PWM);		
	#endif
}

void motor_ad(void)
{
	#if USE_STEP_MOTOR      	//步进电机		
	#elif USE_BRUSHLESS_MOTOR    	//无刷电机
    MOTOR_CTL_ENABLE();
    TIM_SetCompare4(TIM1,7000);
    PWM=5000;
    TIM_SetCompare4(TIM1,PWM);		
	#endif
}

__attribute__((unused))
static void tim1_pwm_init(int arr, int psc)
{
    GPIO_InitTypeDef     GPIO_InitStrue;
    TIM_OCInitTypeDef     TIM_OCInitStrue;
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseInitStrue;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);

    GPIO_InitStrue.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStrue.GPIO_Mode = GPIO_Mode_AF_PP;    // 复用推挽
    GPIO_InitStrue.GPIO_Speed=GPIO_Speed_50MHz;    //设置最大输出速度
    GPIO_Init(GPIOA,&GPIO_InitStrue);                //GPIO端口初始化设置
    //    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1,ENABLE);// 部分重映射

    TIM_TimeBaseInitStrue.TIM_Period=arr;    //设置自动重装载值
    TIM_TimeBaseInitStrue.TIM_Prescaler=psc;        //预分频系数
    TIM_TimeBaseInitStrue.TIM_CounterMode=TIM_CounterMode_Up;    //计数器向上溢出
    TIM_TimeBaseInitStrue.TIM_ClockDivision=TIM_CKD_DIV1;        //时钟的分频因子，起到了一点点的延时作用，一般设为TIM_CKD_DIV1
    TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStrue);        //TIM3初始化设置(设置PWM的周期)

    TIM_OCInitStrue.TIM_OCMode=TIM_OCMode_PWM1;        // PWM模式2:CNT>CCR时输出有效
    TIM_OCInitStrue.TIM_OCPolarity=TIM_OCPolarity_High;// 设置极性-有效为高电平
    TIM_OCInitStrue.TIM_OutputState=TIM_OutputState_Enable;// 输出使能
    TIM_OC4Init(TIM1,&TIM_OCInitStrue);        //TIM1的通道4PWM 模式设置

    TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);        //使能预装载寄存器

    TIM_Cmd(TIM1,ENABLE);        //使能TIM1
    MOTOR_PWM_START();
    //全部映射，将TIM3_CH2映射到PB5
    //根据STM32中文参考手册2010中第第119页可知：
    //当没有重映射时，TIM3的四个通道CH1，CH2，CH3，CH4分别对应PA6，PA7,PB0,PB1
    //当部分重映射时，TIM3的四个通道CH1，CH2，CH3，CH4分别对应PB4，PB5,PB0,PB1
    //当完全重映射时，TIM3的四个通道CH1，CH2，CH3，CH4分别对应PC6，PC7,PC8,PC9
    //TIM_OCInitStrue.TIM_OCMode=TIM_OCMode_PWMx;        // PWM模式x:CNT>CCR时输出有效
}

void motor_init(void)
{
/*
    GPIO_InitTypeDef  def;
    def.GPIO_Pin = _P(MOTOR_DIR_PIN_) | _P(MOTOR_SPD_PIN_);
    def.GPIO_Mode = GPIO_Mode_Out_PP;
    def.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &def);
*/
    motor_stop();
		#if USE_STEP_MOTOR      	//步进电机
			TIM1_PWM_Configuration();
			Motor.DIR = DIR_CW;				//方向标志
			Motor.Current = Speed_Min;		//速度当前值,定时器自动重装载值和比较值
			Motor.Set = 900;
		#elif USE_BRUSHLESS_MOTOR    	//无刷电机
		#endif
		
    /*tim1_pwm_init(10000, 0);*/
    /*TIM_SetCompare4(TIM1, 10000);*/
}

