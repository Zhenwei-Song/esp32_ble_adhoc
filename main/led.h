/*
 * @Author: Zhenwei Song zhenwei_song@foxmail.com
 * @Date: 2025-02-27 10:44:55
 * @LastEditors: Zhenwei Song zhenwei_song@foxmail.com
 * @LastEditTime: 2025-02-27 11:08:07
 * @FilePath: \esp32_ble_positioning\main\led.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2025 by Zhenwei Song, All Rights Reserved.
 */
#ifndef _MY_LED_H_
#define _MY_LED_H_

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include <stdio.h>

#define CONFIG_BLINK_GPIO 48
#define BLINK_GPIO CONFIG_BLINK_GPIO

extern led_strip_handle_t led_strip;
extern SemaphoreHandle_t xCountingSemaphore_led;

extern bool s_led_state;

void configure_led(void);

void led_red(void);
void led_green(void);
void led_blue(void);
void led_off(void);

#endif