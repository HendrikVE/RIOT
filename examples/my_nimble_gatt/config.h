#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>

#define ALIGNMENT_ATTR
#define FLASHPAGE_NUM_CONFIG FLASHPAGE_NUMOF - 1

static_assert(FLASHPAGE_SIZE >= 4096, "condition not met: FLASHPAGE_SIZE >= 4096");
struct config_values {
    uint8_t device_room[256];
    uint8_t device_id[256];
    uint8_t ota_host[256];
    uint8_t ota_filename[256];
    uint8_t ota_server_username[256];
    uint8_t ota_server_password[256];
    uint8_t wifi_ssid[256];
    uint8_t wifi_password[256];
    uint8_t mqtt_user[256];
    uint8_t mqtt_password[256];
    uint8_t mqtt_server_ip[256];
    uint8_t mqtt_server_port[32];
    uint8_t sensor_poll_interval_ms[32];
};

void config_read(struct config_values* config);
void config_store(struct config_values* config);

#endif /*CONFIG_H*/
