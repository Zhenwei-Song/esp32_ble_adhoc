#ifndef BLE_UART_H_
#define BLE_UART_H_

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#define TX_TAG "TX_TASK"

extern int RX_BUF_SIZE;

#define TXD_PIN (GPIO_NUM_2)
#define RXD_PIN (GPIO_NUM_4)

void ble_uart_init(void);

int sendData_tx(const char *logName, const char *data);

// static void tx_task(void *arg)
// {
//     static const char *TX_TASK_TAG = "TX_TASK";   // 发送内容
//     esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO); // 设置log打印等级
//     while (1) {
//         sendData_tx(TX_TASK_TAG, "Hello world");  // 发送数据
//         vTaskDelay(2000 / portTICK_PERIOD_MS); // 延时
//     }
// }

#endif