/*
 * cruise_control_stalk.c
 *
 *  Created on: Jan 19, 2026
 *      Author: creany
 */

#include "cruise_control_stalk.h"
#include "main.h"

/* Button state tracking */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState active_state;

    // State tracking
    uint8_t debounced_state;
    uint8_t last_stable_state;
    uint32_t last_change_time;
    uint32_t press_start_time;
    uint8_t click_count;
    uint32_t last_click_time;
} ButtonState_t;

/* Define your buttons - UPDATE THESE based on your actual GPIO pins */
static ButtonState_t buttons[] = {
    {BTN_CC_EN_GPIO_Port, BTN_CC_EN_Pin, GPIO_PIN_SET, 0, 0, 0, 0, 0, 0},
    {BTN_CC_RES_GPIO_Port, BTN_CC_RES_Pin, GPIO_PIN_SET, 0, 0, 0, 0, 0, 0},
    {BTN_CC_SET_GPIO_Port, BTN_CC_SET_Pin, GPIO_PIN_SET, 0, 0, 0, 0, 0, 0},
};

#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))

extern osMessageQueueId_t ccStalkButtonEventQueueHandle;

void CCStalkInit(void)
{
    // Initialize button states if needed
    // Currently all static initialization, but could add runtime init here
}

void CCStalkProcessButtons(void)
{
    static uint32_t current_time = 0;
    current_time = osKernelGetTickCount();

    ButtonMessage_t msg;

    // Process each button
    for(int i = 0; i < NUM_BUTTONS; i++)
    {
        ButtonState_t *btn = &buttons[i];

        // Read current pin state
        GPIO_PinState current_reading = HAL_GPIO_ReadPin(btn->port, btn->pin);
        uint8_t is_pressed = (current_reading == btn->active_state);

        // Debouncing logic
        if(is_pressed != btn->debounced_state) {
            btn->last_change_time = current_time;
            btn->debounced_state = is_pressed;
        }

        // Check if button state has stabilized
        if((current_time - btn->last_change_time) >= DEBOUNCE_TIME_MS)
        {
            // State has changed and is stable
            if(btn->debounced_state != btn->last_stable_state)
            {
                btn->last_stable_state = btn->debounced_state;

                if(btn->debounced_state) // Button pressed
                {
                    btn->press_start_time = current_time;

                    // Send press event
                    msg.button = (ButtonID_t)i;
                    msg.event = BTN_EVENT_PRESS;
                    msg.timestamp = current_time;
                    osMessageQueuePut(ccStalkButtonEventQueueHandle, &msg, 0, 0);
                }
                else // Button released
                {
                    // Send release event
                    msg.button = (ButtonID_t)i;
                    msg.event = BTN_EVENT_RELEASE;
                    msg.timestamp = current_time;
                    osMessageQueuePut(ccStalkButtonEventQueueHandle, &msg, 0, 0);

                    // Handle click/double-click detection
                    if((current_time - btn->last_click_time) <= DOUBLE_CLICK_TIME_MS) {
                        btn->click_count++;

                        if(btn->click_count == 2) {
                            // Double click detected
                            msg.button = (ButtonID_t)i;
                            msg.event = BTN_EVENT_DOUBLE_CLICK;
                            msg.timestamp = current_time;
                            osMessageQueuePut(ccStalkButtonEventQueueHandle, &msg, 0, 0);
                            btn->click_count = 0;
                        }
                    }
                    else {
                        btn->click_count = 1;
                    }

                    btn->last_click_time = current_time;
                }
            }
        }
    }

    // Handle delayed single-click detection
    for(int i = 0; i < NUM_BUTTONS; i++)
    {
        ButtonState_t *btn = &buttons[i];

        if(btn->click_count == 1 &&
           (current_time - btn->last_click_time) > DOUBLE_CLICK_TIME_MS)
        {
            // Single click confirmed
            msg.button = (ButtonID_t)i;
            msg.event = BTN_EVENT_CLICK;
            msg.timestamp = btn->last_click_time;
            osMessageQueuePut(ccStalkButtonEventQueueHandle, &msg, 0, 0);
            btn->click_count = 0;
        }
    }
}
