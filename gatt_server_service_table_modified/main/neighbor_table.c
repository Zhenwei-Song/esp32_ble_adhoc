/*
 * @Author: Zhenwei Song zhenwei.song@qq.com
 * @Date: 2023-12-05 17:18:06
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-28 10:01:22
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\neighbor_table.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2024 by Zhenwei Song, All Rights Reserved.
 */
/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-09 15:05:15
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-16 19:21:04
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\neighbor_table.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */

#include "esp_log.h"

#include "ble_quality.h"
#include "ble_queue.h"
#include "ble_timer.h"
#include "down_routing_table.h"
#include "macro_def.h"
#include "neighbor_table.h"

bool refresh_flag_for_neighbor = false;

bool threshold_high_flag = false;

bool threshold_low_flag = false;

neighbor_table my_neighbor_table;

/**
 * @description: 初始化邻居表
 * @param {p_neighbor_table} table
 * @return {*}
 */
void init_neighbor_table(p_neighbor_table table)
{
    table->head = NULL;
}

/**
 * @description: 向邻居表添加节点
 * @param {p_neighbor_table} table
 * @param {uint8_t} *new_id
 * @param {bool} is_root
 * @param {bool} is_connected
 * @param {uint8_t} *quality
 * @param {uint8_t} distance
 * @return {*}
 */
int insert_neighbor_node(p_neighbor_table table, uint8_t *new_id, bool is_root, bool is_connected, uint8_t *quality, uint8_t distance, int rssi, uint8_t *next_id)
{
    p_neighbor_note new_node = (p_neighbor_note)malloc(sizeof(neighbor_note));
    p_neighbor_note prev = NULL;
    if (new_node == NULL) {
        ESP_LOGE(NEIGHBOR_TAG, "malloc failed");
        return -1;
    }
    memcpy(new_node->id, new_id, ID_LEN);
    new_node->is_root = is_root;
    new_node->is_connected = is_connected;
    memcpy(new_node->quality, quality, QUALITY_LEN);
    if (next_id != NULL)
        memcpy(new_node->next_id, next_id, ID_LEN);
    new_node->distance = distance;
    new_node->rssi = rssi;
    // new_node->quality = quality;
    new_node->count = NEIGHBOR_TABLE_COUNT;
    p_neighbor_note cur = table->head;
    int repeated = -1;
    if (table->head == NULL) { // 路由表为空
        double RSSI = pow(10, 0.1 * (float)(rssi));
        double back_noise = pow(10, 0.1 * (float)(BACKGROUND_NOISE));
        double snr = RSSI / back_noise;
        new_node->SNRhat = init_number;
        new_node->P = 1;
        Kalman((&new_node->SNRhat), snr, (&new_node->P), Q, R);
        table->head = new_node;
        new_node->next = NULL;
        return 0;
    }
    else {
        while (cur != NULL) {
            // ESP_LOGI(NEIGHBOR_TAG, "table->head->id addr: %p", cur->id);
            repeated = memcmp(cur->id, new_node->id, ID_LEN);
            if (repeated == 0) { // 检查重复,重复则更新
                // ESP_LOGI(NEIGHBOR_TAG, "repeated id address found");
                cur->is_root = new_node->is_root;
                cur->is_connected = new_node->is_connected;
                memcpy(cur->quality, new_node->quality, QUALITY_LEN);
                if (next_id != NULL)
                    memcpy(cur->next_id, new_node->next_id, ID_LEN);
                // cur->quality = new_node->quality;
                cur->distance = new_node->distance;
                cur->rssi = new_node->rssi;
                cur->count = new_node->count; // 重新计数
                double RSSI = pow(10, 0.1 * (float)(rssi));
                double back_noise = pow(10, 0.1 * (float)(BACKGROUND_NOISE));
                double snr = RSSI / back_noise;
                // printf("rssi:%d, RSSI:%f, back_noise:%10f,BACKGROUND_NOISE: %f snr:%f\n", cur->rssi, RSSI, back_noise, (float)BACKGROUND_NOISE, snr);
                Kalman((&cur->SNRhat), snr, (&cur->P), Q, R);
                free(new_node);
                return 0;
            }
            prev = cur;
            cur = cur->next;
        }
        if (prev != NULL) {
            double RSSI = pow(10, 0.1 * (float)(rssi));
            double back_noise = pow(10, 0.1 * (float)(BACKGROUND_NOISE));
            double snr = RSSI / back_noise;
            new_node->P = 1;
            new_node->SNRhat = init_number;
            Kalman((&new_node->SNRhat), snr, (&new_node->P), Q, R);
            prev->next = new_node; // 表尾添加新项
            new_node->next = NULL;
        }
    }
#ifdef PRINT_NEIGHBOR_TABLE_STATES
    ESP_LOGW(NEIGHBOR_TAG, "ADD NEW NEIGHBOT NOTE");
#endif
    // print_neighbor_table(table);
    return 0;
}

