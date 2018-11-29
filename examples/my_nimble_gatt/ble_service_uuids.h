#include "host/ble_uuid.h"

#define KEY_CONFIG_DEVICE_ROOM              "DEVICE_ROOM"
#define KEY_CONFIG_DEVICE_ID                "DEVICE_ID"
#define KEY_CONFIG_OTA_HOST                 "OTA_HOST"
#define KEY_CONFIG_OTA_FILENAME             "OTA_FILENAME"
#define KEY_CONFIG_OTA_SERVER_USERNAME      "OTA_USERNAME"
#define KEY_CONFIG_OTA_SERVER_PASSWORD      "OTA_PASSWORD"
#define KEY_CONFIG_WIFI_SSID                "WIFI_SSID"
#define KEY_CONFIG_WIFI_PASSWORD            "WIFI_PASSWORD"
#define KEY_CONFIG_MQTT_USER                "MQTT_USER"
#define KEY_CONFIG_MQTT_PASSWORD            "MQTT_PASSWORD"
#define KEY_CONFIG_MQTT_SERVER_IP           "MQTT_IP"
#define KEY_CONFIG_MQTT_SERVER_PORT         "MQTT_PORT"
#define KEY_CONFIG_SENSOR_POLL_INTERVAL_MS  "POLL_INTERVAL"

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

/* CONFIG SERVICE UUID */
/* UUID = 2fa1dab8-3eef-40fc-8540-7fc496a10d75 */
static const ble_uuid128_t gatt_svr_svc_config_uuid
        = BLE_UUID128_INIT(0x75, 0x0d, 0xa1, 0x96, 0xc4, 0x7f, 0x40, 0x85, 0xfc,
                           0x40, 0xef, 0x3e, 0xb8, 0xda, 0xa1, 0x2f);

/* CONFIG DEVICE */
/* UUID = d3491796-683b-4b9c-aafb-f51a35459d43 */
static const ble_uuid128_t gatt_svr_chr_config_device_room_uuid
        = BLE_UUID128_INIT(0x43, 0x9d, 0x45, 0x35, 0x1a, 0xf5, 0xfb, 0xaa, 0x9c,
                           0x4b, 0x3b, 0x68, 0x96, 0x17, 0x49, 0xd3);

/* UUID = 4745e11f-b403-4cfb-83bb-710d46897875 */
static const ble_uuid128_t gatt_svr_chr_config_device_id_uuid
        = BLE_UUID128_INIT(0x75, 0x78, 0x89, 0x46, 0x0d, 0x71, 0xbb, 0x83, 0xfb,
                           0x4c, 0x03, 0xb4, 0x1f, 0xe1, 0x45, 0x47);

/* CONFIG OTA */
/* UUID = 2f44b103-444c-48f5-bf60-91b81dfa0a25 */
static const ble_uuid128_t gatt_svr_chr_config_ota_host_uuid
        = BLE_UUID128_INIT(0x25, 0x0a, 0xfa, 0x1d, 0xb8, 0x91, 0x60, 0xbf, 0xf5,
                           0x48, 0x4c, 0x44, 0x03, 0xb1, 0x44, 0x2f);

/* UUID = 4b95d245-db08-4c56-98f9-738faa8cfbb6 */
static const ble_uuid128_t gatt_svr_chr_config_ota_filename_uuid
        = BLE_UUID128_INIT(0xb6, 0xfb, 0x8c, 0xaa, 0x8f, 0x73, 0xf9, 0x98, 0x56,
                           0x4c, 0x08, 0xdb, 0x45, 0xd2, 0x95, 0x4b);

/* UUID = 1c93dce2-3796-4027-9f55-6d251c41dd34 */
static const ble_uuid128_t gatt_svr_chr_config_ota_server_username_uuid
        = BLE_UUID128_INIT(0x34, 0xdd, 0x41, 0x1c, 0x25, 0x6d, 0x55, 0x9f, 0x27,
                           0x40, 0x96, 0x37, 0xe2, 0xdc, 0x93, 0x1c);

/* UUID = 0e837309-5336-45a3-9b69-d0f7134f30ff */
static const ble_uuid128_t gatt_svr_chr_config_ota_server_password_uuid
        = BLE_UUID128_INIT(0xff, 0x30, 0x4f, 0x13, 0xf7, 0xd0, 0x69, 0x9b, 0xa3,
                           0x45, 0x36, 0x53, 0x09, 0x73, 0x83, 0x0e);

/* CONFIG WiFi */
/* UUID = 8ca0bf1d-bb5d-4a66-9191-341fd805e288 */
static const ble_uuid128_t gatt_svr_chr_config_wifi_ssid_uuid
        = BLE_UUID128_INIT(0x88, 0xe2, 0x05, 0xd8, 0x1f, 0x34, 0x91, 0x91, 0x66,
                           0x4a, 0x5d, 0xbb, 0x1d, 0xbf, 0xa0, 0x8c);

