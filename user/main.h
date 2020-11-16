
#include "motor.h"

#ifndef MAIN_WYFDIK6N
#define MAIN_WYFDIK6N
#ifdef __cplusplus
extern "C" { /*}*/
#endif
#define USE_BOOT_LOADER     1
#define UART1_DISP_ENABLE   1
#define USE_CHECK_BAT       1
#define USE_CHK             0
#define USE_NEW_PROT        0
#define COM_SERIAL_PORT     2
#define DEV_CYCLES          (HZ * 1)


#define DEBUG               0

#define STR(__v)            #__v
#define _CON(__v)           STR(__v)
#define CRLF                "\r\n"


#define _VERSION_PREFIX     "xlbc-"
#if USE_STEP_MOTOR      	//步进电机	
	#define V                 2	
	#if USE_NEW_PROT
	#define P                   2
	#define S                   5
	#else
	#define P                   0
	#define S                   3
	#endif
#elif USE_BRUSHLESS_MOTOR    	//无刷电机
	#define V             	    1
	#if USE_NEW_PROT
	#define P                   2
	#define S                   5
	#else
	#define P                   1
	#define S                   5
	#endif
#endif



#define _VERSION            _CON(V)
#define _PATCHLEVEL         _CON(P)
#define _SUBLEVEL           _CON(S)



#define BAUD                115200
#if DEBUG
#define VERSION             _VERSION_PREFIX""_VERSION"."_PATCHLEVEL"."_SUBLEVEL"D"
#else
#define VERSION             _VERSION_PREFIX""_VERSION"."_PATCHLEVEL"."_SUBLEVEL
#endif


#define ABS(_V)             (((_V) > 0)? (_V): -(_V))

/*#define uint8_t             unsigned char
#define uint16_t            unsigned int
#define int16_t             signed int*/


#ifdef __cplusplus
}
#endif
#endif /* end of include guard: MAIN_WYFDIK6N */



