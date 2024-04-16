/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2023-09-22 17:13:32
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-02-29 10:05:51
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\ble.c
 * @Description:
 * 实现了广播与扫描同时进行（基于gap层）
 * 添加了GPIO的测试内容（由宏定义控制是否启动）
 * 实现了基于adtype中name的判断接收包的方法
 * 实现了字符串的拼接（帧头 + 实际内容）
 * 添加了吞吐量测试
 * 添加了rssi
 * 添加了发送和接收消息队列，用单独的task进行处理
 * 添加了邻居表，添加了邻居表的刷新机制
 * 添加了路由表，添加了路由表的刷新机制（并未使用定时刷新删除）
 * 使用信号来控制发送和数据处理，以及定时器超时动作
 * 添加了定时器的使用
 * 完整的实现了路由发现阶段，并已验证 TODO:包计数号
 * 路由维护阶段实现了RRER TODO:路由表的维护
 * 入网后定时进行入网发现，以获取最好的入网路径（未启用）
 * 添加了基于ID的包过滤机制，方便用于测试
 * 更新了链路质量的计算方法
 * 明细了项目测试时使用到的打印信息
 * Copyright (c) 2024 by Zhenwei Song, All Rights Reserved.
 */

#include "ble.h"
#include "ble_uart.h"
#include "macro_def.h"

#ifdef GPIO
#include "driver/gpio.h"
#endif // GPIO
#ifdef QUEUE
#include "ble_queue.h"
#endif // QUEUE
#ifdef DOWN_ROUTINGTABLE
#include "down_routing_table.h"
#include "esp_mac.h"
#include "neighbor_table.h"
#include "up_routing_table.h"
#endif // DOWN_ROUTINGTABLE
#ifdef BUTTON
#include "board.h"
#endif

#include "ble_timer.h"
#include "data_manage.h"
// 181 B5
// 10  0A

// 235 EB
// 54  36

// 202 CA
// 230 E6

// 119 77
// 74 4A

#ifdef FILTER
// #define FILTERED_ID_H 119
// #define FILTERED_ID_L 75
// static uint8_t filtered_id[ID_LEN] = {FILTERED_ID_H, FILTERED_ID_L};

#endif // FILTER

#ifdef GPIO
#define GPIO_OUTPUT_IO_0 18
#define GPIO_OUTPUT_IO_1 19
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))
#endif // GPIO

#ifdef THROUGHPUT
#define SECOND_TO_USECOND 1000000
static bool start = false;
static uint64_t start_time = 0;
static uint64_t current_time = 0;
static uint64_t cal_len = 0;
#endif // THROUGHPUT

queue rec_queue;
queue send_queue;

/**
 * @description: 定时发送hello包
 * @param {void} *pvParameters
 * @return {*}
 */
static void hello_task(void *pvParameters)
{
    while (1) {
        memcpy(adv_data_final_for_hello, data_match(adv_data_name_7, generate_phello(&my_information), HEAD_DATA_LEN, PHELLO_FINAL_DATA_LEN), FINAL_DATA_LEN);
        queue_push(&send_queue, adv_data_final_for_hello, 0);
        xSemaphoreGive(xCountingSemaphore_send);
        vTaskDelay(pdMS_TO_TICKS(HELLO_TIME));
    }
}

