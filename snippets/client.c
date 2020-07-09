#include "client.h"
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_network_api.h"

/* Host parameter */
#define CONN_HOST           "example.com"
#define CONN_PORT           80

static lwgsmr_t   conn_callback_func(gsm_evt_t* evt);

/**
 * \brief           Request data for connection
 */
static const
uint8_t req_data[] = ""
                     "GET / HTTP/1.1\r\n"
                     "Host: " CONN_HOST "\r\n"
                     "Connection: close\r\n"
                     "\r\n";

/**
 * \brief           Start a new connection(s) as client
 */
void
client_connect(void) {
    lwgsmr_t res;

    /* Attach to GSM network */
    gsm_network_request_attach();

    /* Start a new connection as client in non-blocking mode */
    if ((res = gsm_conn_start(NULL, GSM_CONN_TYPE_TCP, "example.com", 80, NULL, conn_callback_func, 0)) == gsmOK) {
        printf("Connection to " CONN_HOST " started...\r\n");
    } else {
        printf("Cannot start connection to " CONN_HOST "!\r\n");
    }
}

/**
 * \brief           Event callback function for connection-only
 * \param[in]       evt: Event information with data
 * \return          \ref gsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t
conn_callback_func(gsm_evt_t* evt) {
    gsm_conn_p conn;
    lwgsmr_t res;
    uint8_t conn_num;

    conn = gsm_conn_get_from_evt(evt);          /* Get connection handle from event */
    if (conn == NULL) {
        return gsmERR;
    }
    conn_num = gsm_conn_getnum(conn);           /* Get connection number for identification */
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_CONN_ACTIVE: {             /* Connection just active */
            printf("Connection %d active!\r\n", (int)conn_num);
            res = gsm_conn_send(conn, req_data, sizeof(req_data) - 1, NULL, 0); /* Start sending data in non-blocking mode */
            if (res == gsmOK) {
                printf("Sending request data to server...\r\n");
            } else {
                printf("Cannot send request data to server. Closing connection manually...\r\n");
                gsm_conn_close(conn, 0);        /* Close the connection */
            }
            break;
        }
        case GSM_EVT_CONN_CLOSE: {              /* Connection closed */
            if (gsm_evt_conn_close_is_forced(evt)) {
                printf("Connection %d closed by client!\r\n", (int)conn_num);
            } else {
                printf("Connection %d closed by remote side!\r\n", (int)conn_num);
            }
            break;
        }
        case GSM_EVT_CONN_SEND: {               /* Data send event */
            lwgsmr_t res = gsm_evt_conn_send_get_result(evt);
            if (res == gsmOK) {
                printf("Data sent successfully on connection %d...waiting to receive data from remote side...\r\n", (int)conn_num);
            } else {
                printf("Error while sending data on connection %d!\r\n", (int)conn_num);
            }
            break;
        }
        case GSM_EVT_CONN_RECV: {               /* Data received from remote side */
            gsm_pbuf_p pbuf = gsm_evt_conn_recv_get_buff(evt);
            gsm_conn_recved(conn, pbuf);        /* Notify stack about received pbuf */
            printf("Received %d bytes on connection %d..\r\n", (int)gsm_pbuf_length(pbuf, 1), (int)conn_num);
            break;
        }
        case GSM_EVT_CONN_ERROR: {              /* Error connecting to server */
            const char* host = gsm_evt_conn_error_get_host(evt);
            gsm_port_t port = gsm_evt_conn_error_get_port(evt);
            printf("Error connecting to %s:%d\r\n", host, (int)port);
            break;
        }
        default:
            break;
    }
    return gsmOK;
}
