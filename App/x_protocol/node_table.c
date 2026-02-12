/**
 * @file node_table.c
 * @brief TPMesh 节点映射表模块实现
 * 
 * @version 0.6.2
 */

#include "node_table.h"
#include "tpmesh_debug.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>

/* ============================================================================
 * 私有变量
 * ============================================================================ */

/** 节点表 */
static node_entry_t s_node_table[NODE_TABLE_MAX_ENTRIES];

/** 访问互斥锁 */
static SemaphoreHandle_t s_table_mutex = NULL;

/** 初始化状态 */
static bool s_initialized = false;

/* ============================================================================
 * 私有函数
 * ============================================================================ */

/**
 * @brief 比较 MAC 地址
 */
static inline bool mac_equal(const uint8_t *a, const uint8_t *b)
{
    return memcmp(a, b, 6) == 0;
}

/**
 * @brief 获取当前 tick (ms)
 */
static inline uint32_t get_tick_ms(void)
{
    TickType_t ticks = xTaskGetTickCount();
    return (uint32_t)(((uint64_t)ticks * 1000ULL) / (uint64_t)configTICK_RATE_HZ);
}

/**
 * @brief 查找空闲槽位
 */
static int find_free_slot(void)
{
    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (!s_node_table[i].valid) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 通过 Mesh ID 查找槽位
 */
static int find_by_mesh_id(uint16_t mesh_id)
{
    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (s_node_table[i].valid && s_node_table[i].mesh_id == mesh_id) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 通过 MAC 查找槽位
 */
static int find_by_mac(const uint8_t *mac)
{
    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (s_node_table[i].valid && mac_equal(s_node_table[i].mac, mac)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 通过 IP 查找槽位
 */
static int find_by_ip(const ip4_addr_t *ip)
{
    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (s_node_table[i].valid && 
            ip4_addr_cmp(&s_node_table[i].ip, ip)) {
            return i;
        }
    }
    return -1;
}

/* ============================================================================
 * 公共函数
 * ============================================================================ */

void node_table_init(void)
{
    if (s_initialized) {
        return;
    }

    memset(s_node_table, 0, sizeof(s_node_table));
    
    s_table_mutex = xSemaphoreCreateMutex();
    s_initialized = true;

    tpmesh_debug_printf("NodeTable: Initialized (max %d entries)\n", NODE_TABLE_MAX_ENTRIES);
}

void node_table_clear(void)
{
    if (s_table_mutex) {
        xSemaphoreTake(s_table_mutex, portMAX_DELAY);
    }

    memset(s_node_table, 0, sizeof(s_node_table));

    if (s_table_mutex) {
        xSemaphoreGive(s_table_mutex);
    }
}

int node_table_add_static(const uint8_t *mac, const ip4_addr_t *ip, uint16_t mesh_id)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    /* 检查是否已存在 */
    int idx = find_by_mesh_id(mesh_id);
    if (idx < 0) {
        idx = find_free_slot();
    }

    if (idx < 0) {
        xSemaphoreGive(s_table_mutex);
        tpmesh_debug_printf("NodeTable: Table full\n");
        return -1;
    }

    node_entry_t *entry = &s_node_table[idx];
    entry->valid = 1;
    memcpy(entry->mac, mac, 6);
    ip4_addr_copy(entry->ip, *ip);
    entry->mesh_id = mesh_id;
    entry->last_seen = get_tick_ms();
    entry->source = NODE_SOURCE_STATIC;
    entry->online = 0;

    xSemaphoreGive(s_table_mutex);

    tpmesh_debug_printf("NodeTable: Added static 0x%04X -> %d.%d.%d.%d\n",
           mesh_id, ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
    return 0;
}

int node_table_register(const uint8_t *mac, const ip4_addr_t *ip, uint16_t mesh_id)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    /* 查找已有条目或空闲槽位 */
    int idx = find_by_mesh_id(mesh_id);
    if (idx < 0) {
        idx = find_by_mac(mac);
    }
    if (idx < 0) {
        idx = find_free_slot();
    }

    if (idx < 0) {
        /* 表满,覆盖最老的动态条目 */
        uint32_t oldest_time = 0xFFFFFFFF;
        int oldest_idx = -1;
        for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
            if (s_node_table[i].valid && 
                s_node_table[i].source != NODE_SOURCE_STATIC &&
                s_node_table[i].last_seen < oldest_time) {
                oldest_time = s_node_table[i].last_seen;
                oldest_idx = i;
            }
        }
        idx = oldest_idx;
    }

    if (idx < 0) {
        xSemaphoreGive(s_table_mutex);
        return -1;
    }

    node_entry_t *entry = &s_node_table[idx];
    uint8_t old_valid = entry->valid;
    uint8_t old_mac[6] = {0};
    ip4_addr_t old_ip;
    uint16_t old_mesh_id = 0;
    uint8_t old_source = 0;
    uint8_t old_online = 0;

    if (old_valid) {
        memcpy(old_mac, entry->mac, sizeof(old_mac));
        ip4_addr_copy(old_ip, entry->ip);
        old_mesh_id = entry->mesh_id;
        old_source = entry->source;
        old_online = entry->online;
    }

    entry->valid = 1;
    memcpy(entry->mac, mac, 6);
    ip4_addr_copy(entry->ip, *ip);
    entry->mesh_id = mesh_id;
    entry->last_seen = get_tick_ms();
    entry->source = NODE_SOURCE_REGISTER;
    entry->online = 1;

    bool mapping_changed =
        (!old_valid) ||
        (!mac_equal(old_mac, mac)) ||
        (!ip4_addr_cmp(&old_ip, ip)) ||
        (old_mesh_id != mesh_id) ||
        (old_source != NODE_SOURCE_REGISTER) ||
        (old_online != 1);

    xSemaphoreGive(s_table_mutex);

    if (mapping_changed) {
        tpmesh_debug_printf("NodeTable: Registered 0x%04X MAC=%02X:%02X:%02X:%02X:%02X:%02X IP=%d.%d.%d.%d\n",
               mesh_id,
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
               ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
    }
    return 0;
}

int node_table_learn(const uint8_t *mac, const ip4_addr_t *ip, uint16_t mesh_id)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    /* 检查是否已存在 */
    int idx = find_by_mesh_id(mesh_id);
    if (idx >= 0) {
        /* 已存在,更新时间 */
        s_node_table[idx].last_seen = get_tick_ms();
        s_node_table[idx].online = 1;
        xSemaphoreGive(s_table_mutex);
        return 0;
    }

    /* 新条目 */
    idx = find_free_slot();
    if (idx < 0) {
        xSemaphoreGive(s_table_mutex);
        return -1;
    }

    node_entry_t *entry = &s_node_table[idx];
    entry->valid = 1;
    memcpy(entry->mac, mac, 6);
    ip4_addr_copy(entry->ip, *ip);
    entry->mesh_id = mesh_id;
    entry->last_seen = get_tick_ms();
    entry->source = NODE_SOURCE_LEARNED;
    entry->online = 1;

    xSemaphoreGive(s_table_mutex);

    tpmesh_debug_printf("NodeTable: Learned 0x%04X\n", mesh_id);
    return 0;
}

int node_table_learn_by_mesh(uint16_t mesh_id, const uint8_t *mac)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    if (idx >= 0) {
        /* 更新 MAC */
        memcpy(s_node_table[idx].mac, mac, 6);
        s_node_table[idx].last_seen = get_tick_ms();
        s_node_table[idx].online = 1;
    }

    xSemaphoreGive(s_table_mutex);
    return (idx >= 0) ? 0 : -1;
}

