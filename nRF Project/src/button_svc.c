/** @file
 *  @brief Button Service sample
 */

/*
 * Copyright (c) 2019 Marcio Montenegro <mtuxpe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <drivers/gpio.h>
#include <logging/log.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "button_svc.h"

#define BITT 7
#define COM 8
#define ENA 9

const struct device *dev;

unsigned int readTriWire(void);
void beginTriWire(void);

LOG_MODULE_REGISTER(button_svc);

#define BUT_PORT    DT_GPIO_LABEL(DT_ALIAS(sw0), gpios)
#define BUT_PIN     DT_GPIO_PIN(DT_ALIAS(sw0), gpios)

extern struct bt_conn *conn;
extern struct bt_gatt_service_static stsensor_svc[];
extern volatile bool notify_enable;

uint32_t spo_val;

void sensor_loop(void)
{
unsigned int memory = 0;
	while (1)
    {
        static unsigned int temp = 0;
        temp = readTriWire();
        if (temp >> 8 != 0)
        {
            if (temp >> 8 == 1)
            {
                memory = temp & 0x00FF;
            }
            else if (temp >> 8 == 2)
            {
                memory = memory<<8;
                memory = memory + (temp & 0x00FF);
            }
            else if (temp >> 8 == 3)
            {
                memory = memory<<8;
                memory = memory + (temp & 0x00FF);
            }
            else if (temp >> 8 == 4)
            {
                memory = memory<<8;
                memory = memory + (temp & 0x00FF);
                spo_val = memory;
                bt_gatt_notify(NULL, &stsensor_svc->attrs[2], &spo_val, sizeof(spo_val));
            }
            printk("%d \n",(temp & 0x00FF));
        }
    }
}

int sensor_init(void)
{
	beginTriWire();
	return 0;
}

void beginTriWire(void)
{
    dev = device_get_binding("GPIO_1");
    gpio_pin_configure(dev, BITT, GPIO_INPUT);
    gpio_pin_configure(dev, COM, GPIO_INPUT);
    gpio_pin_configure(dev, ENA, GPIO_INPUT);
}

unsigned int readTriWire(void)
{
    unsigned int recv = 0;
    bool printFlag = false;
    int pointer = 1;
    while (gpio_pin_get(dev, ENA) == 1)
    {
        bool flag = true;
        while (gpio_pin_get(dev, COM) == 1)
        {
            if (flag)
            {
                recv += (((int)gpio_pin_get(dev, BITT)) * pointer);
                pointer *= 2;
                flag = false;
            }
        }
        printFlag = true;
    }
    if (printFlag)
    {
        return (recv);
    }
    else
    {
        return (0);
    }
}
