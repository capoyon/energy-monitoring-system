#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "storage.h"
#include "time.h"
#include "web.h"
#include "wifi.h"


void app_main(void)
{
    setupWifi();
    start_web();
}

