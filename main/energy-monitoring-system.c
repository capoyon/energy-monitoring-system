#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "data.h"
#include "time.h"
#include "web.h"
#include "wifi.h"


void app_main(void)
{
    setupWifi();
    start_web();
    adc_init();
    read_sensors();
}

