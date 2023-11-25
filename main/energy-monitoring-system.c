#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "storage.h"
#include "time.h"
#include "web.h"
#include "wifi.h"


void app_main(void)
{
    printf("Set-up WiFi: ");
    setupWifi();
    printf("done\n");
    printf("Set-up time: ");
    setupTime();
    printf("done\n");
    printf("Set-up storage: ");
    printf("done\n");
    setupStorage();

   
    while(1){
        vTaskDelay(pdMS_TO_TICKS(3000)); // 3 seconds delay
        printf("handle client\n");
        serveClient();   
    }
}