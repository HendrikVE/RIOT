#include <stdio.h>
#include <string.h>

#include "periph/flashpage.h"

#include "config.h"

static uint8_t page_mem[FLASHPAGE_SIZE] ALIGNMENT_ATTR;

static int read_page(int page, uint8_t buffer[]);
static int write_page(int page, uint8_t buffer[]);

void config_read(struct config_values* config)
{
    read_page(FLASHPAGE_NUM_CONFIG, page_mem);

    int size_data_field;
    int offset = 0;
    void *p;

    size_data_field = sizeof(config->device_room) / sizeof(config->device_room[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->device_room, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->device_id) / sizeof(config->device_id[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->device_id, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_host) / sizeof(config->ota_host[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->ota_host, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_filename) / sizeof(config->ota_filename[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->ota_filename, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_server_username) / sizeof(config->ota_server_username[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->ota_server_username, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_server_password) / sizeof(config->ota_server_password[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->ota_server_password, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->wifi_ssid) / sizeof(config->wifi_ssid[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->wifi_ssid, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->wifi_password) / sizeof(config->wifi_password[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->wifi_password, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_user) / sizeof(config->mqtt_user[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->mqtt_user, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_password) / sizeof(config->mqtt_password[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->mqtt_password, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_server_ip) / sizeof(config->mqtt_server_ip[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->mqtt_server_ip, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_server_port) / sizeof(config->mqtt_server_port[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->mqtt_server_port, p, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->sensor_poll_interval_ms) / sizeof(config->sensor_poll_interval_ms[0]);
    p = &page_mem[size_data_field - 1] + offset  - (size_data_field - 1);
    memcpy(config->sensor_poll_interval_ms, p, size_data_field);
    offset += size_data_field;
}

void config_store(struct config_values* config)
{
    int size_data_field;
    int offset = 0;
    void *p;

    size_data_field = sizeof(config->device_room) / sizeof(config->device_room[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->device_room, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->device_id) / sizeof(config->device_id[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->device_id, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_host) / sizeof(config->ota_host[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->ota_host, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_filename) / sizeof(config->ota_filename[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->ota_filename, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_server_username) / sizeof(config->ota_server_username[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->ota_server_username, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->ota_server_password) / sizeof(config->ota_server_password[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->ota_server_password, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->wifi_ssid) / sizeof(config->wifi_ssid[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->wifi_ssid, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->wifi_password) / sizeof(config->wifi_password[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->wifi_password, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_user) / sizeof(config->mqtt_user[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->mqtt_user, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_password) / sizeof(config->mqtt_password[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->mqtt_password, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_server_ip) / sizeof(config->mqtt_server_ip[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->mqtt_server_ip, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->mqtt_server_port) / sizeof(config->mqtt_server_port[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->mqtt_server_port, size_data_field);
    offset += size_data_field;

    size_data_field = sizeof(config->sensor_poll_interval_ms) / sizeof(config->sensor_poll_interval_ms[0]);
    p = &page_mem[size_data_field - 1] + offset - (size_data_field - 1);
    memcpy(p, config->sensor_poll_interval_ms, size_data_field);
    offset += size_data_field;

    write_page(FLASHPAGE_NUM_CONFIG, page_mem);
}

static int read_page(int page, uint8_t buffer[])
{
    if ((page >= (int)FLASHPAGE_NUMOF) || (page < 0)) {
        printf("error: page %i is invalid\n", page);
        return 1;
    }

    flashpage_read(page, buffer);
    printf("Read flash page %i into local page buffer\n", page);

    return 0;
}

static int write_page(int page, uint8_t buffer[])
{
    if ((page >= (int)FLASHPAGE_NUMOF) || (page < 0)) {
        printf("error: page %i is invalid\n", page);
        return 1;
    }

    if (flashpage_write_and_verify(page, buffer) != FLASHPAGE_OK) {
        printf("error: verification for page %i failed\n", page);
        return 1;
    }

    printf("wrote local page buffer to flash page %i at addr %p\n",
           page, flashpage_addr(page));

    return 0;
}
