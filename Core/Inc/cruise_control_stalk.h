/*
 * cruise_control_stalk.h
 *
 *  Created on: Jan 19, 2026
 *      Author: creany
 */

#ifndef INC_CRUISE_CONTROL_STALK_H_
#define INC_CRUISE_CONTROL_STALK_H_

#include "cmsis_os2.h"
#include "stm32g4xx_hal.h"

/* Button event structure */
typedef enum {
    BTN_CC_EN,
    BTN_CC_RES,
    BTN_CC_SET,
} ButtonID_t;

typedef enum {
    BTN_EVENT_PRESS,
    BTN_EVENT_RELEASE,
    BTN_EVENT_CLICK,
    BTN_EVENT_DOUBLE_CLICK,
} ButtonEvent_t;

typedef struct {
    ButtonID_t button;
    ButtonEvent_t event;
    uint32_t timestamp;
} ButtonMessage_t;

/* Timing constants */
#define DEBOUNCE_TIME_MS        50
#define DOUBLE_CLICK_TIME_MS    300

/* Function prototypes */
void CCStalkInit(void);
void CCStalkProcessButtons(void);

#endif /* INC_CRUISE_CONTROL_STALK_H_ */
