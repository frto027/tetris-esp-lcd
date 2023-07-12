#ifndef WIFIINIT_H
#define WIFIINIT_H

#include "esp_http_server.h"

void wifiinit();

extern esp_err_t data_handler(httpd_req_t *req);

#endif