static void message_task(void *pvParameters)
{
#if 0
    uint8_t temp_message[MESSAGE_FINAL_DATA_LEN] = {0};
    memcpy(temp_message, generate_message(adv_data_message_16, &my_information, NULL), MESSAGE_FINAL_DATA_LEN);
    int i = 0;
    while (1) {
        temp_message[23] = i;
        memcpy(adv_data_final_for_message, data_match(adv_data_name_7, temp_message, HEAD_DATA_LEN, MESSAGE_FINAL_DATA_LEN), FINAL_DATA_LEN);
        queue_push(&send_queue, adv_data_final_for_message, 0);
        xSemaphoreGive(xCountingSemaphore_send);
        i++;
        if (i == 15)
            i = 0;
        vTaskDelay(pdMS_TO_TICKS(MESSAGE_TIME));
    }
#else
    while (1) {
        if (my_information.is_connected == true) {
            memcpy(adv_data_final_for_message, data_match(adv_data_name_7, generate_message(adv_data_message_16, &my_information, NULL), HEAD_DATA_LEN, MESSAGE_FINAL_DATA_LEN), FINAL_DATA_LEN);
            queue_push(&send_queue, adv_data_final_for_message, 0);
            xSemaphoreGive(xCountingSemaphore_send);
            vTaskDelay(pdMS_TO_TICKS(MESSAGE_TIME));
        }
    }
#endif
}

/**
 * @description: 刷新邻居表，更新链路质量，进行路由发现
 * @param {void} *pvParameters
 * @return {*}
 */
static void ble_down_routing_table_task(void *pvParameters)
{
    while (1) {
        refresh_cnt_neighbor_table(&my_neighbor_table, &my_information);
#ifndef SELF_ROOT
        // update_quality_of_neighbor_table(&my_neighbor_table, &my_information);
        uint8_t neighbor_number = get_neighbor_node_number(&my_neighbor_table);
        if (neighbor_number != 0) {
            if ((timer3_running == false && timer2_running == false && timer1_running == false && timer1_timeout == false && timer2_timeout == false && my_information.is_connected == false) || (timer3_running == false && entry_network_flag == true && my_information.is_connected == true)) { // 路由错误后等待一段时间后才进行路由发现
                entry_network_flag = false;
                if (threshold_high_flag == true) {
#ifdef PRINT_ENTRY_NETWORK_FLAG_STATES
                    ESP_LOGE(DATA_TAG, "threshold_high_flag");
#endif
#ifndef ONLY_SEND_HELLO
                    // threshold_high_ops(&my_neighbor_table, &my_information);
                    threshold_between_ops(&my_neighbor_table, &my_information);
#endif
                }
                else if (threshold_low_flag == true) {
#ifdef PRINT_ENTRY_NETWORK_FLAG_STATES
                    ESP_LOGE(DATA_TAG, "threshold_between_flag");
#endif
                    if (timer1_running == false) {
                        // ESP_LOGE(DATA_TAG, "threshold_between_ops");
#ifndef ONLY_SEND_HELLO
                        threshold_between_ops(&my_neighbor_table, &my_information);
#endif
                    }
                }
                else {
#ifdef PRINT_ENTRY_NETWORK_FLAG_STATES
                    ESP_LOGE(DATA_TAG, "threshold_low_flag");
#endif
                    if (timer2_running == false && timer1_running == false) {
                        // ESP_LOGE(DATA_TAG, "threshold_low_ops");
#ifndef ONLY_SEND_HELLO
                        threshold_low_ops(&my_neighbor_table, &my_information);
#endif
                    }
                }
            }
        }
        // set_my_next_id_quality_and_distance(&my_neighbor_table, &my_information);
#endif
#ifdef PRINT_NEIGHBOR_TABLE
        print_neighbor_table(&my_neighbor_table);
#endif // PRINT_NEIGHBOR_TABLE
#ifdef PRINT_UP_ROUTING_TABLE
#ifndef SELF_ROOT
        print_up_routing_table(&my_up_routing_table);
#endif
#endif // PRINT_UP_ROUTING_TABLE
#ifdef PRINT_DOWN_ROUTING_TABLE
        print_down_routing_table(&my_down_routing_table);
#endif // PRINT_DOWN_ROUTING_TABLE
#if 1
        if (refresh_flag_for_neighbor == true) { // 状态改变，立即发送hello
            refresh_flag_for_neighbor = false;
            memcpy(adv_data_final_for_hello, data_match(adv_data_name_7, generate_phello(&my_information), HEAD_DATA_LEN, PHELLO_FINAL_DATA_LEN), FINAL_DATA_LEN);
            queue_push(&send_queue, adv_data_final_for_hello, 0);
            xSemaphoreGive(xCountingSemaphore_send);
        }
#endif
#ifdef PRINT_MY_INFO
        ESP_LOGW(DATA_TAG, "****************************Start printing my info:***********************************************");
        ESP_LOGI(DATA_TAG, "my_id:");
        esp_log_buffer_hex(DATA_TAG, my_information.my_id, ID_LEN);
        ESP_LOGI(DATA_TAG, "root_id:");
        esp_log_buffer_hex(DATA_TAG, my_information.root_id, ID_LEN);
        ESP_LOGI(DATA_TAG, "is_root:%d", my_information.is_root);
        ESP_LOGI(DATA_TAG, "is_connected:%d", my_information.is_connected);
        ESP_LOGI(DATA_TAG, "next_id:");
        esp_log_buffer_hex(DATA_TAG, my_information.next_id, ID_LEN);
        ESP_LOGI(DATA_TAG, "distance:%d", my_information.distance);
        ESP_LOGI(DATA_TAG, "quality_from_me_to_cluster——via_best_neighbor:");
        esp_log_buffer_hex(DATA_TAG, my_information.quality_from_me, QUALITY_LEN);
        ESP_LOGI(DATA_TAG, "quality_from_me_to_neighbor:");
        esp_log_buffer_hex(DATA_TAG, my_information.quality_from_me_to_neighbor, QUALITY_LEN);
        ESP_LOGI(DATA_TAG, "update:%d", my_information.update);
        ESP_LOGW(DATA_TAG, "****************************Printing my info is finished *****************************************");
#endif // PRINT_MY_INFO
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DOWN_ROUTING_TABLE_TIME));
    }
}

