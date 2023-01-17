#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "uart";

#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (16)
#define ECHO_TEST_RTS (-1)
#define ECHO_TEST_CTS (-1)

#define ECHO_UART_PORT_NUM      (2)
#define ECHO_UART_BAUD_RATE     (115200)
#define ECHO_TASK_STACK_SIZE    (2048)
#define TIMEOUT (20 / portTICK_RATE_MS)
#define xDelay (1000 / portTICK_PERIOD_MS)
#define BUF_SIZE (256)

char uart_recv_data[BUF_SIZE] = "";
struct time
{
    int hh;
    int mm;
    int ss;
} myTime;

static void update_time()
{
    const char temp[BUF_SIZE] = "t:";
    if (strncmp(uart_recv_data, temp, 2)==0){
        //parse into myTime
        myTime.hh = 10;
        myTime.mm = 20;
        myTime.ss = 30;

        uart_write_bytes(ECHO_UART_PORT_NUM, "time updated", 12);
    }
    uart_recv_data[0] = '\0';
}

static void query_time()
{
    static const char query[5] = "query";
    uart_write_bytes(ECHO_UART_PORT_NUM, query, 5);
}

static void time_task(void *arg)
{
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

    uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags);
    uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
    uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    
    while (1) {
        //request time
        query_time();

        //read data
        uart_read_bytes(ECHO_UART_PORT_NUM, uart_recv_data, BUF_SIZE, TIMEOUT);

        //update time
        update_time(uart_recv_data);
        ESP_LOGI(TAG, "%d:%d:%d", myTime.hh, myTime.mm, myTime.ss);

        //delay
        vTaskDelay(xDelay);
    }
}

void app_main(void)
{
        xTaskCreate(time_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
