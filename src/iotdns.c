#include <stdio.h>
#include "tuya_endpoint.h"
#include "tuya_log.h"
#include "cJSON.h"
#include "base64.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "storage_interface.h"
#include "http_client_interface.h"

#define IOTDNS_REQUEST_FMT "{\"config\":[{\"key\":\"httpsSelfUrl\",\"need_ca\":true},{\"key\":\"mqttsSelfUrl\",\"need_ca\":true}],\"region\":\"%s\",\"env\":\"%s\"}"

const uint8_t iot_dns_cert_der[] = {
    0x30, 0x82, 0x03, 0x0e, 0x30, 0x82, 0x01, 0xf6, 0x02, 0x09, 0x00, 0xc5, 0x3e, 0x9d, 0xe4, 0xdc,
    0x37, 0xae, 0x68, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
    0x05, 0x00, 0x30, 0x48, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
    0x53, 0x31, 0x18, 0x30, 0x16, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0f, 0x48, 0x54, 0x54, 0x50,
    0x20, 0x44, 0x4e, 0x53, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x31, 0x1f, 0x30, 0x1d, 0x06,
    0x03, 0x55, 0x04, 0x0b, 0x0c, 0x16, 0x49, 0x4f, 0x54, 0x20, 0x44, 0x4e, 0x53, 0x20, 0x50, 0x75,
    0x62, 0x6c, 0x69, 0x63, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x30, 0x20, 0x17, 0x0d,
    0x31, 0x39, 0x30, 0x35, 0x32, 0x35, 0x30, 0x37, 0x35, 0x35, 0x35, 0x37, 0x5a, 0x18, 0x0f, 0x32,
    0x31, 0x31, 0x39, 0x30, 0x35, 0x30, 0x31, 0x30, 0x37, 0x35, 0x35, 0x35, 0x37, 0x5a, 0x30, 0x48,
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x18, 0x30,
    0x16, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0f, 0x48, 0x54, 0x54, 0x50, 0x20, 0x44, 0x4e, 0x53,
    0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x31, 0x1f, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x0c, 0x16, 0x49, 0x4f, 0x54, 0x20, 0x44, 0x4e, 0x53, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63,
    0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09,
    0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
    0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xe4, 0x98, 0xdd, 0x80, 0x21, 0xb0, 0xee,
    0x24, 0x5d, 0xf5, 0xbd, 0x9f, 0x5a, 0x5f, 0x0a, 0xd8, 0x48, 0x6b, 0x16, 0x46, 0x33, 0x71, 0xdb,
    0x5b, 0x41, 0xb8, 0x77, 0x87, 0xc6, 0x69, 0xc4, 0xf8, 0x08, 0x53, 0x1a, 0xc8, 0xce, 0xa4, 0x39,
    0xa2, 0xa6, 0x3b, 0x5e, 0x98, 0xba, 0x09, 0x0a, 0xef, 0x61, 0x7c, 0x81, 0x99, 0x56, 0x6e, 0xe0,
    0xde, 0x43, 0x2e, 0x18, 0xc5, 0x7d, 0x90, 0xd1, 0x57, 0x19, 0x39, 0x43, 0xa7, 0x04, 0x1c, 0x06,
    0x77, 0x5a, 0x10, 0xc2, 0xb7, 0xa8, 0x27, 0xdc, 0xf3, 0x64, 0x0f, 0x32, 0x70, 0x14, 0x38, 0x05,
    0xe9, 0xb7, 0x8d, 0xf7, 0x27, 0x96, 0x3e, 0x39, 0xf4, 0x1e, 0x48, 0xb9, 0x7b, 0xc2, 0x01, 0xb7,
    0x8b, 0xcb, 0x1f, 0xb4, 0x6b, 0xd1, 0x14, 0x5e, 0x23, 0x89, 0xfb, 0x09, 0x0c, 0x28, 0x84, 0x1b,
    0x37, 0x04, 0xcf, 0x89, 0xfa, 0x85, 0x4c, 0xd4, 0xa9, 0x44, 0xaf, 0x83, 0x5d, 0x1b, 0xe4, 0x21,
    0x5c, 0xa0, 0xd5, 0x07, 0xd1, 0x7f, 0x7a, 0x48, 0xd5, 0x6e, 0xe0, 0x26, 0x99, 0x34, 0x78, 0x5f,
    0xfd, 0x14, 0x78, 0xef, 0x84, 0xe8, 0x35, 0xd0, 0x16, 0x56, 0xa4, 0x01, 0x47, 0x4a, 0xf7, 0x43,
    0x71, 0xe9, 0x0c, 0xfe, 0xbd, 0x0a, 0xd2, 0xbe, 0x9c, 0x2a, 0x6e, 0xdf, 0xde, 0x6a, 0x3f, 0xc8,
    0x86, 0x1d, 0x4e, 0xf2, 0x6c, 0x8d, 0x85, 0xa0, 0x10, 0xdf, 0x2d, 0x22, 0x74, 0x33, 0xd4, 0x9a,
    0xf9, 0x52, 0xfb, 0x3d, 0x0d, 0xf6, 0xda, 0xe0, 0xf1, 0x33, 0xcc, 0x8b, 0x56, 0x21, 0x08, 0x99,
    0x96, 0x42, 0x61, 0x8e, 0xda, 0xed, 0xcc, 0x57, 0x6b, 0xc6, 0x7e, 0xa8, 0x96, 0x98, 0xad, 0x26,
    0x21, 0x14, 0x31, 0xac, 0x78, 0x9b, 0x4b, 0x64, 0xa5, 0x0e, 0x12, 0x19, 0x80, 0x45, 0x90, 0x15,
    0xf5, 0xf6, 0xfe, 0x4b, 0x6c, 0x85, 0x47, 0x7a, 0xbb, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30, 0x0d,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x82, 0x01,
    0x01, 0x00, 0xae, 0x50, 0x2e, 0x46, 0x49, 0xd5, 0xaf, 0x80, 0x26, 0xba, 0x80, 0xbe, 0xc3, 0x06,
    0xf2, 0x4e, 0x91, 0x79, 0xa2, 0x89, 0x9d, 0x93, 0xe8, 0x34, 0x80, 0xb7, 0xea, 0x4d, 0x0c, 0x60,
    0x43, 0x11, 0x33, 0x7a, 0x9d, 0x5d, 0x7f, 0x53, 0x82, 0xd4, 0x55, 0x74, 0xde, 0x83, 0xa0, 0x6b,
    0x52, 0xee, 0x15, 0x42, 0x53, 0xd0, 0x77, 0x99, 0x03, 0xb5, 0x14, 0xae, 0xd4, 0x1f, 0xe8, 0x08,
    0xba, 0x61, 0x09, 0x4d, 0xd4, 0x1a, 0x4e, 0x6f, 0xfa, 0x89, 0xa0, 0x9e, 0xcc, 0x16, 0x04, 0x39,
    0x28, 0xb5, 0xf7, 0xc1, 0xb3, 0x8c, 0xce, 0x59, 0x52, 0x62, 0x9c, 0x94, 0x6c, 0xe3, 0x1e, 0xa3,
    0x39, 0x55, 0xa7, 0xe5, 0x66, 0x18, 0xf5, 0x4d, 0xaf, 0xee, 0xfb, 0x64, 0xf1, 0xfb, 0x56, 0x58,
    0x2c, 0x11, 0x65, 0xc2, 0xce, 0x8f, 0xcc, 0xa3, 0xe2, 0x16, 0x80, 0x79, 0x60, 0x86, 0x52, 0xc6,
    0x21, 0xc9, 0x06, 0x86, 0x43, 0xf2, 0x12, 0xaf, 0x8f, 0xb0, 0xfa, 0x4b, 0x00, 0x20, 0x41, 0x3d,
    0xcc, 0x38, 0x5e, 0x6c, 0xbe, 0xa4, 0xa9, 0xf4, 0x3c, 0xa8, 0x20, 0x5b, 0x44, 0x1c, 0x56, 0xd6,
    0x44, 0x02, 0xc0, 0xcf, 0xe1, 0x9d, 0x11, 0x84, 0x0d, 0xc9, 0x34, 0x00, 0x99, 0xe8, 0xe0, 0xdd,
    0x09, 0xff, 0xf8, 0x7a, 0xa1, 0x46, 0x91, 0x14, 0xc6, 0xb2, 0xbb, 0x31, 0xf9, 0x6a, 0x6d, 0xa9,
    0x1f, 0x81, 0x74, 0xbc, 0x39, 0x4f, 0x58, 0x03, 0x72, 0x39, 0x0b, 0xe2, 0x2c, 0x93, 0x9b, 0xce,
    0xcf, 0x92, 0x19, 0xb0, 0x82, 0x3e, 0x30, 0x90, 0xd3, 0xee, 0x70, 0xea, 0xcb, 0xf0, 0x8e, 0x90,
    0x99, 0x28, 0x26, 0x5b, 0xe0, 0x9e, 0xfc, 0xac, 0xca, 0xa5, 0x65, 0xb7, 0xf4, 0x5a, 0x4b, 0x46,
    0x46, 0xc1, 0x36, 0xb1, 0x98, 0xca, 0x1c, 0xf2, 0x73, 0x88, 0xcc, 0x95, 0x23, 0xf4, 0xd7, 0x94,
    0xa2, 0xb5
};

