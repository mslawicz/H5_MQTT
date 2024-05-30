#ifndef __MQTT_CLIENT_H
#define __MQTT_CLIENT_H

#include "tx_api.h"

#define MQTT_CLIENT_PRIORITY    15

extern VOID mqttClientThreadEntry(ULONG initial_input);

#endif /* __MQTT_CLIENT_H */