#include "main.h"
#include "sys.h"
#include "bsp.h"
#include "key.h"

enum SCAN_KEY_STATE {
    KEY_STATE_START = 0, /*start */
    KEY_STATE_JITTERS1,  /* eliminate jitters  */
    KEY_STATE_JITTERS2,
    KEY_STATE_JITTERS3,
    KEY_STATE_JITTERS4,
    KEY_STATE_DOWN,    /* valid down */
    KEY_STATE_LONGDOWN, /*long press*/
    KEY_STATE_CONT_DOWN, /*press is continue ?*/
    KEY_STATE_UP, /*up start continue*/
};

#if KEY_CALLBACK_ENABLE
#define EVENT_CB(ev) if(handle->cb[ev])handle->cb[ev]((struct key_t *)handle)
#else
#define EVENT_CB(...)   do{ } while(0)
#endif

//key handle list head.
static struct key_t * head_handle;

/**
 * @brief  Initializes the key struct handle.
 * @param  handle: the key handle strcut.
 * @param  pin_level: read the HAL GPIO of the connet key level.
 * @param  active_level: pressed GPIO level.
 * @retval None
 */
void key_init(struct key_t* handle, uint8_t mark, uint8_t(*pin_level)(void), uint8_t active_level)
{
    /*memset(handle, 0, sizeof(struct key_t));*/
    handle->event = (uint8_t)PRESS_MAX;
    handle->mark = mark;
    handle->hal_get_level = pin_level;
    /*handle->cur_level = handle->hal_get_level();*/
    handle->active_level = active_level;
    handle->state = KEY_STATE_START;
}
#if KEY_CALLBACK_ENABLE
/**
  * @brief  Attach the key event callback function.
  * @param  handle: the key handle strcut.
  * @param  event: trigger event type.
  * @param  cb: callback function.
  * @retval None
  */
void key_attach(struct key_t *handle, PressEvent event, key_callback cb)
{
    if (event > CALLBACK_MAX)
        return;
    handle->cb[event] = cb;
}

#endif
/**
 * @brief  key driver core function, driver state machine.
 * @param  handle: the key handle strcut.
 * @retval None
 */
void key_handler(struct key_t  *handle)
{
    uint8_t level = handle->hal_get_level();

    //ticks counter working..
    /*if(handle->state !=  KEY_STATE_START) handle->ticks++;*/
    /*-----------------State machine-------------------*/
    switch (handle->state) {
    case KEY_STATE_START:
        if(level != handle->active_level) {	//start press down
            handle->state = KEY_STATE_JITTERS1;
        }
        break;
    case KEY_STATE_JITTERS1:
        if(level != handle->active_level)
            handle->state = KEY_STATE_DOWN;
        else
            handle->state = KEY_STATE_START;
        break;
    case KEY_STATE_JITTERS2:
        if(level != handle->active_level)
            handle->state = KEY_STATE_JITTERS3;
        else
            handle->state = KEY_STATE_START;
        break;
    case KEY_STATE_JITTERS3:
        if(level != handle->active_level)
            handle->state = KEY_STATE_JITTERS4;
        else
            handle->state = KEY_STATE_START;
        break;
    case KEY_STATE_JITTERS4:
        if(level != handle->active_level)
            handle->state = KEY_STATE_DOWN;
        else
            handle->state = KEY_STATE_START;
        break;
    case KEY_STATE_DOWN:
        if(level != handle->active_level) {
            handle->ticks = 0;
            handle->event = (uint8_t)PRESS_DOWN;
            EVENT_CB(PRESS_DOWN);
            handle->state = KEY_STATE_LONGDOWN;
        } else {
            handle->state = KEY_STATE_UP;
        }
        break;
    case KEY_STATE_LONGDOWN:
        if(level != handle->active_level) {
            if(++handle->ticks > LONG_TICKS) {
                handle->ticks = 0;
                handle->event = (uint8_t)PRESS_LONG_DOWN;
                EVENT_CB(PRESS_LONG_DOWN);
                handle->state = KEY_STATE_CONT_DOWN;
            }
        } else {
            handle->state = KEY_STATE_UP;
        }
        break;
    case KEY_STATE_CONT_DOWN:
        if(level != handle->active_level) {
            if(++handle->ticks > SHORT_TICKS) {
                handle->ticks = 0;
                handle->event = (uint8_t)PRESS_CONTINUE_DOWN;
                EVENT_CB(PRESS_CONTINUE_DOWN);
                handle->state = KEY_STATE_CONT_DOWN;
            }
        } else {
            handle->state = KEY_STATE_UP;
        }
        break;
    case KEY_STATE_UP:
        if(level == handle->active_level) {
            handle->event = (uint8_t)PRESS_UP;
            EVENT_CB(PRESS_UP);
            handle->state = KEY_STATE_START;
        }
        break;
    }
}

/**
 * @brief  Start the key work, add the handle into work list.
 * @param  handle: target handle strcut.
 */
void key_start(struct key_t *handle)
{
    struct key_t *target = head_handle;
    while(target) {
        if(target == handle) return;	//already exist.
        target = target->next;
    }
    handle->next = head_handle;
    head_handle = handle;
}

/**
 * @brief  Stop the key work, remove the handle off work list.
 * @param  handle: target handle strcut.
 * @retval None
 */
void key_stop(struct key_t *handle)
{
    struct key_t **curr;
    for(curr = &head_handle; *curr; ) {
        struct key_t *entry = *curr;
        if (entry == handle) {
            *curr = entry->next;
            //			free(entry);
        } else
            curr = &entry->next;
    }
}

/**
 * @brief  background ticks, timer repeat invoking interval 5ms.
 * @param  None.
 * @retval None
 */
void key_ticks_run(void)
{
    struct key_t *link;
    for(link = head_handle; link; link = link->next) {
        key_handler(link);
    }
}
