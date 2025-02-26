/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2023-10-12 11:16:49
 * @LastEditors: Zhenwei-Song zhenwei.song@qq.com
 * @LastEditTime: 2023-10-30 21:17:32
 * @FilePath: \vscode_settinge:\github\esp32\esp_ble_mesh\ble_mesh_node\onoff_client\main\board.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei Song, All Rights Reserved.
 */
/* board.c - Board-specific hooks */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"

#include "board.h"
#include "iot_button.h"

#define TAG "BOARD"

#define BUTTON_IO_NUM 0
#define BUTTON_ACTIVE_LEVEL 0

extern void button_ops(void);

static void button_tap_cb(void *arg)
{
    ESP_LOGI(TAG, "tap cb (%s)", (char *)arg);

    button_ops();
}

static void board_button_init(void)
{
    button_handle_t btn_handle = iot_button_create(BUTTON_IO_NUM, BUTTON_ACTIVE_LEVEL);
    if (btn_handle) {
        iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, button_tap_cb, "RELEASE");
    }
}

void board_init(void)
{
    board_button_init();
}