/**
 * @description:从邻居链表移除项
 * @param {p_neighbor_table} table
 * @param {p_neighbor_note} old_neighbor
 * @return {*}
 */
void remove_neighbor_node_from_node(p_neighbor_table table, p_neighbor_note old_neighbor)
{
    if (table->head != NULL) {
        p_neighbor_note prev = NULL;
        if (table->head == old_neighbor) { // 移除头部
            table->head = table->head->next;
            free(old_neighbor);
        }
        else { // 找出old_neighbor的上一项
            prev = table->head;
            while (prev != NULL) {
                if (prev->next == old_neighbor) // 找到要删除的项
                    break;
                else
                    prev = prev->next;
            }
            if (prev != NULL) {
                prev->next = old_neighbor->next;
                free(old_neighbor);
            }
        }
    }
}

/**
 * @description: 从邻居表移除项（根据id）
 * @param {p_neighbor_table} table
 * @param {uint8_t} *old_id
 * @return {*}
 */
void remove_neighbor_node(p_neighbor_table table, uint8_t *old_id)
{
    if (table->head != NULL) {
        p_neighbor_note prev = NULL;
        if (memcmp(table->head->id, old_id, ID_LEN) == 0) { // 移除头部
            prev = table->head;
            table->head = table->head->next;
            free(prev);
        }
        else { // 找出old_neighbor的上一项
            prev = table->head;
            while (prev != NULL) {
                if (prev->next->id == old_id) // 找到要删除的项
                    break;
                else
                    prev = prev->next;
            }
            if (prev != NULL) {
                p_neighbor_note old_neighbor = prev->next;
                prev->next = prev->next->next;
                free(old_neighbor);
            }
        }
    }
}

/**
 * @description: 判断邻居表是否为空
 * @param {p_neighbor_table} table
 * @return {*}true：为空    false：非空
 */
bool is_neighbor_table_empty(p_neighbor_table table)
{
    if (table->head == NULL)
        return true;
    else
        return false;
}

/**
 * @description: 检查邻居表中是否有相同项
 * @param {p_neighbor_table} table
 * @param {p_neighbor_note} neighbor_note
 * @return {*} true:有相同项;false：无相同项
 */
bool neighbor_table_check_id(p_neighbor_table table, uint8_t *id)
{
    p_neighbor_note temp = table->head;
    if (temp == NULL) {
        return false;
    }
    else {
        while (temp != NULL) {
            if (memcmp(temp->id, id, ID_LEN) == 0)
                return true;
            else
                temp = temp->next;
        }
        return false;
    }
}
#if 0
void update_my_connection_info(p_neighbor_table table, p_my_info my_info)
{
    my_info->able_to_connect = false;
    if (!is_neighbor_table_empty(table)) { // 邻居表非空
        p_neighbor_note temp_node;
        temp_node = table->head;
        while (temp_node != NULL) {
            if (temp_node->is_connected) {
                my_info->able_to_connect = true;
                my_info->is_connected |= 1;
                if (temp_node->quality[0] < my_info->quality[0] || (temp_node->quality[0] == my_info->quality[0] && temp_node->quality[1] < my_info->quality[1])) {
                    memcpy(my_info->next_id, temp_node->id, ID_LEN);
                    memcpy(my_info->quality, temp_node->quality, QUALITY_LEN);
                }
            }
            temp_node = temp_node->next;
        }
        ESP_LOGE(DATA_TAG, "update_my_connection_info finished");
    }
}
#endif
/**
 * @description: 设置特定id的distance
 * @param {p_neighbor_table} table
 * @param {uint8_t} *id
 * @param {uint8_t} distance
 * @return {*}0:成功
 */
