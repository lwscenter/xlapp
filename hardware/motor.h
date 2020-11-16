#ifndef MOTOR_AKP4LD2A
#define MOTOR_AKP4LD2A
#ifdef __cplusplus
extern "C" { /*}*/
#endif

void motor_foward(void);
void motor_backward(void);
void motor_stop(void);
void motor_ab(void);
void motor_ac(void);
void motor_ad(void);

void motor_check_abnormal(void);

void motor_pwm_ticks(void);
void motor_init(void);  

enum {
    MOTOR_ACT_STOP,
    MOTOR_ACT_FOWARD,
    MOTOR_ACT_BACKWARD,
};

#if			 0   
#define USE_STEP_MOTOR      		1 	//0：无刷电机，1：步进电机
#else  
#define USE_BRUSHLESS_MOTOR     1		//无刷电机
#endif

//步进电机方向
#define RCC_APB_CS_DIR	 			RCC_APB2Periph_GPIOA
#define Port_CS_DIR						GPIOA
#define Pin_CS_DIR	 					(GPIO_Pin_12)

#define MOTOR_CTL_IN_PIN               PAin(MOTOR_CTL_PIN_)
#define MOTOR_TS_IN_PIN                PAin(MOTOR_TS_PIN_)
#define MOTOR_ZF_IN_PIN                PAin(MOTOR_ZF_PIN_)

#define MOTOR_CTL_DISABLE()            (MOTOR_CTL_PIN = HIGH)
#define MOTOR_CTL_ENABLE()             (MOTOR_CTL_PIN = LOW)
#define MOTOR_CTL_TURN()               (MOTOR_CTL_PIN = !MOTOR_CTL_PIN)

#define MOTOR_FORWARD_EX()             (MOTOR_ZF_PIN = HIGH)
#define MOTOR_BACKWARD_EX()            (MOTOR_ZF_PIN = LOW)

#define MOTOR_TS_START()               (MOTOR_TS_PIN = LOW)
#define MOTOR_TS_STOP()                (MOTOR_TS_PIN = HIGH)

#define MOTOR_ZF_STOP()                (MOTOR_ZF_PIN = HIGH)

/*#define MOTOR_SPD_STOP()               MOTOR_SPD_PIN = LOW
#define MOTOR_DIR_STOP()               (MOTOR_DIR_PIN = LOW)

#define MOTOR_SPD_START()              (MOTOR_SPD_PIN = HIGH)
#define MOTOR_DIR_START()              (MOTOR_DIR_PIN = HIGH)*/

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: MOTOR_AKP4LD2A */