void node_table_touch(uint16_t mesh_id)
{
    if (!s_initialized) return;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    if (idx >= 0) {
        s_node_table[idx].last_seen = get_tick_ms();
        s_node_table[idx].online = 1;
    }

    xSemaphoreGive(s_table_mutex);
}

void node_table_remove(uint16_t mesh_id)
{
    if (!s_initialized) return;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    if (idx >= 0) {
        s_node_table[idx].valid = 0;
    }

    xSemaphoreGive(s_table_mutex);
}

/* ============================================================================
 * 查询函数
 * ============================================================================ */

uint16_t node_table_get_mesh_by_mac(const uint8_t *mac)
{
    if (!s_initialized) return 0xFFFF;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mac(mac);
    uint16_t mesh_id = (idx >= 0) ? s_node_table[idx].mesh_id : 0xFFFF;

    xSemaphoreGive(s_table_mutex);
    return mesh_id;
}

uint16_t node_table_get_mesh_by_ip(const ip4_addr_t *ip)
{
    if (!s_initialized) return 0xFFFF;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_ip(ip);
    uint16_t mesh_id = (idx >= 0) ? s_node_table[idx].mesh_id : 0xFFFF;

    xSemaphoreGive(s_table_mutex);
    return mesh_id;
}

int node_table_get_mac_by_mesh(uint16_t mesh_id, uint8_t *mac)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    if (idx >= 0) {
        memcpy(mac, s_node_table[idx].mac, 6);
    }

    xSemaphoreGive(s_table_mutex);
    return (idx >= 0) ? 0 : -1;
}