/**
 * @description: 处理接收数据
 * @param {void} *pvParameters
 * @return {*}
 */
static void ble_rec_data_task(void *pvParameters)
{
    uint8_t *phello = NULL;
    uint8_t *anhsp = NULL;
    uint8_t *hsrrep = NULL;
    uint8_t *anrreq = NULL;
    uint8_t *anrrep = NULL;
    uint8_t *rrer = NULL;
    uint8_t *message = NULL;
    uint8_t *block = NULL;
    uint8_t *rec_data = NULL;
    uint8_t phello_len = 0;
    uint8_t anhsp_len = 0;
    uint8_t hsrrep_len = 0;
    uint8_t anrreq_len = 0;
    uint8_t anrrep_len = 0;
    uint8_t rrer_len = 0;
    uint8_t message_len = 0;
    uint8_t block_len = 0;
    while (1) {
        if (xSemaphoreTake(xCountingSemaphore_receive, portMAX_DELAY) == pdTRUE) // 得到了信号量
        {
            if (!queue_is_empty(&rec_queue)) {
                rec_data = queue_pop(&rec_queue);
                if (rec_data != NULL) {
                    phello = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_PHELLO, &phello_len);
                    anhsp = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_ANHSP, &anhsp_len);
                    hsrrep = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_HSRREP, &hsrrep_len);
                    anrreq = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_ANRREQ, &anrreq_len);
                    anrrep = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_ANRREP, &anrrep_len);
                    rrer = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_RRER, &rrer_len);
                    message = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_MESSAGE, &message_len);
                    block = esp_ble_resolve_adv_data(rec_data, ESP_BLE_AD_TYPE_BLOCK, &block_len);
                    // ESP_LOGI(TAG, "ADV_DATA:");
                    // esp_log_buffer_hex(TAG, rec_data, 31);
                    if (phello != NULL) {
                        resolve_phello(phello, &my_information, temp_rssi);
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "PHELLO_DATA:");
                        esp_log_buffer_hex(TAG, phello, phello_len);
#endif
                        // ESP_LOGE(TAG, "rssi:%d", temp_rssi);
                    }
                    if (anhsp != NULL) {
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "ANHSP_DATA:");
                        esp_log_buffer_hex(TAG, anhsp, anhsp_len);