static int iotdns_response_decode(const uint8_t* input, size_t ilen, tuya_endpoint_t* endport)
{
    cJSON* root = cJSON_Parse(input);
    if (root == NULL) {
        return OPRT_CJSON_PARSE_ERR;
    }

    if (cJSON_GetObjectItem(root, "httpsSelfUrl") == NULL || \
        cJSON_GetObjectItem(root, "mqttsSelfUrl") == NULL) {
        return OPRT_CR_CJSON_ERR;
    }

    char* httpsSelfUrl = cJSON_GetObjectItem(cJSON_GetObjectItem(root, "httpsSelfUrl"), "addr")->valuestring;
    char* mqttsSelfUrl = cJSON_GetObjectItem(cJSON_GetObjectItem(root, "mqttsSelfUrl"), "addr")->valuestring;
    char* caArr0 = cJSON_GetArrayItem(cJSON_GetObjectItem(root, "caArr"), 0)->valuestring;
    TY_LOGV("httpsSelfUrl:%s", httpsSelfUrl);
    TY_LOGV("mqttsSelfUrl:%s", mqttsSelfUrl);

    /* ATOP url decode */
    int port = 443;
    sscanf(httpsSelfUrl, "https://%99[^/]%99[^\n]", endport->atop.host, endport->atop.path);
    endport->atop.port = (uint16_t)port;
    TY_LOGV("endport->atop.host = \"%s\"", endport->atop.host);
    TY_LOGV("endport->atop.port = %d", endport->atop.port);
    TY_LOGV("endport->atop.path = \"%s\"", endport->atop.path);

    /* MQTT host decode */
    sscanf(mqttsSelfUrl, "%99[^:]:%99d[^\n]", endport->mqtt.host, &port);
    endport->mqtt.port = (uint16_t)port;
    TY_LOGV("endport->mqtt.host = \"%s\"", endport->mqtt.host);
    TY_LOGV("endport->mqtt.port = %d", endport->mqtt.port);

    /* cert decode */
    // base64 decode buffer
    size_t caArr0_len = strlen(caArr0);
    size_t buffer_len = caArr0_len * 3 / 4;
    uint8_t* caArr_raw = system_malloc(buffer_len);
    size_t caArr_raw_len = 0;

    // base64 decode
    if (mbedtls_base64_decode(caArr_raw, buffer_len, &caArr_raw_len, (const uint8_t*)caArr0, caArr0_len) != 0) {
        TY_LOGE("base64 decode error");
        system_free(caArr_raw);
        cJSON_Delete(root);
        return OPRT_COM_ERROR;
    }

    endport->atop.cert = caArr_raw;
    endport->atop.cert_len = caArr_raw_len;
    endport->mqtt.cert = caArr_raw;
    endport->mqtt.cert_len = caArr_raw_len;
    cJSON_Delete(root);
    return OPRT_OK;
}