int set_neighbor_node_distance(p_neighbor_table table, uint8_t *id, int8_t distance)
{
    p_neighbor_note temp = table->head;
    if (temp == NULL) {
        return -1;
    }
    else {
        while (temp != NULL) {
            if (memcmp(temp->id, id, ID_LEN) == 0) { // 找到了
                temp->distance = distance;
                return 0;
            }
            else {
                temp = temp->next;
            }
        }
        return -1;
    }
}

/**
 * @description:获取特定id的distance
 * @param {p_neighbor_table} table
 * @param {uint8_t} *id
 * @return {*}成功：返回distance
 */
uint8_t get_neighbor_node_distance(p_neighbor_table table, uint8_t *id)
{
    p_neighbor_note temp = table->head;
    if (temp == NULL) {
        return 0;
    }
    else {
        while (temp != NULL) {
            if (memcmp(temp->id, id, ID_LEN) == 0) { // 找到了
                return temp->distance;
            }
            temp = temp->next;
        }
        return 0;
    }
}

uint8_t *get_neighbor_node_quality_from_me(p_neighbor_table table, uint8_t *id)
{
    p_neighbor_note temp = table->head;
    if (temp == NULL) {
        return 0;
    }
    else {
        while (temp != NULL) {
            if (memcmp(temp->id, id, ID_LEN) == 0) { // 找到了
                return temp->quality_from_me;
            }
            temp = temp->next;
        }
        return 0;
    }
}

uint8_t get_neighbor_node_number(p_neighbor_table table)
{
    uint8_t number = 0;
    p_neighbor_note temp = table->head;
    if (temp == NULL) {
        return 0;
    }
    else {
        while (temp != NULL) {
            number = number + 1;
            temp = temp->next;
        }
    }
    return number;
}

/**
 * @description: 更新邻居表的计数号（-1）
 * @param {p_neighbor_table} table
 * @return {*}
 */
void refresh_cnt_neighbor_table(p_neighbor_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->count == 0) {
#ifndef SELF_ROOT
                if (memcmp(temp->id, info->next_id, ID_LEN) == 0) { // 若自己的父节点无了，更新自己info
                    // info->update = info->update + 1;
                    info->is_connected = false;
                    info->distance = NOR_NODE_INIT_DISTANCE;
                    info->quality_from_me[0] = NOR_NODE_INIT_QUALITY;
                    memset(info->root_id, 0, ID_LEN);
                    memset(info->next_id, 0, ID_LEN);
// refresh_flag_for_neighbor = true;
//  update_my_connection_info(table, info);
#ifdef PRINT_NEIGHBOR_TABLE_STATES
                    ESP_LOGE(NEIGHBOR_TAG, "father deleted"); // 自己的父节点无了，发送rrer
#endif
                    memcpy(adv_data_final_for_rrer, data_match(adv_data_name_7, generate_rrer(info), HEAD_DATA_LEN, RRER_FINAL_DATA_LEN), FINAL_DATA_LEN);
                    queue_push(&send_queue, adv_data_final_for_rrer, 0);
                    vTaskDelay(pdMS_TO_TICKS(RERR_REPEAT_TIME));
                    queue_push(&send_queue, adv_data_final_for_rrer, 0);
                    xSemaphoreGive(xCountingSemaphore_send);
                    // 开始计时
                    esp_timer_start_once(ble_time3_timer, TIME3_TIMER_PERIOD);
                    timer3_running = true;
                    destroy_down_routing_table(&my_down_routing_table); // 清空路由表
                }
#endif
                p_neighbor_note next_temp = temp->next; // 保存下一个节点以防止删除后丢失指针
                remove_neighbor_node_from_node(table, temp);
                temp = next_temp; // 更新temp为下一个节点
#ifdef PRINT_NEIGHBOR_TABLE_STATES
                ESP_LOGW(NEIGHBOR_TAG, "neighbor table node deleted");
#endif
            }
            else {
                temp->count = temp->count - 1;
                temp = temp->next; // 继续到下一个节点
                // ESP_LOGW(NEIGHBOR_TAG, "neighbor table refreshed");
            }
        }
        // print_neighbor_table(table);
    }
    else { // 邻居表里一个节点都没有,但是不是因为自己的父节点断开
#ifndef SELF_ROOT
        info->is_connected = false;
        info->distance = 100;
        info->quality_from_me[0] = NOR_NODE_INIT_QUALITY;
        memset(info->root_id, 0, ID_LEN);
        memset(info->next_id, 0, ID_LEN);
        // refresh_flag_for_neighbor = true;
#endif
    }
}

