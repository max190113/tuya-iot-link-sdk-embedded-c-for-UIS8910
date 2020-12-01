#ifndef __TUYA_MQTT_API_H_
#define __TUYA_MQTT_API_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "tuya_cloud_types.h"
#include "tuya_error_code.h"

#include "mqtt_service.h"
#include "atop_service.h"
#include "cJSON.h"

/**
 * @brief SDK Version info
 * 
 */
#define BS_VERSION "40.07"
#define PV_VERSION "2.2"

/**
 * @brief Fields length
 * 
 */
#define MAX_LENGTH_PRODUCT_ID  16
#define MAX_LENGTH_UUID        25
#define MAX_LENGTH_AUTHKEY     32
#define MAX_LENGTH_DEVICE_ID   25
#define MAX_LENGTH_SECKEY      16
#define MAX_LENGTH_LOCALKEY    16
#define MAX_LENGTH_SCHEMA_ID   16
#define MAX_LENGTH_TOKEN       10
#define MAX_LENGTH_SW_VER      10   // max string length of VERSION

/* tuya sdk gateway reset type */
typedef enum {
    TUYA_EVENT_RESET,
    TUYA_EVENT_BIND_START,
    TUYA_EVENT_BIND_TOKEN_ON,
    TUYA_EVENT_ACTIVATE_SUCCESSED,
    TUYA_EVENT_MQTT_CONNECTED,
    TUYA_EVENT_MQTT_DISCONNECT,
    TUYA_EVENT_DP_RECEIVE,
    TUYA_EVENT_DP_RECEIVE_CJSON,
    TUYA_EVENT_UPGRADE_NOTIFY,
} tuya_event_id_t;

#define EVENT_ID2STR(S)\
((S) == TUYA_EVENT_RESET ? "TUYA_EVENT_RESET":\
((S) == TUYA_EVENT_BIND_START ? "TUYA_EVENT_BIND_START":\
((S) == TUYA_EVENT_BIND_TOKEN_ON ? "TUYA_EVENT_BIND_TOKEN_ON":\
((S) == TUYA_EVENT_ACTIVATE_SUCCESSED ? "TUYA_EVENT_ACTIVATE_SUCCESSED":\
((S) == TUYA_EVENT_MQTT_CONNECTED ? "TUYA_EVENT_MQTT_CONNECTED":\
((S) == TUYA_EVENT_MQTT_DISCONNECT ? "TUYA_EVENT_MQTT_DISCONNECT":\
((S) == TUYA_EVENT_DP_RECEIVE ? "TUYA_EVENT_DP_RECEIVE":\
((S) == TUYA_EVENT_DP_RECEIVE_CJSON ? "TUYA_EVENT_DP_RECEIVE_CJSON":\
((S) == TUYA_EVENT_UPGRADE_NOTIFY ? "TUYA_EVENT_UPGRADE_NOTIFY":\
"Unknown")))))))))

typedef enum {
    TUYA_STATUS_UNACTIVE = 0,
    TUYA_STATUS_NETCFG_IDLE = 1,
    TUYA_STATUS_UNCONNECT_ROUTER = 2,
    TUYA_STATUS_WIFI_CONNECTED = 3,
    TUYA_STATUS_MQTT_CONNECTED = 4,
} tuya_client_status_t;

typedef enum {
    GW_LOCAL_RESET_FACTORY = 0,
    GW_REMOTE_UNACTIVE, //解除绑定
    GW_LOCAL_UNACTIVE,
    GW_REMOTE_RESET_FACTORY, //解除并清除数据
    GW_RESET_DATA_FACTORY, //need clear local data when active
} tuya_reset_type_t;

typedef enum {
    TUYA_DATE_TYPE_UNDEFINED,
    TUYA_DATE_TYPE_BOOLEAN,
    TUYA_DATE_TYPE_INTEGER,
    TUYA_DATE_TYPE_STRING,
    TUYA_DATE_TYPE_RAW,
    TUYA_DATE_TYPE_JSON
} tuya_data_type_t;

