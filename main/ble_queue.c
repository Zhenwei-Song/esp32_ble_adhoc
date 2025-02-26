/*
 * @Author: Zhenwei-Song zhenwei.song@qq.com
 * @Date: 2023-11-08 16:36:10
 * @LastEditors: Zhenwei Song zhenwei_song@foxmail.com
 * @LastEditTime: 2025-02-26 20:05:01
 * @FilePath: \esp32_ble_positioning\main\ble_queue.c
 * @Description: 仅供学习交流使用
 * Copyright (c) 2023 by Zhenwei-Song, All Rights Reserved.
 */
#include "ble_queue.h"

int temp_rssi = 0;

/**
 * @description: 初始化队列结构
 * @param {queue} *q
 * @return {*}
 */
void queue_init(p_queue q)
{
    q->head = NULL;
    q->tail = NULL;
}

/**
 * @description: 队列尾部插入数据
 * @param {queue} *q
 * @param {uint8_t} data
 * @return {*}
 */
void queue_push(p_queue q, uint8_t *data, int rssi, uint8_t len)
{
    p_qnode new_node = (p_qnode)malloc(sizeof(qnode));
    if (new_node == NULL) {
        ESP_LOGE(QUEUE_TAG, "malloc failed");
    }
    else {
        // memcpy(new_node->data, data, queue_data_length);
        new_node->data = (uint8_t *)malloc(sizeof(uint8_t) * len);
        if (new_node->data == NULL) {
            ESP_LOGE(QUEUE_TAG, "malloc failed for node data");
            free(new_node); // 释放已分配的节点内存
        }
        memcpy(new_node->data, data, len);
        new_node->len = len;
        new_node->rssi = rssi;
        if (q->head == NULL) { // 队列为空
            q->head = q->tail = new_node;
            // new_node->next = NULL;
            q->tail->next = NULL;
        }
        else {
            q->tail->next = new_node;
            q->tail = new_node;
            // new_node->next = NULL;
            q->tail->next = NULL;
        }
    }
}

/**
 * @description: 带重复性检查（检查队列最后一项）的push
 * @param {p_queue} q
 * @param {uint8_t} *data
 * @return {*}
 */
void queue_push_with_check(p_queue q, uint8_t *data, int rssi, uint8_t len)
{
    int repeated = -1;
    if (q->tail == NULL) { // 队列为空
        p_qnode new_node_head = (p_qnode)malloc(sizeof(qnode));
        if (new_node_head == NULL) {
            ESP_LOGE(QUEUE_TAG, "malloc failed");
        }
        new_node_head->data = (uint8_t *)malloc(sizeof(uint8_t) * len);
        memcpy(new_node_head->data, data, len);
        new_node_head->len = len;
        new_node_head->rssi = rssi;
        q->head = q->tail = new_node_head;
        new_node_head->next = NULL;
        q->tail->next = NULL;
        // ESP_LOGW(QUEUE_TAG, "message add to head");
    }
    else {
        repeated = memcmp(data, q->tail->data, len);
        if (repeated != 0) { // 检查是否与队列中最后一个数据相同

            p_qnode new_node = (p_qnode)malloc(sizeof(qnode));
            if (new_node == NULL) {
                ESP_LOGE(QUEUE_TAG, "malloc failed");
            }
            else {
                new_node->data = (uint8_t *)malloc(sizeof(uint8_t) * len);
                memcpy(new_node->data, data, len);
                new_node->len = len;
                new_node->rssi = rssi;
                q->tail->next = new_node;
                q->tail = new_node;
                q->tail->next = NULL;
            }
            // ESP_LOGW(QUEUE_TAG, "message add to queue");
        }
        else {
            // ESP_LOGW(QUEUE_TAG, "message is repeated");
        }
    }
}
/**
 * @description: 队列头出数据
 * @param {p_queue} q
 * @return {*}
 */
uint8_t *queue_pop(p_queue q, uint8_t *len)
{
    if (q->head != NULL) {
        uint8_t data_len = q->head->len;
        uint8_t *pop_data = (uint8_t *)malloc(sizeof(uint8_t) * data_len);
        if (pop_data == NULL) {
            ESP_LOGE(QUEUE_TAG, "malloc failed");
            return NULL;
        }
        else {
            memcpy(pop_data, q->head->data, data_len);
            temp_rssi = q->head->rssi;
            p_qnode temp = q->head;
            *len = data_len;
            if (q->head->next == NULL) { // 仅有一个node
                q->head = q->tail = NULL;
            }
            else {
                q->head = q->head->next;
            }
            free(temp->data); // 释放节点中存储的数据
            free(temp);       // 释放节点本身
            return pop_data;
        }
    }
    else { // 队列为空
        ESP_LOGW(QUEUE_TAG, "pop NULL");
        return NULL;
    }
}

/**
 * @description: 获取某项的rssi
 * @param {p_queue} q
 * @return {*}
 */
int get_queue_node_rssi(p_queue q)
{
    if (q->head != NULL) {
        temp_rssi = q->head->rssi;
        return temp_rssi;
    }
    return -1;
}

/**
 * @description: 判断队列是否为空
 * @param {p_queue} q
 * @return {*} 1：队列为空；0：队列非空
 */
bool queue_is_empty(p_queue q)
{
    if (q->head == NULL)
        return true;
    else
        return false;
}

/**
 * @description: 打印队列(有问题)
 * @param {p_queue} q
 * @return {*}
 */
void queue_print(p_queue q)
{
    p_qnode node = q->head;
    ESP_LOGW(QUEUE_TAG, "********************************Start printing queue:********************************************************");
    while (node != NULL) {
        esp_log_buffer_hex(QUEUE_TAG, node->data, node->len);
        node = node->next;
    }
    ESP_LOGW(QUEUE_TAG, "********************************Printing queue is finished***************************************************");
}

/**
 * @description: 销毁队列
 * @param {p_queue} q
 * @return {*}
 */
void queue_destroy(p_queue q)
{
    uint8_t len;
    ESP_LOGW(QUEUE_TAG, "Destroying queue!");
    while (q->head != NULL)
        queue_pop(q, &len);
    ESP_LOGW(QUEUE_TAG, "Destroying queue finished!");
}