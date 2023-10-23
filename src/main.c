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

#define RX_THREAD_STACK_SIZE 512
#define RX_THREAD_PRIORITY 2
#define STATE_POLL_THREAD_STACK_SIZE 512
#define STATE_POLL_THREAD_PRIORITY 2
#define LED_MSG_ID 0x10
#define COUNTER_MSG_ID 0x12345
#define SET_LED 1
#define RESET_LED 0
#define SLEEP_TIME K_MSEC(250)

struct zcan_frame frame = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .dlc =8,
    .data={1,2,3,4,5,6,7,8}
};
const struct zcan_filter my_filter = {
    .id_type = CAN_STANDARD_IDENTIFIER,
    .rtr = CAN_DATAFRAME,
    .id = 0x123,
    .rtr_mask =1,
    .id_mask = CAN_STD_ID_MASK

};

const struct device *can_dev;
void activity0(const struct device *dev, const struct zcan_frame *msg, k_timeout_t timeout, can_tx_callback_t callback_isr, void *callback_arg){
    int ret;
    ret = can_send(dev, &frame, K_MSEC(100), NULL,NULL);
    if (ret != CAN_TX_OK){
    printk("Sending failed [%d]", ret);
}

void rx_callback_function(struct zcan_frame *frame, void *arg){
    
}

int main(){
    
    int filter_id;
   
    can_dev =device_get_binding("CAN_1");
    can_set_mode(can_dev, CAN_LOOPBACK_MODE);
    filter_id = can_attach_isr(can_dev, rx_callback_function, callback_arg, &my_filter);
   
    if (filter_id <0){
        printk("unable to attach isr [%d]", filter_id);
    }
   
    while(1){
        k_sleep(K_MSEC(1000));
        activity0(can_dev, &frame, K_MSEC(100),NULL,NULL);
    }    
 }