/**
 * @description: 更新邻居表中每一项的链路质量，并判断处于阈值哪个区间
 * @param {p_neighbor_table} table
 * @param {p_my_info} info
 * @return {*}
 */
void update_quality_of_neighbor_table(p_neighbor_table table, p_my_info info)
{
    threshold_high_flag = 0;
    threshold_low_flag = 0;
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->is_connected == true) { // 若为入网节点
                // 完成对邻居表中 我到邻居节点的链路质量 乘积项*65535
                memcpy(temp->quality_from_me_to_neighbor, quality_calculate_from_me_to_neighbor(temp->SNRhat), QUALITY_LEN);
                memcpy(temp->quality_from_me, quality_calculate_from_me_to_cluster(temp->quality_from_me_to_neighbor, temp->quality, temp->distance), QUALITY_LEN);
                if (memcmp(temp->id, info->next_id, ID_LEN) == 0) { // 更新自己到父节点的链路质量
                    memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                }
                if (memcmp(temp->quality_from_me, threshold_high, QUALITY_LEN) >= 0) { // 大于阈值1
                    threshold_high_flag |= 1;
                    threshold_low_flag |= 0;
                }
                else if (memcmp(temp->quality_from_me, threshold_low, QUALITY_LEN) >= 0) { // 大于阈值2,小于阈值1
                    threshold_high_flag |= 0;
                    threshold_low_flag |= 1;
                }
                else { // 小于阈值2
                    threshold_high_flag |= 0;
                    threshold_low_flag |= 0;
                }
            }
            temp = temp->next; // 继续到下一个节点
        }
        // print_neighbor_table(table);
    }
    else { // 路由表为空
        threshold_high_flag = 0;
        threshold_low_flag = 0;
    }
}

#if 0
void threshold_high_ops(p_neighbor_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->is_connected == true) {                                          // 若为入网节点
                if (memcmp(temp->quality_from_me, threshold_high, QUALITY_LEN) >= 0) { // 大于阈值1，直接入网
                    info->is_connected |= 1;
                    if (memcmp(info->quality_from_me, temp->quality_from_me, QUALITY_LEN) < 0) { // 若有节点的链路质量比自己当前的链路质量高
                        if (memcmp(info->next_id, temp->id, ID_LEN) == 0) {              // 若它就是自己的next id，则更新自己的链路质量
                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                        else { // 更新自己的next_id
                            memcpy(info->next_id, temp->id, ID_LEN);
                            info->distance = temp->distance + 1;
                            refresh_flag_for_neighbor = true; // 因为自己的next_id变了，所以立即广播hello
                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                    }
                }
            }
            temp = temp->next; // 继续到下一个节点
        }
        // print_neighbor_table(table);
    }
}
#else
/**
 * @description: 阈值区域1时的入网行为
 * @param {p_neighbor_table} table
 * @param {p_my_info} info
 * @return {*}
 */
void threshold_high_ops(p_neighbor_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->is_connected == true && memcmp(temp->next_id, info->my_id, ID_LEN) != 0) { // 若为入网节点
                if (memcmp(temp->quality_from_me, threshold_high, QUALITY_LEN) >= 0) {           // 大于阈值1，直接入网
                    info->is_connected |= 1;
                    if (memcmp(info->quality_from_me, temp->quality_from_me, QUALITY_LEN) < 0) { // 若有节点的链路质量比自己当前的链路质量高
                        if (memcmp(info->next_id, temp->id, ID_LEN) == 0) {                      // 若它就是自己的next id，则更新自己的链路质量
                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                        else { // 更新自己的next_id
                            memcpy(info->next_id, temp->id, ID_LEN);
                            info->distance = temp->distance + 1;
                            refresh_flag_for_neighbor = true; // 因为自己的next_id变了，所以立即广播hello
                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                    }
                }
            }
            temp = temp->next; // 继续到下一个节点
        }
        // print_neighbor_table(table);
    }
}
#endif

