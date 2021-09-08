#include <Arduino.h>

#include "config.h"
#include "settings.h"
#include "Timer.h"
#include "control.h"

Timer eventTimer;
Timer openTimer;
Timer closeTimer;

void turn_off_hbridge() {

  digitalWrite(LED, LOW);
  digitalWrite(DIS, HIGH);
  analogWrite(PWM, 0);

  Serial.println("turn_off_hbridge");
  logger("turn_off_hbridge");
}

static void turn_on_hbridge_cw() {

  digitalWrite(LED, HIGH);
  digitalWrite(DIR, CW);
  analogWrite(PWM, PWMRANGE);
  digitalWrite(DIS, LOW);

  Serial.println("turn_on_hbridge_cw");
  logger("turn_on_hbridge_cw");
}


static void turn_on_hbridge_ccw() {

  digitalWrite(LED, HIGH);
  digitalWrite(DIR, CCW);
  analogWrite(PWM, PWMRANGE);
  digitalWrite(DIS, LOW);

  Serial.println("turn_on_hbridge_ccw");
  logger("turn_on_hbridge_ccw");
}

static status_t status = STATUS_OPENED;

static void checkFaultEvent()
{
  int err = digitalRead(ERROR);

  if (err == 0) {
    logger("max14871 error overcurrent or thermal shutdown");
  }
}

static void openTimerEvent()
{
  stopBlind();
  
  status = STATUS_OPENED;

  PubSubClient* client = get_pubsub_client();
  client->publish(mqtt_status, getBlindStatusText());
}

static void closeTimerEvent()
{
  stopBlind();

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
  if(force == false && getBlindStatus() == STATUS_OPENED) {
    return false;
  }

  turn_on_hbridge_cw();
  logger("openBlind");

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
  if(force == false && getBlindStatus() == STATUS_CLOSED) {
    return false;
  }

  turn_on_hbridge_ccw();
  logger("closeBlind");

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

void stopBlind() {
   turn_off_hbridge();
}

void motorSetup() {
  turn_off_hbridge();
  eventTimer.every(1000, checkFaultEvent);
}

void motorUpdate() {

  eventTimer.update();
  openTimer.update();
  closeTimer.update();
}