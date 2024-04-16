/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-11 11:06:54
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-26 16:16:41
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\data_manage.h
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */
#ifndef _DATA_H_
#define _DATA_H_
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"

#define DATA_TAG "DATA"

#define BACKGROUND_NOISE -100

#define X_B50A 0
#define Y_B50A 0

#define X_CAE6 4
#define Y_CAE6 0

#define X_EB36 8
#define Y_EB36 0

#define X_774A 4
#define Y_774A 4

#define NOR_NODE_INIT_QUALITY 0
#define NOR_NODE_INIT_DISTANCE 100

#define WAIT_TIME_1 5
#define WAIT_TIME_2 10

#define ID_LEN 2
#define QUALITY_LEN 2
#define THRESHOLD_LEN 2
#define SERIAL_NUM_LEN 2
#define SENSOR_LEN 1
#define USEFUL_MESSAGE_LEN 16

#define HEAD_DATA_LEN 7
#define FINAL_DATA_LEN 31

#define THRESHOLD_HIGH_1 0x7f
#define THRESHOLD_HIGH_2 0xff

#define THRESHOLD_LOW_1 0x04
#define THRESHOLD_LOW_2 0xff

#define PHELLO_FINAL_DATA_LEN 18
#define PHELLO_DATA_LEN PHELLO_FINAL_DATA_LEN - 2

#define ANHSP_FINAL_DATA_LEN 14
#define ANHSP_DATA_LEN ANHSP_FINAL_DATA_LEN - 2

#define HSRREP_FINAL_DATA_LEN 14
#define HSRREP_DATA_LEN HSRREP_FINAL_DATA_LEN - 2

#define ANRREQ_FINAL_DATA_LEN 10
#define ANRREQ_DATA_LEN ANRREQ_FINAL_DATA_LEN - 2

#define ANRREP_FINAL_DATA_LEN 14
#define ANRREP_DATA_LEN ANRREP_FINAL_DATA_LEN - 2

#define RRER_FINAL_DATA_LEN 10
#define RRER_DATA_LEN RRER_FINAL_DATA_LEN - 2

#define MESSAGE_FINAL_DATA_LEN 24
#define MESSAGE_DATA_LEN MESSAGE_FINAL_DATA_LEN - 2

#define BLOCK_MESSAGE_FINAL_DATA_LEN 10
#define BLOCK_MESSAGE_DATA_LEN BLOCK_MESSAGE_FINAL_DATA_LEN - 2

#define BLUDE_PACKET_LENGTH 376
extern SemaphoreHandle_t xCountingSemaphore_send;
extern SemaphoreHandle_t xCountingSemaphore_receive;

extern uint8_t id_774a[ID_LEN];
extern uint8_t id_cae6[ID_LEN];
extern uint8_t id_eb36[ID_LEN];
extern uint8_t id_b50a[ID_LEN];
extern uint8_t id_0936[ID_LEN];

typedef struct my_info {
    bool moveable;
    bool is_root;
    bool ready_to_connect;
    bool is_connected;
    uint8_t x;
    uint8_t y;
    uint8_t distance;
    uint8_t quality_from_me[QUALITY_LEN];
    uint8_t quality_from_me_to_neighbor[QUALITY_LEN];
    uint8_t my_id[ID_LEN];
    uint8_t root_id[ID_LEN];
    uint8_t next_id[ID_LEN];
    uint8_t threshold[THRESHOLD_LEN];
    uint8_t update;
} my_info, *p_my_info;

typedef struct phello_info {
    bool moveable;
    bool is_root;
    bool is_connected;
    uint8_t x;
    uint8_t y;
    uint8_t distance;
    // uint8_t quality[QUALITY_LEN];
    uint8_t quality[QUALITY_LEN];
    uint8_t node_id[ID_LEN];
    uint8_t root_id[ID_LEN];
    uint8_t next_id[ID_LEN];
    uint8_t update;

} phello_info, *p_phello_info;

typedef struct anhsp_info {
    uint8_t distance;
    uint8_t quality[QUALITY_LEN];
    uint8_t node_id[ID_LEN];
    uint8_t source_id[ID_LEN];
    uint8_t next_id[ID_LEN];
    uint8_t last_root_id[ID_LEN];
} anhsp_info, *p_anhsp_info;

typedef struct hsrrep_info {
    uint8_t distance;
    uint8_t quality[QUALITY_LEN];
    uint8_t node_id[ID_LEN];
    uint8_t destination_id[ID_LEN];
    uint8_t reverse_next_id[ID_LEN];
} hsrrep_info, *p_hsrrep_info;