/**
 * @description: 阈值区域2时入网行为
 * @param {p_neighbor_table} table
 * @param {p_my_info} info
 * @return {*}
 */
void threshold_between_ops(p_neighbor_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->is_connected == true && memcmp(temp->next_id, info->my_id, ID_LEN) != 0) {                                                        // 若为入网节点
                if (memcmp(temp->quality_from_me, threshold_low, QUALITY_LEN) >= 0 && memcmp(temp->quality_from_me, threshold_high, QUALITY_LEN) < 0) { // 大于阈值2，发送ANHSP`
                    if (memcmp(info->quality_from_me, temp->quality_from_me, QUALITY_LEN) < 0) {                                                        // 若有节点的链路质量比自己当前的链路质量高
                        if (memcmp(info->next_id, temp->id, ID_LEN) == 0) {                                                                             // 若它就是自己的next id，则更新自己的链路质量

                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                        else { // 更新自己的next_id
                            memcpy(info->next_id, temp->id, ID_LEN);

                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                    }
                }
            }
            temp = temp->next; // 继续到下一个节点
        }
        /* -------------------------------------------------------------------------- */
        /*                               向next_id发送入网请求包                              */
        /* -------------------------------------------------------------------------- */
        memcpy(adv_data_final_for_anhsp, data_match(adv_data_name_7, generate_anhsp(info), HEAD_DATA_LEN, ANHSP_FINAL_DATA_LEN), FINAL_DATA_LEN);
        queue_push(&send_queue, adv_data_final_for_anhsp, 0);
        xSemaphoreGive(xCountingSemaphore_send);
#ifdef BLE_TIMER
        // 开始计时
        esp_timer_start_once(ble_time1_timer, TIME1_TIMER_PERIOD);
        timer1_running = true;
#ifdef PRINT_TIMER_STATES
        printf("between_ops timer1 started\n");
#endif
#endif
        // print_neighbor_table(table);
    }
}

/**
 * @description: 阈值区域3时入网行为
 * @param {p_neighbor_table} table
 * @param {p_my_info} info
 * @return {*}
 */
void threshold_low_ops(p_neighbor_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->is_connected == true && memcmp(temp->next_id, info->my_id, ID_LEN) != 0) { // 若为入网节点
                if (memcmp(temp->quality_from_me, threshold_low, QUALITY_LEN) < 0) {             // 小于阈值2，发送ANRREQ
                    if (memcmp(info->quality_from_me, temp->quality_from_me, QUALITY_LEN) < 0) { // 若有节点的链路质量比自己当前的链路质量高
                        if (memcmp(info->next_id, temp->id, ID_LEN) == 0) {                      // 若它就是自己的next id，则更新自己的链路质量
                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                        else { // 更新自己的next_id
                            memcpy(info->next_id, temp->id, ID_LEN);
                            memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                        }
                    }
                }
            }
            temp = temp->next; // 继续到下一个节点
        }
        /* -------------------------------------------------------------------------- */
        /*                               向next_id发送入网请求包                              */
        /* -------------------------------------------------------------------------- */
        memcpy(adv_data_final_for_anrreq, data_match(adv_data_name_7, generate_anrreq(info), HEAD_DATA_LEN, ANRREQ_FINAL_DATA_LEN), FINAL_DATA_LEN);
        queue_push(&send_queue, adv_data_final_for_anrreq, 0);
        xSemaphoreGive(xCountingSemaphore_send);
#ifdef BLE_TIMER
        // 开始计时
        esp_timer_start_once(ble_time2_timer, TIME2_TIMER_PERIOD);
        timer2_running = true;
#ifdef PRINT_TIMER_STATES
        printf("low_ops timer2 started\n");
#endif

#endif
        // print_neighbor_table(table);
    }
}

/**
 * @description: 设置我的下一跳的链路质量和距离（未使用）
 * @param {p_neighbor_table} table
 * @param {p_my_info} info
 * @return {*}
 */
