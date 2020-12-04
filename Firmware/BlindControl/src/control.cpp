#include <Arduino.h>

#include "config.h"
#include "Timer.h"
#include "control.h"

Timer openTimer;
Timer closeTimer;

static status_t status = STATUS_OPENED;

static void openTimerEvent()
{
  stopBlind();
  digitalWrite(LED, LOW);

  status = STATUS_OPENED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, "OPENED");
}

static void closeTimerEvent()
{
  stopBlind();
  digitalWrite(LED, LOW);

  status = STATUS_CLOSED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, "CLOSED");
}

status_t getBlindStatus()
{
  return status;
}

const char* getBlindStatusText()
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
  digitalWrite(CLOSE_DIR_MOTOR_PIN, LOW);
  digitalWrite(OPEN_DIR_MOTOR_PIN, HIGH);
}

void stopOpenBlindAfterTime(long milli) {
  openTimer.after(milli, openTimerEvent);
}

void closeBlind()
{
  digitalWrite(CLOSE_DIR_MOTOR_PIN, HIGH);
  digitalWrite(OPEN_DIR_MOTOR_PIN, LOW);
}

void stopCloseBlindAfterTime(long milli) {
  closeTimer.after(milli, closeTimerEvent);
}

void stopBlind()
{
  digitalWrite(CLOSE_DIR_MOTOR_PIN, LOW);
  digitalWrite(OPEN_DIR_MOTOR_PIN, LOW);
}

void motorSetup() {

  pinMode(OPEN_DIR_MOTOR_PIN, OUTPUT);
  pinMode(CLOSE_DIR_MOTOR_PIN, OUTPUT);

  digitalWrite(OPEN_DIR_MOTOR_PIN, LOW);
  digitalWrite(CLOSE_DIR_MOTOR_PIN, LOW);
}

void motorUpdate() {

  openTimer.update();
  closeTimer.update();
}