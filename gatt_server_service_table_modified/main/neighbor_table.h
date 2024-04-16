/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-09 15:05:02
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-24 17:21:54
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\neighbor_table.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */
#ifndef _NEIGHBOR_TABLE_H_
#define _NEIGHBOR_TABLE_H_

#define NEIGHBOR_TAG "NEIGHBOR_TABLE"

#include "data_manage.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define init_number 10
typedef struct neighbor_note {
    uint8_t id[ID_LEN];
    uint8_t next_id[ID_LEN];
    bool is_root;
    bool is_connected;
    uint8_t quality[QUALITY_LEN];                     // 邻居节点到簇首的链路质量（乘积项）
    uint8_t quality_from_me[QUALITY_LEN];             // 我通过邻居节点到簇首的链路质量
    uint8_t quality_from_me_to_neighbor[QUALITY_LEN]; // 我到邻居节点的链路质量  乘积项*65535
    // uint8_t quality;
    uint8_t distance; // 邻居到簇首的跳数
    int rssi;
    double SNRhat;
    double P;
    uint8_t count;
    struct neighbor_note *next;

} neighbor_note, *p_neighbor_note;

typedef struct neighbor_table {
    p_neighbor_note head;
} neighbor_table, *p_neighbor_table;

extern neighbor_table my_neighbor_table;

extern bool refresh_flag_for_neighbor;

extern bool threshold_high_flag;

extern bool threshold_low_flag;

void init_neighbor_table(p_neighbor_table table);

// int insert_neighbor_node(p_neighbor_table table, p_neighbor_note new_neighbor);
int insert_neighbor_node(p_neighbor_table table, uint8_t *new_id, bool is_root, bool is_connected, uint8_t *quality, uint8_t distance, int rssi, uint8_t *next_id);

    void remove_neighbor_node_from_node(p_neighbor_table table, p_neighbor_note old_neighbor);

void remove_neighbor_node(p_neighbor_table table, uint8_t *old_id);

bool is_neighbor_table_empty(p_neighbor_table table);

bool neighbor_table_check_id(p_neighbor_table table, uint8_t *id);

// void update_my_connection_info(p_neighbor_table table, p_my_info my_info);

int set_neighbor_node_distance(p_neighbor_table table, uint8_t *id, int8_t distance);

uint8_t get_neighbor_node_distance(p_neighbor_table table, uint8_t *id);

uint8_t *get_neighbor_node_quality_from_me(p_neighbor_table table, uint8_t *id);

uint8_t get_neighbor_node_number(p_neighbor_table table);

    void refresh_cnt_neighbor_table(p_neighbor_table table, p_my_info info);

void update_quality_of_neighbor_table(p_neighbor_table table, p_my_info info);

void threshold_high_ops(p_neighbor_table table, p_my_info info);

void threshold_between_ops(p_neighbor_table table, p_my_info info);

void threshold_low_ops(p_neighbor_table table, p_my_info info);

void set_my_next_id_quality_and_distance(p_neighbor_table table, p_my_info info);

void print_neighbor_table(p_neighbor_table table);

void destroy_neighbor_table(p_neighbor_table table);

#endif // _NEIGHBOR_TABLE_H_