#endif
                        resolve_anhsp(anhsp, &my_information);
                    }
                    if (hsrrep != NULL) {
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "HSRREP_DATA:");
                        esp_log_buffer_hex(TAG, hsrrep, hsrrep_len);
#endif
                        resolve_hsrrep(hsrrep, &my_information);
                    }
                    if (anrreq != NULL) {
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "ANRREQ_DATA:");
                        esp_log_buffer_hex(TAG, anrreq, anrreq_len);
#endif
                        resolve_anrreq(anrreq, &my_information);
                    }
                    if (anrrep != NULL) {
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "ANRREP_DATA:");
                        esp_log_buffer_hex(TAG, anrrep, anrrep_len);
#endif
                        resolve_anrrep(anrrep, &my_information, temp_rssi);
                    }
                    if (rrer != NULL) {
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "RRER_DATA:");
                        esp_log_buffer_hex(TAG, rrer, rrer_len);
#endif
                        resolve_rrer(rrer, &my_information);
                    }
                    if (message != NULL) {
#ifdef PRINT_MESSAGE_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "MESSAGE_DATA:");
                        esp_log_buffer_hex(TAG, message, message_len);
#endif
                        resolve_message(message, &my_information);
                    }
                    if (block != NULL) {
#ifdef PRINT_CONTROL_PACKAGES_RECEIVED
                        ESP_LOGI(TAG, "BLOCK_DATA:");
                        esp_log_buffer_hex(TAG, block, block_len);
#endif
                        resolve_block_message(block, &my_information);
                    }
                    free(rec_data);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(REC_TIME));
        }
    }
}

/**
 * @description: 发送数据
 * @param {void} *pvParameters
 * @return {*}
 */
static void ble_send_data_task(void *pvParameters)
{
    uint8_t *send_data = NULL;
    while (1) {
#if 0
        if (!queue_is_empty(&send_queue)) {
            send_data = queue_pop(&send_queue);
            if (send_data != NULL) {
                esp_ble_gap_config_adv_data_raw(send_data, 31);
                esp_ble_gap_start_advertising(&adv_params);
                free(send_data);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ADV_TIME));
#else
        if (xSemaphoreTake(xCountingSemaphore_send, portMAX_DELAY) == pdTRUE) // 得到了信号量
        {
            if (!queue_is_empty(&send_queue)) {
                send_data = queue_pop(&send_queue);
                if (send_data != NULL) {
                    esp_ble_gap_config_adv_data_raw(send_data, 31);
                    esp_ble_gap_start_advertising(&adv_params);
                    free(send_data);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ADV_TIME));
#endif
    }
}

/**
 * @description: 监听timer1超时
 * @param {void} *pvParameters
 * @return {*}
 */
static void ble_timer1_check_task(void *pvParameters)
{
    while (1) {
        if (xSemaphoreTake(xCountingSemaphore_timeout1, portMAX_DELAY) == pdTRUE) // 得到了信号量
        {
            memcpy(adv_data_final_for_anrreq, data_match(adv_data_name_7, generate_anrreq(&my_information), HEAD_DATA_LEN, ANRREQ_FINAL_DATA_LEN), FINAL_DATA_LEN);
            queue_push(&send_queue, adv_data_final_for_anrreq, 0);
            xSemaphoreGive(xCountingSemaphore_send);
            // 开始计时
            esp_timer_start_once(ble_time2_timer, TIME2_TIMER_PERIOD);
            timer2_running = true;
#ifdef PRINT_TIMER_STATES
            printf("timer1 timeout ,timer2 started\n");
#endif
            timer1_timeout = false;
        }
    }
}

/**
 * @description: 监听timer2超时
 * @param {void} *pvParameters
 * @return {*}
 */
static void ble_timer2_check_task(void *pvParameters)
{
    while (1) {
        if (xSemaphoreTake(xCountingSemaphore_timeout2, portMAX_DELAY) == pdTRUE) // 得到了信号量
        {
            timer2_timeout = false; // 一切重置，回到最初，重新在三个阈值中选择
#ifdef PRINT_TIMER_STATES
            printf("timer2 timeout ,restart\n");
#endif
        }
    }
}

#ifdef THROUGHPUT
/**
 * @description: 计算2秒内吞吐量
 * @param {void} *param
 * @return {*}
 */
static void throughput_task(void *param)
{
    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        uint32_t bit_rate = 0;
        if (start_time) {
            current_time = esp_timer_get_time();
            bit_rate = cal_len * SECOND_TO_USECOND / (current_time - start_time);
            ESP_LOGI(TAG, " Bit rate = %" PRIu32 " Byte/s, = %" PRIu32 " bit/s, time = %ds",
                     bit_rate, bit_rate << 3, (int)((current_time - start_time) / SECOND_TO_USECOND));
        }
        else {
            ESP_LOGI(TAG, " Bit rate = 0 Byte/s, = 0 bit/s");
        }
    }
}
#endif // THROUGHPUT