int iotdns_cloud_endpoint_get(const char* region, const char* env, tuya_endpoint_t* endport)
{
    // TODO 参数校验
    if (NULL == region || NULL == env) {
        return OPRT_INVALID_PARM;
    }

    int rt = OPRT_OK;
    int i;
    http_client_status_t http_status;

    /* POST data buffer */
    size_t body_length = 0;
    uint8_t* body_buffer = system_malloc(128);
    if (NULL == body_buffer) {
        TY_LOGE("body_buffer malloc fail");
        return OPRT_MALLOC_FAILED;
    }

    body_length = sprintf(body_buffer, IOTDNS_REQUEST_FMT, region, env);
    TY_LOGV("out post data len:%d, data:%s", body_length, body_buffer);

    /* HTTP headers */
    http_client_header_t headers[] = {
        {.key = "Content-Type", .value = "application/x-www-form-urlencoded;charset=UTF-8"},
    };
    uint8_t headers_count = sizeof(headers)/sizeof(http_client_header_t);

    /* Response buffer length preview */
    uint8_t* response_buffer = NULL;
    size_t response_buffer_length = 1024 * 6;

    /* response buffer make */
    response_buffer = system_calloc(1, response_buffer_length);
    if (NULL == response_buffer) {
        TY_LOGE("response_buffer malloc fail");
        system_free(body_buffer);
        return OPRT_MALLOC_FAILED;
    }
    http_client_response_t http_response = {
        .buffer = response_buffer,
        .buffer_length = response_buffer_length
    };

    /* HTTP Request send */
    TY_LOGD("http request send!");
    http_status = http_client_request(
        &(const http_client_request_t){
            .cacert = iot_dns_cert_der,
            .cacert_len = sizeof(iot_dns_cert_der),
            .host = "h2.iot-dns.com",
            .port = 443,
            .method = "POST",
            .path = "/v1/url_config",
            .headers = headers,
            .headers_count = headers_count,
            .body = body_buffer,
            .body_length = body_length,
        }, 
        &http_response);

    /* Release http buffer */
    system_free(body_buffer);

    if (HTTP_CLIENT_SUCCESS != http_status) {
        TY_LOGE("http_request_send error:%d", http_status);
		system_free(response_buffer);
        return OPRT_LINK_CORE_HTTP_CLIENT_SEND_ERROR;
    }

    /* Decoded response data */
    rt = iotdns_response_decode(http_response.body, http_response.body_length, endport);
    system_free(response_buffer);
    return rt;
}
