#include "button.h"

void initButtons(){
    //setup queue
    buttonQueue = xQueueCreate(1, sizeof(BUTTON));

    gpio_install_isr_service(0);

    //left button setup
    gpio_set_direction(GPIO_NUM_26,GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_NUM_26,GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_NUM_26, buttonISRHandler, (void *) LEFTBUTTON);
    gpio_set_pull_mode(GPIO_NUM_26,GPIO_PULLUP_ONLY);

    //right button setup
    gpio_set_direction(GPIO_NUM_25,GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_NUM_25,GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_NUM_25, buttonISRHandler, (void *) RIGHTBUTTON);
    gpio_set_pull_mode(GPIO_NUM_25,GPIO_PULLUP_ONLY);
}

static void IRAM_ATTR buttonISRHandler(void* arg)
{
    BUTTON pressedButton = (BUTTON) arg;
    //ets_printf("BUTTON %d", pressedButton);
    xQueueSendFromISR(buttonQueue, &pressedButton, NULL);
}

int getButton(BUTTON *selectedButton)
{
    //timeout after 30 seconds
    return xQueueReceive(buttonQueue, selectedButton, 30000 / portTICK_PERIOD_MS);
}

void clearStoredButtons(){
    xQueueReset(buttonQueue);
}