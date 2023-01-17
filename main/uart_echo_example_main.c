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
#define BUF_SIZE (256)

static void update_time(char[] data){
    const char temp[BUF_SIZE] = "t:";
    if (strncmp(data, temp, 2)==0){
        uart_write_bytes(ECHO_UART_PORT_NUM, "time updated", 12);
    }
}

static void query_time(){
    const char query[BUF_SIZE] = "query";
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

    char data[BUF_SIZE] = "";
    
    while (1) {
        //request time
        query_time();

        //read data
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, TIMEOUT);

        //update time
        update_time(data)

        //delay
        
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
    }
}

void app_main(void)
{
        xTaskCreate(time_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
