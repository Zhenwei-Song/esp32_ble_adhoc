/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-09 15:05:15
 * @LastEditors: Zhenwei Song zhenwei.song@qq.com
 * @LastEditTime: 2024-01-19 11:51:02
 * @FilePath: \esp32\esp32_ble\gatt_server_service_table_modified\main\down_routing_table.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */

#include "esp_log.h"

#include "ble_queue.h"
#include "down_routing_table.h"
#include "macro_def.h"

bool refresh_flag_for_down_routing = false;

down_routing_table my_down_routing_table;
/**
 * @description: 初始化路由表
 * @param {p_down_routing_table} table
 * @return {*}
 */
void init_down_routing_table(p_down_routing_table table)
{
    table->head = NULL;
}

/**
 * @description: 向路由表添加节点
 * @param {p_down_routing_table} table
 * @param {uint8_t} *new_id
 * @param {bool} is_root
 * @param {bool} is_connected
 * @param {uint8_t} *quality
 * @param {uint8_t} distance
 * @return {*}
 */
int insert_down_routing_node(p_down_routing_table table, uint8_t *source_id, uint8_t *destination_id, uint8_t *reverse_next_id, uint8_t distance_from_me)
{
    p_down_routing_note new_node = (p_down_routing_note)malloc(sizeof(down_routing_note));
    p_down_routing_note prev = NULL;
    if (new_node == NULL) {
        ESP_LOGE(DOWN_ROUTING_TAG, "malloc failed");
        return -1;
    }
    memcpy(new_node->source_id, source_id, ID_LEN);
    memcpy(new_node->destination_id, destination_id, ID_LEN);
    memcpy(new_node->reverse_next_id, reverse_next_id, ID_LEN);
    new_node->distance_from_me = distance_from_me;
    new_node->count = DOWN_ROUTING_TABLE_COUNT;
    p_down_routing_note cur = table->head;
    int repeated = -1;
    if (table->head == NULL) { // 路由表为空
        table->head = new_node;
        new_node->next = NULL;
        // ESP_LOGW(DOWN_ROUTING_TAG, "ADD NEW DOWN_ROUTING NOTE TO HEAD");
        return 0;
    }
    else {
        while (cur != NULL) {
            // ESP_LOGI(DOWN_ROUTING_TAG, "table->head->id addr: %p", cur->id);
            repeated = memcmp(cur->destination_id, new_node->destination_id, ID_LEN);
            if (repeated == 0) { // 检查重复,重复则更新
                // ESP_LOGI(DOWN_ROUTING_TAG, "repeated id address found");
                // memcpy(cur->destination_id, new_node->destination_id, ID_LEN);
                memcpy(cur->reverse_next_id, new_node->reverse_next_id, ID_LEN);
                cur->distance_from_me = new_node->distance_from_me;
                cur->count = new_node->count; // 重新计数
                free(new_node);
                return 0;
            }
            prev = cur;
            cur = cur->next;
        }
        if (prev != NULL) {
            prev->next = new_node; // 表尾添加新项
            new_node->next = NULL;
        }
    }
#ifdef PRINT_DOWN_ROUTING_TABLE_STATES
    ESP_LOGW(DOWN_ROUTING_TAG, "ADD NEW DOWN_ROUTING NOTE");
#endif
    // print_down_routing_table(table);
    return 0;
}

/**
 * @description:从路由链表移除项
 * @param {p_down_routing_table} table
 * @param {p_down_routing_note} old_down_routing
 * @return {*}
 */
void remove_down_routing_node_from_node(p_down_routing_table table, p_down_routing_note old_down_routing)
{
    if (table->head != NULL) {
        p_down_routing_note prev = NULL;
        if (table->head == old_down_routing) { // 移除头部
            table->head = table->head->next;
            free(old_down_routing);
        }
        else { // 找出old_down_routing的上一项
            prev = table->head;
            while (prev != NULL) {
                if (prev->next == old_down_routing) // 找到要删除的项
                    break;
                else
                    prev = prev->next;
            }
            if (prev != NULL) {
                prev->next = old_down_routing->next;
                free(old_down_routing);
            }
        }
    }
}

/**
 * @description: 从路由表移除项（根据source_id）
 * @param {p_down_routing_table} table
 * @param {uint8_t} *old_id
 * @return {*}
 */
void remove_down_routing_node(p_down_routing_table table, uint8_t *old_id)
{
    if (table->head != NULL) {
        p_down_routing_note prev = NULL;
        if (memcmp(table->head->source_id, old_id, ID_LEN) == 0) { // 移除头部
            prev = table->head;
            table->head = table->head->next;
            free(prev);
        }
        else { // 找出old_down_routing的上一项
            prev = table->head;
            while (prev != NULL) {
                if (prev->next->source_id == old_id) // 找到要删除的项
                    break;
                else
                    prev = prev->next;
            }
            if (prev != NULL) {
                p_down_routing_note old_down_routing = prev->next;
                prev->next = prev->next->next;
                free(old_down_routing);
            }
        }
    }
}