/* UUID = fa41c195-ae99-422e-8f1f-0730702b3fc5 */
static const ble_uuid128_t gatt_svr_chr_config_wifi_password_uuid
        = BLE_UUID128_INIT(0xc5, 0x3f, 0x2b, 0x70, 0x30, 0x07, 0x1f, 0x8f, 0x2e,
                           0x42, 0x99, 0xae, 0x95, 0xc1, 0x41, 0xfa);

/* CONFIG MQTT */
/* UUID = 69150609-18f8-4523-a41f-6d9a01d2e08d */
static const ble_uuid128_t gatt_svr_chr_config_mqtt_user_uuid
        = BLE_UUID128_INIT(0x8d, 0xe0, 0xd2, 0x01, 0x9a, 0x6d, 0x1f, 0xa4, 0x23,
                           0x45, 0xf8, 0x18, 0x09, 0x06, 0x15, 0x69);

/* UUID = 8bebec77-ea21-4c14-9d64-dbec1fd5267c */
static const ble_uuid128_t gatt_svr_chr_config_mqtt_password_uuid
        = BLE_UUID128_INIT(0x7c, 0x26, 0xd5, 0x1f, 0xec, 0xdb, 0x64, 0x9d, 0x14,
                           0x4c, 0x21, 0xea, 0x77, 0xec, 0xeb, 0x8b);

/* UUID = e3b150fb-90a2-4cd3-80c5-b1189e110ef1 */
static const ble_uuid128_t gatt_svr_chr_config_mqtt_server_ip_uuid
        = BLE_UUID128_INIT(0xf1, 0x0e, 0x11, 0x9e, 0x18, 0xb1, 0xc5, 0x80, 0xd3,
                           0x4c, 0xa2, 0x90, 0xfb, 0x50, 0xb1, 0xe3);

/* UUID = 4eeff953-0f5e-43ee-b1be-1783a8190b0d */
static const ble_uuid128_t gatt_svr_chr_config_mqtt_server_port_uuid
        = BLE_UUID128_INIT(0x0d, 0x0b, 0x19, 0xa8, 0x83, 0x17, 0xbe, 0xb1, 0xee,
                           0x43, 0x5e, 0x0f, 0x53, 0xf9, 0xef, 0x4e);

/* CONFIG SENSOR */
/* UUID = 68011c92-854a-4f2c-a94c-5ee37dc607c3 */
static const ble_uuid128_t gatt_svr_chr_config_sensor_poll_interval_ms_uuid
        = BLE_UUID128_INIT(0xc3, 0x07, 0xc6, 0x7d, 0xe3, 0x5e, 0x4c, 0xa9, 0x2c,
                           0x4f, 0x4a, 0x85, 0x92, 0x1c, 0x01, 0x68);

/* RESTART */
/* UUID = 890f7b6f-cecc-4e3e-ade2-5f2907867f4b */
static const ble_uuid128_t gatt_svr_chr_config_restart_uuid
        = BLE_UUID128_INIT(0x4b, 0x7f, 0x86, 0x07, 0x29, 0x5f, 0xe2, 0xad, 0x3e,
                           0x4e, 0xcc, 0xce, 0x6f, 0x7b, 0x0f, 0x89);

static int gatt_svr_chr_access_device_info(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

/* define several bluetooth services for our device */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /*
     * access_cb defines a callback for read and write access events on
     * given characteristics
     */
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: * Manufacturer name */
            .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
            .access_cb = gatt_svr_chr_access_device_info,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            /* Characteristic: Model number string */
            .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
            .access_cb = gatt_svr_chr_access_device_info,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
    {
        /* Service: Config */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (ble_uuid_t*) &gatt_svr_svc_config_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_device_room_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_device_id_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_ota_host_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_ota_filename_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_ota_server_username_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_ota_server_password_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_wifi_ssid_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_wifi_password_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_mqtt_user_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_mqtt_password_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_mqtt_server_ip_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_mqtt_server_port_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_config_sensor_poll_interval_ms_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            0, /* No more characteristics in this service */
        }, }
    },
    {
        0, /* No more services */
    },
};

struct Key1 {
    const ble_uuid128_t* uuid;
};

struct Value1 {
    char* config_key;
};

struct Map1 {
    struct Key1 key;
    struct Value1 value;
};

