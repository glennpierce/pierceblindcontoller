#include <Arduino.h>

#include "config.h"
#include "Timer.h"
#include "control.h"

Timer openTimer;
Timer closeTimer;

static status_t status = STATUS_OPENED;

static void openTimerEvent()
{
  digitalWrite(OPEN_MOTOR, LOW);
  digitalWrite(LED, LOW);

  status = STATUS_OPENED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, "OPENED");
}

static void closeTimerEvent()
{
  digitalWrite(CLOSE_MOTOR, LOW);
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
  digitalWrite(CLOSE_MOTOR, LOW);
  digitalWrite(OPEN_MOTOR, HIGH);
}

void stopOpenBlindAfterTime(long milli) {
  openTimer.after(milli, openTimerEvent);
}

void closeBlind()
{
  digitalWrite(CLOSE_MOTOR, HIGH);
  digitalWrite(OPEN_MOTOR, LOW);
}

void stopCloseBlindAfterTime(long milli) {
  closeTimer.after(milli, closeTimerEvent);
}

void stopBlind()
{
  digitalWrite(CLOSE_MOTOR, LOW);
  digitalWrite(OPEN_MOTOR, LOW);
}

void motorSetup() {

    pinMode(OPEN_MOTOR, OUTPUT);
    pinMode(CLOSE_MOTOR, OUTPUT);

    digitalWrite(OPEN_MOTOR, LOW);
    digitalWrite(CLOSE_MOTOR, LOW);
}

void motorUpdate() {

    openTimer.update();
    closeTimer.update();
}