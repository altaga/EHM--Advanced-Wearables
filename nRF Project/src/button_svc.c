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

// Tri Wire Port Definitions, You can use any GPIO port
#define BITT 7
#define COM 8
#define ENA 9

const struct device *dev;

// Tri Wire Prototypes (good practice only).
unsigned int readTriWire(void);
void beginTriWire(void);

// Other Definitions and Variables

LOG_MODULE_REGISTER(button_svc);

#define BUT_PORT    DT_GPIO_LABEL(DT_ALIAS(sw0), gpios)
#define BUT_PIN     DT_GPIO_PIN(DT_ALIAS(sw0), gpios)

extern struct bt_conn *conn;
extern struct bt_gatt_service_static stsensor_svc[];
extern volatile bool notify_enable;

uint32_t spo_val;

// Main Read Sensors Function

void sensor_loop(void)
{
// Global memory variable
unsigned int memory = 0;
// Infinite Loop
	while (1)
    {
        static unsigned int temp = 0;
        // Reading TriWire Data
        temp = readTriWire();
        if (temp >> 8 != 0) // Check if temp is not empty
        {   
            /*
                TriWire protocol explanation.
                https://github.com/altaga/EHM--Advanced-Wearables#read-sensor-data
            */
           // Reding 3 Sensors Data Enconding.
            if (temp >> 8 == 1)
            {
                memory = temp & 0x00FF;             // SPO2 Data
            }
            else if (temp >> 8 == 2)
            {
                memory = memory<<8;
                memory = memory + (temp & 0x00FF);  // BPM Data
            }
            else if (temp >> 8 == 3)
            {
                memory = memory<<8;
                memory = memory + (temp & 0x00FF);  // Temperature Data
            }
            else if (temp >> 8 == 4)
            {
                memory = memory<<8;
                memory = memory + (temp & 0x00FF);  // EMPTY, only dummy data to fill 32 bits but you can add any extra sensor
                spo_val = memory;
                // Sending Data from sensors to nRF Cloud via BLE.
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
    // Setup GPIO to INPUT MODE
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
    // Reading Enable if yes y can start read values
    while (gpio_pin_get(dev, ENA) == 1)
    {
        bool flag = true;
        // Read while flag is true
        while (gpio_pin_get(dev, COM) == 1)
        {
            // If bit is ready to read COM is 1
            if (flag)
            {
                // Reading bit and Concatenate bit in recv
                recv += (((int)gpio_pin_get(dev, BITT)) * pointer);
                pointer *= 2;
                flag = false;
            }
        }
        // Correct Read 
        printFlag = true;
    }
    if (printFlag)
    {
        // Return Sensor number and value encoding in recv (16 bits)
        return (recv);
    }
    else
    {
        // Incorrect Read Return 0
        return (0);
    }
}
