#include <Arduino.h>

#include "config.h"
#include "Timer.h"
#include "control.h"

Timer openTimer;
Timer closeTimer;

static status_t status = STATUS_OPENED;

static void openTimerEvent()
{
  digitalWrite(get_open_pin(), LOW);
  digitalWrite(LED, LOW);

  status = STATUS_OPENED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, "OPENED");
}

static void closeTimerEvent()
{
  digitalWrite(get_close_pin(), LOW);
  digitalWrite(LED, LOW);

  status = STATUS_CLOSED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, "CLOSED");
}

status_t getBlindStatus()
{
  return status;
}

char* getBlindStatusText()
{
  if(getBlindStatus() == STATUS_CLOSED) {
    return "CLOSED";
  }
  else {
    return "OPENED";
  }
}

void openBlind()
{
  digitalWrite(get_close_pin(), LOW);
  digitalWrite(get_open_pin(), HIGH);
}

void stopOpenBlindAfterTime(long milli) {
  openTimer.after(milli, openTimerEvent);
}

void closeBlind()
{
  digitalWrite(get_close_pin(), HIGH);
  digitalWrite(get_open_pin(), LOW);
}

void stopCloseBlindAfterTime(long milli) {
  closeTimer.after(milli, closeTimerEvent);
}

void stopBlind()
{
  digitalWrite(get_close_pin(), LOW);
  digitalWrite(get_open_pin(), LOW);
}

void motorSetup() {

  pinMode(get_open_pin(), OUTPUT);
  pinMode(get_close_pin(), OUTPUT);

  digitalWrite(get_open_pin(), LOW);
  digitalWrite(get_close_pin(), LOW);
}

void motorUpdate() {

  openTimer.update();
  closeTimer.update();
}