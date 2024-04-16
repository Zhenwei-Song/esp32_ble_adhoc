/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2024-01-16 15:05:32
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-22 20:02:32
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\ble_timer.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2024 by Zhenwei Song, All Rights Reserved.
 */
#include "ble_timer.h"
#include "macro_def.h"

esp_timer_handle_t ble_time1_timer;
esp_timer_handle_t ble_time2_timer;
esp_timer_handle_t ble_time3_timer;
esp_timer_handle_t ble_time4_timer;
esp_timer_handle_t ble_time5_timer;

bool timer1_timeout = false;
bool timer2_timeout = false;

bool timer1_running = false;
bool timer2_running = false;
bool timer3_running = false;

bool entry_network_flag = false;

SemaphoreHandle_t xCountingSemaphore_timeout1;
SemaphoreHandle_t xCountingSemaphore_timeout2;

/**
 * @description: 定时器初始化函数
 * @return {*}
 */
void ble_timer_init(void)
{
    const esp_timer_create_args_t time1_timer_args = {
        .callback = &time1_timer_cb,
        .name = "timer1"}; // 定时器名字
    ESP_ERROR_CHECK(esp_timer_create(&time1_timer_args, &ble_time1_timer));

    const esp_timer_create_args_t time2_timer_args = {
        .callback = &time2_timer_cb,
        .name = "timer2"}; // 定时器名字
    ESP_ERROR_CHECK(esp_timer_create(&time2_timer_args, &ble_time2_timer));

    const esp_timer_create_args_t time3_timer_args = {
        .callback = &time3_timer_cb,
        .name = "timer3"}; // 定时器名字
    ESP_ERROR_CHECK(esp_timer_create(&time3_timer_args, &ble_time3_timer));

    const esp_timer_create_args_t time4_timer_args = {
        .callback = &time4_timer_cb,
        .name = "timer4"}; // 定时器名字
    ESP_ERROR_CHECK(esp_timer_create(&time4_timer_args, &ble_time4_timer));

    const esp_timer_create_args_t time5_timer_args = {
        .callback = &time5_timer_cb,
        .name = "timer5"}; // 定时器名字
    ESP_ERROR_CHECK(esp_timer_create(&time5_timer_args, &ble_time5_timer));
}

/**
 * @description: timer1超时函数，用于time1
 * @return {*}
 */
void time1_timer_cb(void)
{
    timer1_timeout = true;
    timer1_running = false;
    xSemaphoreGive(xCountingSemaphore_timeout1);
#ifdef PRINT_TIMER_STATES
    printf("time1_timeout\n");
#endif
}

/**
 * @description: timer2超时函数，用于time2
 * @return {*}
 */
void time2_timer_cb(void)
{
    timer2_timeout = true;
    timer2_running = false;
    xSemaphoreGive(xCountingSemaphore_timeout2);
#ifdef PRINT_TIMER_STATES
    printf("time2_timeout\n");
#endif
}

/**
 * @description: timer3超时函数，用于time3
 * @return {*}
 */
void time3_timer_cb(void)
{
    timer3_running = false;
#ifdef PRINT_TIMER_STATES
    printf("time3_timeout\n");
#endif
}

/**
 * @description: timer4超时函数，用于入网后定时进行路由发现
 * @return {*}
 */
void time4_timer_cb(void)
{
    entry_network_flag = true;
#ifdef PRINT_TIMER_STATES
    printf("time4_timeout\n");
#endif
}

/**
 * @description: timer5超时函数，用于定时发送传感数据
 * @return {*}
 */
void time5_timer_cb(void)
{
#ifdef PRINT_TIMER_STATES
    printf("time4_timeout\n");
#endif
}