#if 0
static void ble_scan_task(void *pvParameters)
{
    while (1) {
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {

            xSemaphoreGive(xMutex); // 释放互斥信号量
            vTaskDelay(pdMS_TO_TICKS(ADV_INTERVAL_MS));
        }
        //        vTaskDelay(pdMS_TO_TICKS(ADV_INTERVAL_MS)); // 等待一段时间后再次扫描
        // ESP_LOGI(TAG, "scanning");
    }
}
#endif

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    bool is_filtered = false;
#ifdef THROUGHPUT
    uint32_t bit_rate = 0;
#endif // THROUGHPUT
#ifdef GPIO
    static bool flag = false;
#endif // GPIO
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: /*!< When raw advertising data set complete, the event comes */
        // ESP_LOGW(TAG, "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT");
        // esp_ble_gap_stop_scanning();
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        /* advertising start complete event to indicate advertising start successfully or failed */
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGE(TAG, "Advertising start failed");
#endif
        }
        else {
            // esp_ble_gap_start_scanning(duration);
            // ESP_LOGI(TAG, "Advertising start successfully");
#ifdef GPIO
            gpio_set_level(GPIO_OUTPUT_IO_0, 0);
            esp_ble_gap_stop_advertising();
#endif
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGE(TAG, "Advertising stop failed");
#endif
        }
        else {
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGI(TAG, "Stop adv successfully");
#endif
#ifdef GPIO
            gpio_set_level(GPIO_OUTPUT_IO_0, 1);
            esp_ble_gap_start_advertising(&adv_params);
#endif
        }
        break;

    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGE(TAG, "Scan start failed");
#endif
        }
        else {
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGI(TAG, "Scan start successfully");
#endif
        }
        break;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGE(TAG, "Scan stop failed");