void set_my_next_id_quality_and_distance(p_neighbor_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_neighbor_note temp = table->head;
        while (temp != NULL) {
            if (temp->is_connected == true) {                                          // 若为入网节点
                if (memcmp(temp->quality_from_me, threshold_high, QUALITY_LEN) >= 0) { // 大于阈值1，直接入网
                    info->is_connected |= 1;
                    if (memcmp(info->next_id, temp->id, ID_LEN) == 0) { // 若它就是自己的next id，则更新自己的链路质量
                        memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                    }
                    if (memcmp(info->quality_from_me, temp->quality_from_me, QUALITY_LEN) > 0) { // 若有邻居节点的链路质量比自己当前的链路质量高
                        if (memcmp(info->next_id, temp->id, ID_LEN) != 0) {                      // 更新了自己的next_id
                            memcpy(info->next_id, temp->id, ID_LEN);
                            info->distance = temp->distance + 1;
                            refresh_flag_for_neighbor = true; // 因为自己的next_id变了，所以立即广播hello
                        }
                        memcpy(info->quality_from_me, temp->quality_from_me, QUALITY_LEN);
                    }
                }
                else if (memcmp(temp->quality_from_me, threshold_low, QUALITY_LEN) >= 0) { // 小于阈值1大于阈值2，发送入网请求
                }
            }
            temp = temp->next; // 继续到下一个节点
        }
        // print_neighbor_table(table);
    }
}

/**
 * @description: 打印邻居表
 * @param {p_neighbor_table} table
 * @return {*}
 */
void print_neighbor_table(p_neighbor_table table)
{
    p_neighbor_note temp = table->head;
    ESP_LOGI(NEIGHBOR_TAG, "****************************Start printing neighbor table:***********************************************");
    while (temp != NULL) {
        ESP_LOGI(NEIGHBOR_TAG, "id:");
        esp_log_buffer_hex(NEIGHBOR_TAG, temp->id, ID_LEN);
        ESP_LOGI(NEIGHBOR_TAG, "is_root:%d", temp->is_root);
        ESP_LOGI(NEIGHBOR_TAG, "is_connected:%d", temp->is_connected);
        ESP_LOGI(NEIGHBOR_TAG, "quality_from_neighbor_to_cluster:");
        esp_log_buffer_hex(NEIGHBOR_TAG, temp->quality, QUALITY_LEN);
        ESP_LOGI(NEIGHBOR_TAG, "quality_from_me_to_cluster_via_neighbor:");
        esp_log_buffer_hex(NEIGHBOR_TAG, temp->quality_from_me, QUALITY_LEN);
        ESP_LOGI(NEIGHBOR_TAG, "quality_from_me_to_neighbor:");
        esp_log_buffer_hex(NEIGHBOR_TAG, temp->quality_from_me_to_neighbor, QUALITY_LEN);
        ESP_LOGI(NEIGHBOR_TAG, "next_id:");
        esp_log_buffer_hex(NEIGHBOR_TAG, temp->next_id, ID_LEN);
        ESP_LOGI(NEIGHBOR_TAG, "distance:%d", temp->distance);
        ESP_LOGI(NEIGHBOR_TAG, "rssi:%d", temp->rssi);
        ESP_LOGI(NEIGHBOR_TAG, "SNRhat:%f", temp->SNRhat);
        ESP_LOGI(NEIGHBOR_TAG, "P:%f", temp->P);
        // ESP_LOGI(NEIGHBOR_TAG, "count:%d", temp->count);
        temp = temp->next;
    }
    ESP_LOGI(NEIGHBOR_TAG, "****************************Printing neighbor table is finished *****************************************");
}

/**
 * @description: 销毁路由表
 * @param {p_neighbor_table} table
 * @return {*}
 */
void destroy_neighbor_table(p_neighbor_table table)
{
#ifdef PRINT_NEIGHBOR_TABLE_STATES
    ESP_LOGW(NEIGHBOR_TAG, "Destroying neighbor_table!");
#endif
    while (table->head != NULL)
        remove_neighbor_node_from_node(table, table->head);
#ifdef PRINT_NEIGHBOR_TABLE_STATES
    ESP_LOGW(NEIGHBOR_TAG, "Destroying neighbor_table finished!");
#endif
}