struct Map1 characteristic_config_mapping[] = {
    {{&gatt_svr_chr_config_device_room_uuid}, {KEY_CONFIG_DEVICE_ROOM}},
    {{&gatt_svr_chr_config_device_id_uuid}, {KEY_CONFIG_DEVICE_ID}},
    {{&gatt_svr_chr_config_ota_host_uuid}, {KEY_CONFIG_OTA_HOST}},
    {{&gatt_svr_chr_config_ota_filename_uuid}, {KEY_CONFIG_OTA_FILENAME}},
    {{&gatt_svr_chr_config_ota_server_username_uuid}, {KEY_CONFIG_OTA_SERVER_USERNAME}},
    {{&gatt_svr_chr_config_ota_server_password_uuid}, {KEY_CONFIG_OTA_SERVER_PASSWORD}},
    {{&gatt_svr_chr_config_wifi_ssid_uuid}, {KEY_CONFIG_WIFI_SSID}},
    {{&gatt_svr_chr_config_wifi_password_uuid}, {KEY_CONFIG_WIFI_PASSWORD}},
    {{&gatt_svr_chr_config_mqtt_user_uuid}, {KEY_CONFIG_MQTT_USER}},
    {{&gatt_svr_chr_config_mqtt_password_uuid}, {KEY_CONFIG_MQTT_PASSWORD}},
    {{&gatt_svr_chr_config_mqtt_server_ip_uuid}, {KEY_CONFIG_MQTT_SERVER_IP}},
    {{&gatt_svr_chr_config_mqtt_server_port_uuid}, {KEY_CONFIG_MQTT_SERVER_PORT}},
    {{&gatt_svr_chr_config_sensor_poll_interval_ms_uuid}, {KEY_CONFIG_SENSOR_POLL_INTERVAL_MS}},
};

#define CONFIG_DEVICE_ROOM "CONFIG_DEVICE_ROOM"
#define CONFIG_DEVICE_ID "CONFIG_DEVICE_ID"
#define CONFIG_OTA_HOST "CONFIG_OTA_HOST"
#define CONFIG_OTA_FILENAME "CONFIG_OTA_FILENAME"
#define CONFIG_OTA_SERVER_USERNAME "CONFIG_OTA_SERVER_USERNAME"
#define CONFIG_OTA_SERVER_PASSWORD "CONFIG_OTA_SERVER_PASSWORD"
#define CONFIG_ESP_WIFI_SSID "CONFIG_ESP_WIFI_SSID"
#define CONFIG_ESP_WIFI_PASSWORD "CONFIG_ESP_WIFI_PASSWORD"
#define CONFIG_MQTT_USER "CONFIG_MQTT_USER"
#define CONFIG_MQTT_PASSWORD "CONFIG_MQTT_PASSWORD"
#define CONFIG_MQTT_SERVER_IP "CONFIG_MQTT_SERVER_IP"
#define CONFIG_MQTT_SERVER_PORT "CONFIG_MQTT_SERVER_PORT"
#define CONFIG_SENSOR_POLL_INTERVAL_MS "CONFIG_SENSOR_POLL_INTERVAL_MS"

struct Key2 {
    char* config_key;
};

struct Value2 {
    char* value;
};

struct Map2 {
    struct Key2 key;
    struct Value2 value;
};

struct Map2 config[] = {
        {{KEY_CONFIG_DEVICE_ROOM}, {KEY_CONFIG_DEVICE_ROOM}},
        {{KEY_CONFIG_DEVICE_ID}, {KEY_CONFIG_DEVICE_ID}},
        {{KEY_CONFIG_OTA_HOST}, {KEY_CONFIG_OTA_HOST}},
        {{KEY_CONFIG_OTA_FILENAME}, {KEY_CONFIG_OTA_FILENAME}},
        {{KEY_CONFIG_OTA_SERVER_USERNAME}, {KEY_CONFIG_OTA_SERVER_USERNAME}},
        {{KEY_CONFIG_OTA_SERVER_PASSWORD}, {KEY_CONFIG_OTA_SERVER_PASSWORD}},
        {{KEY_CONFIG_WIFI_SSID}, {KEY_CONFIG_WIFI_SSID}},
        {{KEY_CONFIG_WIFI_PASSWORD}, {KEY_CONFIG_WIFI_PASSWORD}},
        {{KEY_CONFIG_MQTT_USER}, {KEY_CONFIG_MQTT_USER}},
        {{KEY_CONFIG_MQTT_PASSWORD}, {KEY_CONFIG_MQTT_PASSWORD}},
        {{KEY_CONFIG_MQTT_SERVER_IP}, {KEY_CONFIG_MQTT_SERVER_IP}},
        {{KEY_CONFIG_MQTT_SERVER_PORT}, {KEY_CONFIG_MQTT_SERVER_PORT}},
        {{KEY_CONFIG_SENSOR_POLL_INTERVAL_MS}, {KEY_CONFIG_SENSOR_POLL_INTERVAL_MS}},
};