typedef union {
    char *      asString;
    bool        asBoolean;
    int32_t     asInteger;
    cJSON *     asJSON;
    struct {
        uint8_t * buffer;
        uint32_t  length;
    } asBuffer;
} tuya_data_value_t;

typedef struct {
    tuya_event_id_t id;
    tuya_data_type_t type;
    tuya_data_value_t value;
} tuya_event_msg_t;

typedef struct tuya_iot_client_handle tuya_iot_client_t;

typedef void (*event_handle_cb_t)(tuya_iot_client_t* client, tuya_event_msg_t* event);

typedef struct {
    char devid[MAX_LENGTH_DEVICE_ID + 1];
    char seckey[MAX_LENGTH_SECKEY + 1];
    char localkey[MAX_LENGTH_LOCALKEY + 1];
    char schemaId[MAX_LENGTH_SCHEMA_ID + 1];
    bool resetFactory;
    int capability;
} activated_params_t;

typedef struct {
    const char* productkey;
    const char* uuid;
    const char* authkey;
    const char* software_ver;
    event_handle_cb_t event_handler;
} tuya_iot_config_t;

typedef int (*tuya_activate_token_get_t)(const tuya_iot_config_t* config, char* token_out);

struct tuya_iot_client_handle {
    tuya_iot_config_t config;
    uint8_t state;
    uint8_t retry_count;
    tuya_event_msg_t event;
    activated_params_t activate;
    tuya_mqtt_context_t mqctx;
    tuya_activate_token_get_t token_get;
    char token[MAX_LENGTH_TOKEN];
};

/**
 * @brief Initialize the Tuya client implementation
 * 
 * @param client - The context to initialize.
 * @param config - defines the properties of the Tuya connection.
 * @return int - OPRT_OK successful or error code.
 */
int tuya_iot_init(tuya_iot_client_t* client, const tuya_iot_config_t* config);

/**
 * @brief Start Tuya client cloud service. 
 * 
 * @param client - The Tuya client context.
 * @return int - OPRT_OK successful or error code.
 */
int tuya_iot_start(tuya_iot_client_t *client);

/**
 * @brief Stop Tuya client cloud service. 
 * 
 * @param client - The Tuya client context.
 * @return int - OPRT_OK successful or error code.
 */
int tuya_iot_stop(tuya_iot_client_t *client);

/**
 * @brief Reset the Tuya client.
 * 
 * @param client - The Tuya client context.
 * @return int - OPRT_OK successful or error code.
 */
int tuya_iot_reset(tuya_iot_client_t *client);

/**
 * @brief Destroy the Tuya client and release resources.
 * 
 * @param client - The Tuya client context.
 * @return int - OPRT_OK successful or error code.
 */
int tuya_iot_destroy(tuya_iot_client_t* client);

/**
 * @brief Loop called to yield the current thread to the underlying Tuya client.
 * 
 * @param client - The Tuya client context.
 * @return int - OPRT_OK successful or error code. 
 */
int tuya_iot_yield(tuya_iot_client_t* client);

/**
 * @brief Report Tuya data point(DP) services to the cloud.
 * 
 * @param client - The Tuya client context.
 * @param dps - DP JSON format e.g: "{"101":true}"
 * @return int - OPRT_OK successful or error code. 
 */
int tuya_iot_dp_report_json(tuya_iot_client_t* client, const char* dps);

/**
 * @brief Is Tuya client has been activated?
 * 
 * @param client - The Tuya client context.
 * @return true activated
 * @return false inactivated.
 */
bool tuya_iot_activated(tuya_iot_client_t* client);

/**
 * @brief Set up a customized get token interface.
 * 
 * @param client - The Tuya client context.
 * @param token_get_func - token get func
 * @return int - OPRT_OK successful or error code. 
 */
int tuya_iot_token_get_port_register(tuya_iot_client_t* client, tuya_activate_token_get_t token_get_func);

/**
 * @brief Synchronously update the client software version information.
 * 
 * @param client - The Tuya client context.
 * @return int - OPRT_OK successful or error code. 
 */
int tuya_iot_version_update_sync(tuya_iot_client_t* client);

#ifdef __cplusplus
}
#endif
#endif