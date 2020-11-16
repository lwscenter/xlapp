#ifndef KEY_ZRVB25Z0
#define KEY_ZRVB25Z0
#ifdef __cplusplus
extern "C" { /*}*/
#endif

#define KEY_CALLBACK_ENABLE     1

//According to your need to modify the constants.
#define TICKS_INTERVAL          10	//ms
#define DEBOUNCE_TICKS          3	//MAX 8
#define SHORT_TICKS             (300 /TICKS_INTERVAL)
#define LONG_TICKS              (3000 /TICKS_INTERVAL)
#define CON_TICKS               (3000 /TICKS_INTERVAL)
#define CALLBACK_MAX            (PRESS_CONTINUE_DOWN + 1)


#if KEY_CALLBACK_ENABLE
typedef void (*key_callback)(void *);
#endif

typedef enum {
    PRESS_DOWN = 0,
    PRESS_UP,
    PRESS_LONG_DOWN,
    PRESS_CONTINUE_DOWN,
    PRESS_REPEAT,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    PRESS_MAX,
} PressEvent;

#pragma pack(1)
struct key_t {
#if LONG_TICKS < 256
    uint8_t ticks;
#else
    uint16_t ticks;
#endif
    uint8_t event;
    uint8_t repeat;
    uint8_t mark;
    uint8_t state : 6;
    uint8_t active_level : 1;
    uint8_t  (*hal_get_level)(void);
#if KEY_CALLBACK_ENABLE
    key_callback  cb[CALLBACK_MAX];/*PRESS_DOWN, PRESS_UP, LONG_PRESS_HOLD*/
#endif
    struct key_t *next;
};
#pragma pack()

void key_ticks_run(void);

#define get_key_event(_v)            (_v->event)
#define get_key_mark(_v)             (_v->mark)

#if KEY_CALLBACK_ENABLE
void key_attach(struct key_t *handle, PressEvent event, key_callback cb);
#endif

void key_init(struct key_t *handle, uint8_t mark, uint8_t(*pin_level)(void), uint8_t active_level);

void key_start(struct key_t *handle);


#ifdef __cplusplus
}
#endif
#endif /* end of include guard: KEY_ZRVB25Z0 */