int node_table_get_mac_by_ip(const ip4_addr_t *ip, uint8_t *mac)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_ip(ip);
    if (idx >= 0) {
        memcpy(mac, s_node_table[idx].mac, 6);
    }

    xSemaphoreGive(s_table_mutex);
    return (idx >= 0) ? 0 : -1;
}

int node_table_get_ip_by_mesh(uint16_t mesh_id, ip4_addr_t *ip)
{
    if (!s_initialized) return -1;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    if (idx >= 0) {
        ip4_addr_copy(*ip, s_node_table[idx].ip);
    }

    xSemaphoreGive(s_table_mutex);
    return (idx >= 0) ? 0 : -1;
}

const node_entry_t* node_table_get_entry(uint16_t mesh_id)
{
    if (!s_initialized) return NULL;

    /* 注意: 返回指针需要调用者确保线程安全 */
    int idx = find_by_mesh_id(mesh_id);
    return (idx >= 0) ? &s_node_table[idx] : NULL;
}

bool node_table_is_ddc_ip(const ip4_addr_t *ip)
{
    return node_table_get_mesh_by_ip(ip) != 0xFFFF;
}

bool node_table_is_ddc_mac(const uint8_t *mac)
{
    return node_table_get_mesh_by_mac(mac) != 0xFFFF;
}

bool node_table_is_registered(uint16_t mesh_id)
{
    if (!s_initialized) return false;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    bool registered = (idx >= 0) && 
                      (s_node_table[idx].source == NODE_SOURCE_REGISTER);

    xSemaphoreGive(s_table_mutex);
    return registered;
}

bool node_table_is_online(uint16_t mesh_id)
{
    if (!s_initialized) return false;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    int idx = find_by_mesh_id(mesh_id);
    bool online = (idx >= 0) && s_node_table[idx].online;

    xSemaphoreGive(s_table_mutex);
    return online;
}

/* ============================================================================
 * 维护函数
 * ============================================================================ */

void node_table_check_timeout(void)
{
    if (!s_initialized) return;

    uint32_t now = get_tick_ms();

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (!s_node_table[i].valid) continue;
        
        /* 静态节点不超时 */
        if (s_node_table[i].source == NODE_SOURCE_STATIC) continue;

        uint32_t elapsed = now - s_node_table[i].last_seen;
        if (elapsed > NODE_TABLE_TIMEOUT_MS) {
            if (s_node_table[i].online) {
                tpmesh_debug_printf("NodeTable: Node 0x%04X offline (timeout)\n",
                       s_node_table[i].mesh_id);
                s_node_table[i].online = 0;
            }
            
            /* 可选: 完全删除 */
            /* s_node_table[i].valid = 0; */
        }
    }

    xSemaphoreGive(s_table_mutex);
}

void node_table_dump(void)
{
    if (!s_initialized) {
        tpmesh_debug_printf("NodeTable: Not initialized\n");
        return;
    }

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    tpmesh_debug_printf("\n--- Node Table ---\n");
    tpmesh_debug_printf("%-6s %-18s %-16s %-8s %-8s\n", 
           "Mesh", "MAC", "IP", "Source", "Online");

    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (!s_node_table[i].valid) continue;

        const node_entry_t *e = &s_node_table[i];
        const char *src_str[] = {"Static", "Learned", "Register"};

        tpmesh_debug_printf("0x%04X %02X:%02X:%02X:%02X:%02X:%02X %3d.%3d.%3d.%3d %-8s %s\n",
               e->mesh_id,
               e->mac[0], e->mac[1], e->mac[2], e->mac[3], e->mac[4], e->mac[5],
               ip4_addr1(&e->ip), ip4_addr2(&e->ip), 
               ip4_addr3(&e->ip), ip4_addr4(&e->ip),
               src_str[e->source],
               e->online ? "Yes" : "No");
    }
    tpmesh_debug_printf("------------------\n\n");

    xSemaphoreGive(s_table_mutex);
}

uint16_t node_table_count(void)
{
    if (!s_initialized) return 0;

    uint16_t count = 0;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (s_node_table[i].valid) {
            count++;
        }
    }

    xSemaphoreGive(s_table_mutex);
    return count;
}

void node_table_foreach(bool (*callback)(const node_entry_t *entry, void *arg), void *arg)
{
    if (!s_initialized || !callback) return;

    xSemaphoreTake(s_table_mutex, portMAX_DELAY);

    for (int i = 0; i < NODE_TABLE_MAX_ENTRIES; i++) {
        if (s_node_table[i].valid) {
            if (!callback(&s_node_table[i], arg)) {
                break;
            }
        }
    }

    xSemaphoreGive(s_table_mutex);
}
