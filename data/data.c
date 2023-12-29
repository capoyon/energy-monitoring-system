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


char* uint32_to_string(uint32_t value) {
    // Determine the number of digits in the uint32_t value
    int num_digits = snprintf(NULL, 0, "%lu", value);

    // Allocate memory for the string representation
    char* result = (char*)malloc(num_digits + 1);  // +1 for null terminator

    if (result != NULL) {
        // Convert uint32_t to string
        snprintf(result, num_digits + 1, "%lu", value);
    }

    return result;
}

cJSON* create_json_object(int cmd, const char* httpdata) {
    cJSON* root = cJSON_CreateObject();
    
    if (root != NULL) {
        cJSON_AddNumberToObject(root, "cmd", cmd);
        cJSON_AddStringToObject(root, "data", httpdata);
    }
    else {
        printf("Error: create_json_object()");
    }
    return root;
}


void adc_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0);
}

void read_sensors(void) {
    cJSON* sensor_data_json;
    char* sensor_data_string;
    while (1) {
        uint32_t adc_value = adc1_get_raw(ADC_CHANNEL);
        printf("Sensor Value: %lu\n", adc_value);
        sensor_data_json = create_json_object(2, uint32_to_string(adc_value));
        sensor_data_string = cJSON_PrintUnformatted(sensor_data_json);
        printf("%s\n", sensor_data_string);
        printf("%d\n", strlen(sensor_data_string));
        send_strsocks(sensor_data_string, strlen(sensor_data_string));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}