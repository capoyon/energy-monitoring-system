#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"
#include <esp_http_server.h>
#include "esp_spiffs.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include <lwip/netdb.h>

// Web files
#include "html/overview.h"
#include "html/history.h"
#include "html/automate.h"
#include "html/settings.h"

#define LED_PIN 2
int led_state = 0;

httpd_handle_t server = NULL;
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

static const char *TAG = "WebSocket"; // TAG for debug

/* Handler variables */
// for webpages
esp_err_t overview_page_handler(httpd_req_t *req)
{
    int response;
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    response = httpd_resp_send(req, (const char*) overview, overview_len);
    return response;
}

esp_err_t history_page_handler(httpd_req_t *req)
{
    int response;
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    response = httpd_resp_send(req, (const char*) history, history_len);
    return response;
}

esp_err_t automate_page_handler(httpd_req_t *req)
{
    int response;
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    response = httpd_resp_send(req, (const char*) automate, automate_len);
    return response;
}

esp_err_t settings_page_handler(httpd_req_t *req)
{
    int response;
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    response = httpd_resp_send(req, (const char*) settings, settings_len);
    return response;
}

// Function to send WebSocket frame
static void send_ws_frame(const char *payload, size_t len, httpd_ws_type_t type) {
    httpd_ws_frame_t ws_pkt;

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = payload;
    ws_pkt.len = len;
    ws_pkt.type = type;

    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    esp_err_t ret = httpd_get_client_list(server, &fds, client_fds);

    if (ret != ESP_OK) {
        return;
    }

    for (int i = 0; i < fds; i++) {
        int client_info = httpd_ws_get_fd_info(server, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
            httpd_ws_send_frame_async(server, client_fds[i], &ws_pkt);
        }
    }
}

// toggle the led pin and send it's status to websocket
static void toggle_led(void *arg)
{
    struct async_resp_arg *resp_arg = arg;


    led_state = !led_state;
    gpio_set_level(LED_PIN, led_state);

    char buff[4];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "%d", led_state);

    // Send WebSocket frame
    send_ws_frame(buff, strlen(buff), HTTPD_WS_TYPE_TEXT);

    free(resp_arg);
}

static void send_led_state()
{
    char buff[4];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "%d", led_state);
    send_ws_frame(buff, strlen(buff), HTTPD_WS_TYPE_TEXT);
}

void send_strsocks(const char* httpdata, size_t httpdata_size)
{
    send_ws_frame(httpdata, httpdata_size, HTTPD_WS_TYPE_TEXT);
}


// handler for socket connection
static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    return httpd_queue_work(handle, toggle_led, resp_arg);
}

// hander for websocket
static esp_err_t handle_ws_req(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, new connection opened");
        send_led_state();
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "Got socket, len = %d", ws_pkt.len);
    if (ws_pkt.len)
    {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "payload: %s", ws_pkt.payload);
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt.payload, "toggle") == 0)
    {
        free(buf);
        return trigger_async_send(req->handle, req);
    }
    return ESP_OK;
}


/* For handling all http request */
httpd_handle_t setup_websocket_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = overview_page_handler,
        .user_ctx = NULL};

    httpd_uri_t ws = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = handle_ws_req,
        .user_ctx = NULL,
        .is_websocket = true};

    httpd_uri_t uri_settings = {
        .uri = "/settings",
        .method = HTTP_GET,
        .handler = settings_page_handler,
        .user_ctx = NULL
    };

    httpd_uri_t uri_overview = {
        .uri = "/overview",
        .method = HTTP_GET,
        .handler = overview_page_handler,
        .user_ctx = NULL
    };

    httpd_uri_t uri_history = {
        .uri = "/history",
        .method = HTTP_GET,
        .handler = history_page_handler,
        .user_ctx = NULL
    };

    httpd_uri_t uri_automate = {
        .uri = "/automate",
        .method = HTTP_GET,
        .handler = automate_page_handler,
        .user_ctx = NULL
    };


    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_settings);
        httpd_register_uri_handler(server, &uri_overview);
        httpd_register_uri_handler(server, &uri_history);
        httpd_register_uri_handler(server, &uri_automate);
        httpd_register_uri_handler(server, &ws);
    }
    return server;
}

// Entry point function for the web socket
void start_web(void)
{
    //initialize the pin
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    led_state = 0;

    ESP_LOGI(TAG, "ESP32 ESP-IDF WebSocket Web Server is running ... ...\n");
    setup_websocket_server();
}