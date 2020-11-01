#ifndef __CONFIG__
#define __CONFIG__

#include <WiFiClient.h>
#include <PubSubClient.h>

#define DEVICE_NAME "Blind"

#define FIRMWARE_VERSION "0.9"
#define LED 16
#define OPEN_BUTTON_PIN 13
#define CLOSE_BUTTON_PIN 14

#define OPEN_MOTOR 4
#define CLOSE_MOTOR 5

void set_devicename(const char *name);
void set_opentime(uint32 t);
void set_closetime(uint32 t);
const char* get_devicename();
uint32 get_opentime();
uint32 get_closetime();

extern char mqtt_status[200];
extern char mqtt_cmnd[200];

PubSubClient* get_pubsub_client();

#endif