/**
 * \file            lwgsm_mqtt_client.h
 * \brief           MQTT client
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#ifndef GSM_HDR_APP_MQTT_CLIENT_H
#define GSM_HDR_APP_MQTT_CLIENT_H

#include "lwgsm/lwgsm.h"
#include "lwgsm/apps/lwgsm_mqtt_client_evt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \ingroup         GSM_APPS
 * \defgroup        GSM_APP_MQTT_CLIENT MQTT client
 * \brief           MQTT client
 * \{
 */

/**
 * \brief           Quality of service enumeration
 */
typedef enum {
    GSM_MQTT_QOS_AT_MOST_ONCE = 0x00,           /*!< Delivery is not guaranteed to arrive, but can arrive `up to 1 time` = non-critical packets where losses are allowed */
    GSM_MQTT_QOS_AT_LEAST_ONCE = 0x01,          /*!< Delivery is quaranteed `at least once`, but it may be delivered multiple times with the same content */
    GSM_MQTT_QOS_EXACTLY_ONCE = 0x02,           /*!< Delivery is quaranteed `exactly once` = very critical packets such as billing informations or similar */
} gsm_mqtt_qos_t;

struct gsm_mqtt_client;

/**
 * \brief           Pointer to \ref gsm_mqtt_client_t structure
 */
typedef struct gsm_mqtt_client* gsm_mqtt_client_p;

/**
 * \brief           State of MQTT client
 */
typedef enum {
    GSM_MQTT_CONN_DISCONNECTED = 0x00,          /*!< Connection with server is not established */
    GSM_MQTT_CONN_CONNECTING,                   /*!< Client is connecting to server */
    GSM_MQTT_CONN_DISCONNECTING,                /*!< Client connection is disconnecting from server */
    GSM_MQTT_CONNECTING,                        /*!< MQTT client is connecting... CONNECT command has been sent to server */
    GSM_MQTT_CONNECTED,                         /*!< MQTT is fully connected and ready to send data on topics */
} gsm_mqtt_state_t;

/**
 * \brief           MQTT client information structure
 */
typedef struct {
    const char* id;                             /*!< Client unique identifier. It is required and must be set by user */

    const char* user;                           /*!< Authentication username. Set to `NULL` if not required */
    const char* pass;                           /*!< Authentication password, set to `NULL` if not required */

    uint16_t keep_alive;                        /*!< Keep-alive parameter in units of seconds.
                                                    When set to `0`, functionality is disabled (not recommended) */

    const char* will_topic;                     /*!< Will topic */
    const char* will_message;                   /*!< Will message */
    gsm_mqtt_qos_t will_qos;                    /*!< Will topic quality of service */
} gsm_mqtt_client_info_t;

/**
 * \brief           MQTT request object
 */
typedef struct {
    uint8_t status;                             /*!< Entry status flag for in use or pending bit */
    uint16_t packet_id;                         /*!< Packet ID generated by client on publish */

    void* arg;                                  /*!< User defined argument */
    uint32_t expected_sent_len;                 /*!< Number of total bytes which must be sent
                                                    on connection before we can say "packet was sent". */

    uint32_t timeout_start_time;                /*!< Timeout start time in units of milliseconds */
} gsm_mqtt_request_t;

/**
 * \brief           MQTT event types
 */
typedef enum {
    GSM_MQTT_EVT_CONNECT,                       /*!< MQTT client connect event */
    GSM_MQTT_EVT_SUBSCRIBE,                     /*!< MQTT client subscribed to specific topic */
    GSM_MQTT_EVT_UNSUBSCRIBE,                   /*!< MQTT client unsubscribed from specific topic */
    GSM_MQTT_EVT_PUBLISH,                       /*!< MQTT client publish message to server event.
                                                    \note   When publishing packet with quality of service \ref GSM_MQTT_QOS_AT_MOST_ONCE,
                                                            you may not receive event, even if packet was successfully sent,
                                                            thus do not rely on this event for packet with `qos = GSM_MQTT_QOS_AT_MOST_ONCE` */
    GSM_MQTT_EVT_PUBLISH_RECV,                  /*!< MQTT client received a publish message from server */
    GSM_MQTT_EVT_DISCONNECT,                    /*!< MQTT client disconnected from MQTT server */
    GSM_MQTT_EVT_KEEP_ALIVE,                    /*!< MQTT keep-alive sent to server and reply received */
} gsm_mqtt_evt_type_t;

