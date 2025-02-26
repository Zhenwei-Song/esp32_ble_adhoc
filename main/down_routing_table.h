/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-09 15:05:02
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-19 16:34:10
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\down_routing_table.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */
#ifndef _DOWN_ROUTING_TABLE_H_
#define _DOWN_ROUTING_TABLE_H_

#define DOWN_ROUTING_TAG "DOWN_ROUTING_TABLE"

#include "data_manage.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOWN_ROUTING_TABLE_COUNT 10

typedef struct down_routing_note {
    uint8_t source_id[ID_LEN];
    uint8_t destination_id[ID_LEN];
    uint8_t reverse_next_id[ID_LEN];
    uint8_t distance_from_me;
    uint8_t count;
    struct down_routing_note *next;
} down_routing_note, *p_down_routing_note;

typedef struct down_routing_table {
    p_down_routing_note head;
} down_routing_table, *p_down_routing_table;

extern down_routing_table my_down_routing_table;

void init_down_routing_table(p_down_routing_table table);

// int insert_down_routing_node(p_down_routing_table table, p_down_routing_note new_down_routing);
int insert_down_routing_node(p_down_routing_table table, uint8_t *source_id, uint8_t *destination_id, uint8_t *reverse_next_id, uint8_t distance_from_me);

void remove_down_routing_node_from_node(p_down_routing_table table, p_down_routing_note old_down_routing);

void remove_down_routing_node(p_down_routing_table table, uint8_t *old_id);

bool is_down_routing_table_empty(p_down_routing_table table);

bool down_routing_table_check_id(p_down_routing_table table, uint8_t *id);

void refresh_cnt_down_routing_table(p_down_routing_table table, p_my_info info);

uint8_t *get_down_routing_next_id(p_down_routing_table table, uint8_t *des_id);

void print_down_routing_table(p_down_routing_table table);

void destroy_down_routing_table(p_down_routing_table table);

#endif // _DOWN_ROUTING_TABLE_H_