#endif
        }
        else {
            // esp_ble_gap_start_advertising(&adv_params);
#ifdef PRINT_GAP_EVENT_STATES
            ESP_LOGI(TAG, "Stop scan successfully\n");
#endif
        }
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT: { // 表示已经扫描到BLE设备的广播数据
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) { // 用于检查扫描事件类型
        case ESP_GAP_SEARCH_INQ_RES_EVT:            // 表示扫描到一个设备的广播数据
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len); // 解析广播设备名
            if (adv_name != NULL) {
                if (strlen(remote_device_name) == adv_name_len && strncmp((char *)adv_name, remote_device_name, adv_name_len) == 0) { // 检查扫描到的设备名称是否与指定的 remote_device_name 匹配
#ifdef THROUGHPUT
#if 0
                    current_time = esp_timer_get_time();
                    bit_rate = 31 * SECOND_TO_USECOND / (current_time - start_time);
                    ESP_LOGI(TAG, " Bit rate = %" PRIu32 " Byte/s, = %" PRIu32 " bit/s, time = %ds",
                             bit_rate, bit_rate << 3, (int)((current_time - start_time) / SECOND_TO_USECOND));

                    start_time = esp_timer_get_time();
#else
                    cal_len += 31;
                    if (start == false) {
                        start_time = esp_timer_get_time();
                        start = true;
                    }
#endif
#endif // THROUGHPUT

#ifndef THROUGHPUT
                    // ESP_LOGW(TAG, "%s is found", remote_device_name);
#ifdef FILTER
                    if (memcmp(my_information.my_id, id_b50a, 2) == 0) {
                        if (memcmp(id_eb36, scan_result->scan_rst.bda + 4, 2) == 0 ||
                            memcmp(id_774a, scan_result->scan_rst.bda + 4, 2) == 0) {
                            is_filtered = true;
                        }
                    }
                    else if (memcmp(my_information.my_id, id_cae6, 2) == 0) {
                        if (memcmp(id_774a, scan_result->scan_rst.bda + 4, 2) == 0 ||
                            memcmp(id_0936, scan_result->scan_rst.bda + 4, 2) == 0) {
                            is_filtered = true;
                        }
                    }
                    else if (memcmp(my_information.my_id, id_eb36, 2) == 0) {
                        if (memcmp(id_b50a, scan_result->scan_rst.bda + 4, 2) == 0) {
                            is_filtered = true;
                        }
                    }
                    else if (memcmp(my_information.my_id, id_0936, 2) == 0) {
                        if (memcmp(id_cae6, scan_result->scan_rst.bda + 4, 2) == 0) {
                            is_filtered = true;
                        }
                    }
                    else if (memcmp(my_information.my_id, id_774a, 2) == 0) {
                        if (memcmp(id_b50a, scan_result->scan_rst.bda + 4, 2) == 0 ||
                            memcmp(id_cae6, scan_result->scan_rst.bda + 4, 2) == 0) {
                            is_filtered = true;
                        }
                    }
                    if (is_filtered) {
                        is_filtered = false;
                    }
                    else {
                        queue_push_with_check(&rec_queue, scan_result->scan_rst.ble_adv, scan_result->scan_rst.rssi);
                        xSemaphoreGive(xCountingSemaphore_receive);
                    }
#else
                    queue_push_with_check(&rec_queue, scan_result->scan_rst.ble_adv, scan_result->scan_rst.rssi);
                    xSemaphoreGive(xCountingSemaphore_receive);
#endif // FILTER
       //  queue_print(&rec_queue);
#endif // ndef THROUGHPUT

#ifdef GPIO
                    gpio_set_level(GPIO_OUTPUT_IO_1, flag);
                    flag = !flag;
                    printf("flag is %d\n", flag);
#endif // GPIO
                }
            }
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

#ifdef DOWN_ROUTINGTABLE
static void down_routing_table_init(void)
{
    uint8_t my_mac[6];
    init_neighbor_table(&my_neighbor_table);
    esp_read_mac(my_mac, ESP_MAC_BT);
    my_info_init(&my_information, my_mac);

    init_down_routing_table(&my_down_routing_table);
}
#endif // DOWN_ROUTINGTABLE

#ifdef QUEUE
static void all_queue_init(void)
{
    queue_init(&rec_queue);
    queue_init(&send_queue);
}
#endif // QUEUE

#ifdef BUTTON
#ifndef SELF_ROOT
/**
 * @description: 非root节点按下按键，向root发送message
 * @return {*}
 */