/**
 * \brief           List of possible results from MQTT server when executing connect command
 */
typedef enum {
    GSM_MQTT_CONN_STATUS_ACCEPTED =                 0x00,   /*!< Connection accepted and ready to use */
    GSM_MQTT_CONN_STATUS_REFUSED_PROTOCOL_VERSION = 0x01,   /*!< Connection Refused, unacceptable protocol version */
    GSM_MQTT_CONN_STATUS_REFUSED_ID =               0x02,   /*!< Connection refused, identifier rejected  */
    GSM_MQTT_CONN_STATUS_REFUSED_SERVER =           0x03,   /*!< Connection refused, server unavailable */
    GSM_MQTT_CONN_STATUS_REFUSED_USER_PASS =        0x04,   /*!< Connection refused, bad user name or password */
    GSM_MQTT_CONN_STATUS_REFUSED_NOT_AUTHORIZED =   0x05,   /*!< Connection refused, not authorized */
    GSM_MQTT_CONN_STATUS_TCP_FAILED =               0x100,  /*!< TCP connection to server was not successful */
} gsm_mqtt_conn_status_t;

/**
 * \brief           MQTT event structure for callback function
 */
typedef struct {
    gsm_mqtt_evt_type_t type;                   /*!< Event type */
    union {
        struct {
            gsm_mqtt_conn_status_t status;      /*!< Connection status with MQTT */
        } connect;                              /*!< Event for connecting to server */
        struct {
            uint8_t is_accepted;                /*!< Status if client was accepted to MQTT prior disconnect event */
        } disconnect;                           /*!< Event for disconnecting from server */
        struct {
            void* arg;                          /*!< User argument for callback function */
            lwgsmr_t res;                         /*!< Rgsmonse status */
        } sub_unsub_scribed;                    /*!< Event for (un)subscribe to/from topics */
        struct {
            void* arg;                          /*!< User argument for callback function */
            lwgsmr_t res;                         /*!< Rgsmonse status */
        } publish;                              /*!< Published event */
        struct {
            const uint8_t* topic;               /*!< Pointer to topic identifier */
            size_t topic_len;                   /*!< Length of topic */
            const void* payload;                /*!< Topic payload */
            size_t payload_len;                 /*!< Length of topic payload */
            uint8_t dup;                        /*!< Duplicate flag if message was sent again */
            gsm_mqtt_qos_t qos;                 /*!< Received packet quality of service */
        } publish_recv;                         /*!< Publish received event */
    } evt;                                      /*!< Event data parameters */
} gsm_mqtt_evt_t;

/**
 * \brief           MQTT event callback function
 * \param[in]       client: MQTT client
 * \param[in]       evt: MQTT event with type and related data
 */
typedef void        (*gsm_mqtt_evt_fn)(gsm_mqtt_client_p client, gsm_mqtt_evt_t* evt);

gsm_mqtt_client_p   gsm_mqtt_client_new(size_t tx_buff_len, size_t rx_buff_len);
void                gsm_mqtt_client_delete(gsm_mqtt_client_p client);

lwgsmr_t              gsm_mqtt_client_connect(gsm_mqtt_client_p client, const char* host, gsm_port_t port, gsm_mqtt_evt_fn evt_fn, const gsm_mqtt_client_info_t* info);
lwgsmr_t              gsm_mqtt_client_disconnect(gsm_mqtt_client_p client);
uint8_t             gsm_mqtt_client_is_connected(gsm_mqtt_client_p client);

lwgsmr_t              gsm_mqtt_client_subscribe(gsm_mqtt_client_p client, const char* topic, gsm_mqtt_qos_t qos, void* arg);
lwgsmr_t              gsm_mqtt_client_unsubscribe(gsm_mqtt_client_p client, const char* topic, void* arg);

lwgsmr_t              gsm_mqtt_client_publish(gsm_mqtt_client_p client, const char* topic, const void* payload, uint16_t len, gsm_mqtt_qos_t qos, uint8_t retain, void* arg);

void*               gsm_mqtt_client_get_arg(gsm_mqtt_client_p client);
void                gsm_mqtt_client_set_arg(gsm_mqtt_client_p client, void* arg);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_APP_MQTT_CLIENT_H */
