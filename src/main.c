/*
 * Copyright (c) 2018 Alexander Wachter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/can.h>
#include <drivers/gpio.h>
#include <sys/byteorder.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0	DT_GPIO_FLAGS(LED0_NODE, gpios)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN0	0
#define FLAGS0	0
#define LED1	""
#define PIN1	0
#define FLAGS1	0
#endif

int tog = 0;
int tog1 = 0;

struct k_work work_queue;
struct zcan_work rx_work;

const struct device *can_dev;
void activity0(const struct device *dev, const struct zcan_frame *msg, k_timeout_t timeout, can_tx_callback_t callback_isr, void *callback_arg);

void rx_callback_function(struct zcan_frame *frame, void *arg){
    //printk("I made it: %d", frame->data[0]);
    const struct device *ourLED;
    ourLED = device_get_binding(LED0);
    gpio_pin_configure(ourLED, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS0);

    if(tog == 0)
        tog = 1;
    else
        tog = 0;
    
    gpio_pin_set(ourLED, PIN0, tog);
}

void activity3_callback_tx(struct zcan_frame *frame, void *arg){
    const struct device *ourLED1;
    ourLED1 = device_get_binding(LED1);
    gpio_pin_configure(ourLED1, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1);

    if(tog1 == 0)
        tog1 = 1;
    else
        tog1 = 0;

    gpio_pin_set(ourLED1, PIN1, tog1);
}

int main(){
    struct zcan_frame frame = {
        .id_type = CAN_STANDARD_IDENTIFIER,
        .rtr = CAN_DATAFRAME,
        .id = 0x123,
        .dlc =8,
        .data={1,2,3,4,5,6,7,8}
    };
    struct zcan_frame frame2 = {
        .id_type = CAN_STANDARD_IDENTIFIER,
        .rtr = CAN_DATAFRAME,
        .id = 0x156,
        .dlc =8,
        .data={8,7,6,5,5,6,7,8}
    };
    /**
    const struct zcan_filter my_filter = {
        .id_type = CAN_STANDARD_IDENTIFIER,
        .rtr = CAN_DATAFRAME,
        .id = 0x123,
        .rtr_mask =1,
        .id_mask = 0
    };
    */
   const struct zcan_filter activity1_filter = {
        .id_type = CAN_STANDARD_IDENTIFIER,
        .rtr = CAN_DATAFRAME,
        .id = 0x156,
        .rtr_mask =1,
        .id_mask = CAN_STD_ID_MASK
    };

    int filter_id;
   
    can_dev =device_get_binding("CAN_1");
    //Comment out loop back line for activity 2
    can_set_mode(can_dev, CAN_NORMAL_MODE);
    //can_set_mode(can_dev, CAN_LOOPBACK_MODE);
    
    //activity 0
    //filter_id = can_attach_isr(can_dev, rx_callback_function, NULL, &my_filter);
    //filter_id = can_attach_isr(can_dev, rx_callback_function, NULL, &activity1_filter);


    //activity3
    filter_id = can_attach_workq(can_dev, &work_queue, &rx_work, rx_callback_function, NULL, &activity1_filter);

    if (filter_id <0){
        printk("unable to attach isr [%d]", filter_id);
    }
   
    while(1){
        //activity 0
        //k_sleep(K_MSEC(5000));
        //activity0(can_dev, &frame, K_MSEC(100),NULL,NULL);
        
        //activity 1
        k_sleep(K_MSEC(1000));
        activity0(can_dev, &frame, K_MSEC(100), NULL, NULL);
        k_sleep(K_MSEC(1000));
        activity0(can_dev, &frame2, K_MSEC(100), NULL, NULL);
    }    
 }

 void activity0(const struct device *dev, const struct zcan_frame *msg, k_timeout_t timeout, can_tx_callback_t callback_isr, void *callback_arg){
    int ret;
    ret = can_send(dev, msg, K_MSEC(100), activity3_callback_tx, NULL);
    if (ret != CAN_TX_OK){
        printk("Sending failed [%d]", ret);
    }
}