void button_ops()
{
#ifdef BUTTON_MY_MESSAGE
    ESP_LOGE(DATA_TAG, "SENDING MY MESSAGE");
    memcpy(adv_data_final_for_message, data_match(adv_data_name_7, generate_message(adv_data_message_16, &my_information, NULL), HEAD_DATA_LEN, MESSAGE_FINAL_DATA_LEN), FINAL_DATA_LEN);
    queue_push(&send_queue, adv_data_final_for_message, 0);
    xSemaphoreGive(xCountingSemaphore_send);
#endif
#ifdef BUTTON_BLOCK_MESSAGE
    // ESP_LOGE(DATA_TAG, "SENDING BLOCK MESSAGE");
    memcpy(adv_data_final_for_block_message, data_match(adv_data_name_7, generate_block_message(&my_information), HEAD_DATA_LEN, BLOCK_MESSAGE_FINAL_DATA_LEN), FINAL_DATA_LEN);
    queue_push(&send_queue, adv_data_final_for_block_message, 0);
    xSemaphoreGive(xCountingSemaphore_send);
#endif
}
#else
void button_ops()
{
}
#endif
#endif // BUTTON

#ifdef GPIO
static void esp_gpio_init(void)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
}
#endif // GPIO

void app_main(void)
{
    esp_err_t ret;
#if 0
    // 初始化调度锁
    xMutex1 = xSemaphoreCreateMutex();
    if (xMutex1 == NULL) {
        ESP_LOGE(TAG, "semaphore create error");
        return;
    }
#endif
#if 0
    xMutex2 = xSemaphoreCreateMutex();
    if (xMutex2 == NULL) {
        ESP_LOGE(TAG, "semaphore create error");
        return;
    }
#endif
    // srand((unsigned int)esp_random());

    // 初始化NV存储
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化BLE控制器和蓝牙堆栈
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "esp_bt_controller_init failed: %s", esp_err_to_name(ret));
        return;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "esp_bt_controller_enable failed: %s", esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "esp_bluedroid_init failed: %s", esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "esp_bluedroid_enable failed: %s", esp_err_to_name(ret));
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "gap register error, error code = %x", ret);
        return;
    }
#ifdef GPIO
    esp_gpio_init();
#endif // GPIO
#ifdef DOWN_ROUTINGTABLE
    down_routing_table_init();
#endif // DOWN_ROUTINGTABLE
#ifdef QUEUE
    all_queue_init();
#endif // QUEUE

    xCountingSemaphore_send = xSemaphoreCreateCounting(200, 0);
    xCountingSemaphore_receive = xSemaphoreCreateCounting(200, 0);
    xCountingSemaphore_timeout1 = xSemaphoreCreateCounting(200, 0);
    xCountingSemaphore_timeout2 = xSemaphoreCreateCounting(200, 0);

    esp_ble_gap_set_scan_params(&ble_scan_params);
    esp_ble_gap_start_scanning(duration);
    // esp_ble_gap_start_advertising(&adv_params);

    xTaskCreate(hello_task, "hello_task", 1024, NULL, 2, NULL);
#ifndef SELF_ROOT
#ifdef SENGDING_MESSAGE_PERIODIC
    xTaskCreate(message_task, "message_task", 1024, NULL, 2, NULL);
#endif
#endif
    xTaskCreate(ble_down_routing_table_task, "ble_down_routing_table_task", 4096, NULL, 5, NULL);
    xTaskCreate(ble_send_data_task, "ble_send_data_task", 2048, NULL, 3, NULL);
    xTaskCreate(ble_rec_data_task, "ble_rec_data_task", 4096, NULL, 4, NULL);
#ifdef BLE_TIMER
    xTaskCreate(ble_timer1_check_task, "ble_timer1_check_task", 1024, NULL, 4, NULL);
    xTaskCreate(ble_timer2_check_task, "ble_timer2_check_task", 1024, NULL, 4, NULL);
    ble_timer_init();
#ifndef SELF_ROOT
    // esp_timer_start_periodic(ble_time4_timer, TIME3_TIMER_PERIOD);
#endif
#endif
#ifdef PRINT_MESSAGE_FOR_OPENWRT
    ble_uart_init();
#endif
#ifdef BUTTON
    board_init();
#endif // BUTTION
#ifdef THROUGHPUT
    xTaskCreate(throughput_task, "throughput_task", 4096, NULL, 5, NULL);
#endif // THROUGHPUT
}
