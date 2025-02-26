/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-09 15:05:02
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-22 13:17:34
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\up_routing_table.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */
#ifndef _UP_ROUTING_TABLE_H_
#define _UP_ROUTING_TABLE_H_

#define UP_ROUTING_TAG "UP_ROUTING_TABLE"

#include "data_manage.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UP_ROUTING_TABLE_COUNT 10

typedef struct up_routing_note {
    uint8_t destination_id[ID_LEN];
    uint8_t next_id[ID_LEN];
    uint8_t distance_from_me;
    uint8_t count;
    struct up_routing_note *next;
} up_routing_note, *p_up_routing_note;

typedef struct up_routing_table {
    p_up_routing_note head;
} up_routing_table, *p_up_routing_table;

extern up_routing_table my_up_routing_table;

void init_up_routing_table(p_up_routing_table table);

// int insert_up_routing_node(p_up_routing_table table, p_up_routing_note new_up_routing);
int insert_up_routing_node(p_up_routing_table table, uint8_t *destination_id, uint8_t *next_id, uint8_t distance_from_me);

void remove_up_routing_node_from_node(p_up_routing_table table, p_up_routing_note old_up_routing);

uint8_t *get_up_routing_head_id(p_up_routing_table table);

void remove_up_routing_node(p_up_routing_table table, uint8_t *old_id);

bool is_up_routing_table_empty(p_up_routing_table table);

bool up_routing_table_check_id(p_up_routing_table table, uint8_t *id);

void refresh_cnt_up_routing_table(p_up_routing_table table, p_my_info info);

uint8_t *get_up_routing_next_id(p_up_routing_table table, uint8_t *des_id);

void print_up_routing_table(p_up_routing_table table);

void destroy_up_routing_table(p_up_routing_table table);

#endif // _UP_ROUTING_TABLE_H_