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
  client->publish(mqtt_status, getBlindStatusText());
}

static void closeTimerEvent()
{
  stopBlind();
  digitalWrite(LED, LOW);

  status = STATUS_CLOSED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, getBlindStatusText());
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

bool openBlind(bool force)
{
  if(force== false && getBlindStatus() == STATUS_OPENED) {
    return false;
  }

  digitalWrite(CLOSE_DIR_MOTOR_PIN, LOW);
  digitalWrite(OPEN_DIR_MOTOR_PIN, HIGH);

  return true;
}

bool openBlindAndWait(bool force)
{
  if(openBlind(force) == false) {
    return false;
  }
  
  stopOpenBlindAfterTime(get_opentime());  

  return true;
}

void stopOpenBlindAfterTime(long milli) {
  openTimer.after(milli, openTimerEvent);
}

bool closeBlind(bool force)
{
  if(force== false && getBlindStatus() == STATUS_CLOSED) {
    return false;
  }

  digitalWrite(CLOSE_DIR_MOTOR_PIN, HIGH);
  digitalWrite(OPEN_DIR_MOTOR_PIN, LOW);

  return true;
}

bool closeBlindAndWait(bool force)
{
  if(closeBlind(force) == false) {
    return false;
  }

  stopCloseBlindAfterTime(get_closetime());  

  return true;
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