typedef struct anrreq_info {
    uint8_t quality_threshold[THRESHOLD_LEN];
    uint8_t source_id[ID_LEN];
    uint8_t serial_number[SERIAL_NUM_LEN];
} anrreq_info, *p_anrreq_info;

typedef struct anrrep_info {
    uint8_t distance;
    uint8_t quality[QUALITY_LEN];
    uint8_t node_id[ID_LEN];
    uint8_t destination_id[ID_LEN];
    uint8_t root_id[ID_LEN];
    uint8_t serial_number[SERIAL_NUM_LEN];
} anrrep_info, *p_anrrep_info;

typedef struct rrer_info {
    uint8_t node_id[ID_LEN];
    uint8_t destination_id[ID_LEN];
} rrer_info, *p_rrer_info;

typedef struct message_info {
    uint8_t source_id[ID_LEN];
    uint8_t next_id[ID_LEN];
    uint8_t destination_id[ID_LEN];
    uint8_t useful_message[USEFUL_MESSAGE_LEN];//保留数据
    uint8_t temperature[SENSOR_LEN];
    uint8_t humidity[SENSOR_LEN];
    uint8_t infrared[SENSOR_LEN];
    uint8_t illumination[SENSOR_LEN];
    uint8_t smoke[SENSOR_LEN];
} message_info, *p_message_info;

typedef struct block_message_info {
    uint8_t node_id[ID_LEN];
    uint8_t destination_id[ID_LEN];
} block_message_info, *p_block_message_info;

extern my_info my_information;

extern uint8_t threshold_high[QUALITY_LEN];

extern uint8_t threshold_low[QUALITY_LEN];

// extern uint8_t temp_id[ID_LEN];

// extern uint8_t phello_final[PHELLO_FINAL_DATA_LEN];

extern uint8_t adv_data_name_7[HEAD_DATA_LEN];

extern uint8_t temp_data_31[FINAL_DATA_LEN];

extern uint8_t adv_data_final_for_hello[FINAL_DATA_LEN];

extern uint8_t adv_data_final_for_anhsp[FINAL_DATA_LEN];

extern uint8_t adv_data_final_for_anrreq[FINAL_DATA_LEN];

extern uint8_t adv_data_final_for_rrer[FINAL_DATA_LEN];

extern uint8_t adv_data_final_for_message[FINAL_DATA_LEN];

extern uint8_t adv_data_final_for_block_message[FINAL_DATA_LEN];

extern uint8_t adv_data_message_16[MESSAGE_DATA_LEN - 6];
// 31字节数据
extern uint8_t adv_data_31[31];

extern uint8_t adv_data_62[62];

uint8_t *data_match(uint8_t *data1, uint8_t *data2, uint8_t data_1_len, uint8_t data_2_len);

uint8_t *quality_calculate_from_me_to_neighbor(double SNR_hat);

uint8_t *quality_calculate_from_me_to_cluster(uint8_t *quality_from_me_to_neighbor, uint8_t *quality, uint8_t distance);

void my_info_init(p_my_info my_information, uint8_t *my_mac);

uint8_t *generate_phello(p_my_info info);

void resolve_phello(uint8_t *phello_data, p_my_info info, int rssi);

uint8_t *generate_anhsp(p_my_info info);

uint8_t *generate_transfer_anhsp(p_anhsp_info anhsp_info, p_my_info info);

void resolve_anhsp(uint8_t *anhsp_data, p_my_info info);

uint8_t *generate_hsrrep(p_my_info info, uint8_t *des_id);

uint8_t *generate_transfer_hsrrep(p_hsrrep_info hsrrep_info, p_my_info info);

void resolve_hsrrep(uint8_t *hsrrep_data, p_my_info info);

uint8_t *generate_anrreq(p_my_info info);

void resolve_anrreq(uint8_t *anrreq_data, p_my_info info);

uint8_t *generate_anrrep(p_my_info info, uint8_t *des_id);

void resolve_anrrep(uint8_t *anrrep_data, p_my_info info, int rssi);

uint8_t *generate_rrer(p_my_info info);

void resolve_rrer(uint8_t *rrer_data, p_my_info info);

uint8_t *generate_message(uint8_t *message_data, p_my_info info, uint8_t *des_id);

uint8_t *generate_transfer_message(p_message_info message_info, p_my_info info);

void resolve_message(uint8_t *message_data, p_my_info info);

uint8_t *generate_block_message(p_my_info info);

void resolve_block_message(uint8_t *block_message_data, p_my_info info);
#endif // _DATA_H_