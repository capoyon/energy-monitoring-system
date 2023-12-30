#include "data.h"
#include "web.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define ADC_CHANNEL ADC1_CHANNEL_0
#define SENSOR_PIN 34


char* uint32_to_string(uint32_t value)
{
    // Determine the number of digits in the uint16_t value
    int num_digits = snprintf(NULL, 0, "%lu", value);

    // Allocate memory for the string representation
    char* result = (char*)malloc(num_digits + 1);  // +1 for null terminator

    if (result != NULL) {
        // Convert uint16_t to string
        snprintf(result, num_digits + 1, "%lu", value);
    }

    return result;
}


cJSON* create_sensor_json(int sensor_data[])
{
    const uint8_t cmd = 2;
    cJSON* root = cJSON_CreateObject();
    if (root != NULL) {
        cJSON_AddNumberToObject(root, "cmd", cmd);
        cJSON *arr = cJSON_CreateIntArray((const int *)sensor_data, sizeof(sensor_data) / sizeof(sensor_data[0]));
        cJSON_AddItemToObject(root, "data", arr);
    }
    else {
        printf("Error: create_sensor_json()");
    }
    return root;
}

void adc_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0);
}

// read sensor data, then send it to websocket
void read_sensors(void)
{
    cJSON* sensor_data_json;
    char* sensor_data_string;
    int adc_value[1];
    while (1) {
        adc_value[0] = adc1_get_raw(ADC_CHANNEL);
        printf("Sensor Value: %d\n", adc_value[0]);

        sensor_data_json = create_sensor_json(adc_value);
        sensor_data_string = cJSON_PrintUnformatted(sensor_data_json);
        printf("%s\n", sensor_data_string);

        send_strsocks(sensor_data_string, strlen(sensor_data_string));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}