/**
 * @description: 判断路由表是否为空
 * @param {p_down_routing_table} table
 * @return {*}true：为空    false：非空
 */
bool is_down_routing_table_empty(p_down_routing_table table)
{
    if (table->head == NULL)
        return true;
    else
        return false;
}

/**
 * @description: 检查路由表中是否有相同项
 * @param {p_down_routing_table} table
 * @param {p_down_routing_note} down_routing_note
 * @return {*} true:有相同项;false：无相同项
 */
bool down_routing_table_check_id(p_down_routing_table table, uint8_t *id)
{
    p_down_routing_note temp = table->head;
    if (temp == NULL) {
        return false;
    }
    else {
        while (temp != NULL) {
            if (memcmp(temp->destination_id, id, ID_LEN) == 0)
                return true;
            else
                temp = temp->next;
        }
        return false;
    }
}
/**
 * @description: 更新路由表的计数号（-1）
 * @param {p_down_routing_table} table
 * @return {*}
 */
void refresh_cnt_down_routing_table(p_down_routing_table table, p_my_info info)
{
    if (table->head != NULL) {
        p_down_routing_note temp = table->head;
        while (temp != NULL) {
            if (temp->count == 0) {
                p_down_routing_note next_temp = temp->next; // 保存下一个节点以防止删除后丢失指针
                remove_down_routing_node_from_node(table, temp);
                temp = next_temp; // 更新temp为下一个节点
#ifdef PRINT_DOWN_ROUTING_TABLE_STATES
                ESP_LOGW(DOWN_ROUTING_TAG, "down_routing table node deleted");
#endif
            }
            else {
                temp->count = temp->count - 1;
                temp = temp->next; // 继续到下一个节点
                // ESP_LOGW(DOWN_ROUTING_TAG, "down_routing table refreshed");
            }
        }
    }
    else { // 路由表里一个节点都没有
    }
}

/**
 * @description: 根据路由表获取反向路由的下一跳id
 * @param {p_down_routing_table} table
 * @param {uint8_t} *des_id
 * @return {*}
 */
uint8_t *get_down_routing_next_id(p_down_routing_table table, uint8_t *des_id)
{
    p_down_routing_note temp = table->head;
    while (temp != NULL) {
        if (memcmp(temp->destination_id, des_id, ID_LEN) == 0) {
            return temp->reverse_next_id;
        }
        temp = temp->next;
    }
    return NULL;
}

/**
 * @description: 打印路由表
 * @param {p_down_routing_table} table
 * @return {*}
 */
void print_down_routing_table(p_down_routing_table table)
{
    p_down_routing_note temp = table->head;
    ESP_LOGE(DOWN_ROUTING_TAG, "****************************Start printing down_routing table:***********************************************");
    while (temp != NULL) {
        //ESP_LOGI(DOWN_ROUTING_TAG, "source_id:");
        //esp_log_buffer_hex(DOWN_ROUTING_TAG, temp->source_id, ID_LEN);
        ESP_LOGI(DOWN_ROUTING_TAG, "destination_id:");
        esp_log_buffer_hex(DOWN_ROUTING_TAG, temp->destination_id, ID_LEN);
        ESP_LOGI(DOWN_ROUTING_TAG, "reverse_next_id:");
        esp_log_buffer_hex(DOWN_ROUTING_TAG, temp->reverse_next_id, ID_LEN);
        ESP_LOGI(DOWN_ROUTING_TAG, "distance_from_me:%d", temp->distance_from_me);
        // ESP_LOGI(DOWN_ROUTING_TAG, "count:%d", temp->count);
        temp = temp->next;
    }
    ESP_LOGE(DOWN_ROUTING_TAG, "****************************Printing down_routing table is finished *****************************************");
}

/**
 * @description: 销毁路由表
 * @param {p_down_routing_table} table
 * @return {*}
 */
void destroy_down_routing_table(p_down_routing_table table)
{
#ifdef PRINT_DOWN_ROUTING_TABLE_STATES
    ESP_LOGW(DOWN_ROUTING_TAG, "Destroying down_routing_table!");
#endif
    while (table->head != NULL)
        remove_down_routing_node_from_node(table, table->head);
#ifdef PRINT_DOWN_ROUTING_TABLE_STATES
    ESP_LOGW(DOWN_ROUTING_TAG, "Destroying down_routing_table finished!");
#endif
}