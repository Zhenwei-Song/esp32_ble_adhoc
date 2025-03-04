/*
 * @Author: Zhenwei Song zhenwei_song@foxmail.com
 * @Date: 2025-02-27 10:44:48
 * @LastEditors: Zhenwei Song zhenwei_song@foxmail.com
 * @LastEditTime: 2025-02-27 11:28:59
 * @FilePath: \esp32_ble_positioning\main\led.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2025 by Zhenwei Song, All Rights Reserved.
 */
#include "led.h"

#define LED_TAG "LED"
bool s_led_state = false;

led_strip_handle_t led_strip;

SemaphoreHandle_t xCountingSemaphore_led;

void configure_led(void)
{
#ifdef SOC_ESP32S3_SUPPORTED
    ESP_LOGI(LED_TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
#else
    // GPIO初始化
    gpio_reset_pin(BLINK_GPIO); // #define  2
    // 设置GPIO口的模式为输出模式
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT); // 配置模式
#endif
}

void led_red(void)
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, 255, 0, 0);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
}

void led_green(void)
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, 0, 50, 0);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
}

void led_blue(void)
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, 0, 0, 255);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
}

void led_off(void)
{
    led_strip_clear(led_strip);
}