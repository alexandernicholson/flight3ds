#include "api.h"
#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define API_BASE     "https://opensky-network.org/api/states/all"
#define RECV_BUF_SZ  (256 * 1024)  // 256 KB receive buffer

static u8 *recv_buf = NULL;

bool api_init(void) {
    httpcInit(0);
    recv_buf = malloc(RECV_BUF_SZ);
    return recv_buf != NULL;
}

static int http_get(const char *url, u8 *buf, int buf_size) {
    httpcContext context;
    Result ret;

    ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
    if (R_FAILED(ret)) return -1;

    httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
    httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
    httpcAddRequestHeaderField(&context, "User-Agent", "Flight3DS/1.0 (3DS Homebrew)");
    httpcAddRequestHeaderField(&context, "Accept", "application/json");

    ret = httpcBeginRequest(&context);
    if (R_FAILED(ret)) {
        httpcCloseContext(&context);
        return -1;
    }

    u32 status_code = 0;
    httpcGetResponseStatusCode(&context, &status_code);
    if (status_code != 200) {
        httpcCloseContext(&context);
        return -1;
    }

    int total = 0;
    while (total < buf_size - 1) {
        u32 bytes_read = 0;
        ret = httpcDownloadData(&context, buf + total, buf_size - 1 - total, &bytes_read);
        total += bytes_read;
        if (ret != (s32)HTTPC_RESULTCODE_DOWNLOADPENDING) break;
    }
    buf[total] = '\0';

    httpcCloseContext(&context);
    return total;
}

int api_fetch_flights(FlightDB *db, float lat_min, float lat_max,
                      float lon_min, float lon_max) {
    char url[256];
    snprintf(url, sizeof(url),
             "%s?lamin=%.2f&lomin=%.2f&lamax=%.2f&lomax=%.2f",
             API_BASE, lat_min, lon_min, lat_max, lon_max);

    int len = http_get(url, recv_buf, RECV_BUF_SZ);
    if (len <= 0) {
        db->online = false;
        return -1;
    }

    return flights_update_from_json(db, (const char *)recv_buf, len);
}

int api_fetch_all(FlightDB *db) {
    int len = http_get(API_BASE, recv_buf, RECV_BUF_SZ);
    if (len <= 0) {
        db->online = false;
        return -1;
    }

    return flights_update_from_json(db, (const char *)recv_buf, len);
}

void api_shutdown(void) {
    if (recv_buf) {
        free(recv_buf);
        recv_buf = NULL;
    }
    httpcExit();
}
