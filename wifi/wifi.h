#ifndef WIFI_H
#define WIFI_H
#include "esp_netif.h"


void wifi_init(void);

esp_netif_t *wifi_init_softap(void);

esp_netif_t *wifi_init_sta(void);

void setupWifi(void);


#endif // WIFI_H