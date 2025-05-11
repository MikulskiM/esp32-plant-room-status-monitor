#ifndef SERVER_H
#define SERVER_H

#include <WebServer.h>

#include "data_receiver.h"
#include "utils.h"
#include "secrets.h"

extern WebServer server;  // HTTP server on port 80

void setupWebServer();
void handleRoot();
void handleWebServer();

#endif