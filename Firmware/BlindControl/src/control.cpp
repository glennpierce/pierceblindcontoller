#include <Arduino.h>

#include "config.h"
#include "settings.h"
#include "Timer.h"
#include "control.h"

// Timer eventTimer;
Timer openTimer;
Timer closeTimer;

static int openTimerId = -1;
static int closeTimerId = -1;

void turn_off_hbridge() {

  digitalWrite(DIS, HIGH);
  analogWrite(PWM, 0);
  digitalWrite(LED, LOW);

  Serial.println("turn_off_hbridge");
  logger("turn_off_hbridge");
}

static void turn_on_hbridge_cw() {

  digitalWrite(LED, HIGH);
  digitalWrite(DIR, CW);

  int pwm = (int)((get_speed() / 100.0) * PWMRANGE);
  pwm = max(0, min(pwm, PWMRANGE));

  analogWrite(PWM, pwm);
  
  digitalWrite(DIS, LOW);

  Serial.println("turn_on_hbridge_cw");
  logger("turn_on_hbridge_cw");
}

static void turn_on_hbridge_ccw() {

  digitalWrite(LED, HIGH);
  digitalWrite(DIR, CCW);

  int pwm = (int)((get_speed() / 100.0) * PWMRANGE);
  pwm = max(0, min(pwm, PWMRANGE));

  analogWrite(PWM, pwm);
  digitalWrite(DIS, LOW);

  Serial.println("turn_on_hbridge_ccw");
  logger("turn_on_hbridge_ccw");
}

static status_t status = STATUS_OPENED;

// static void checkFaultEvent()
// {
//   int err = digitalRead(ERROR);

//   if (err == 0) {
//     logger("max14871 error overcurrent or thermal shutdown");
//   }
// }

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
  status_t s = getBlindStatus();

  if(s == STATUS_CLOSED) {
    return "CLOSED";
  }
  else if (s == STATUS_OPENED) {
    return "OPENED";
  }
  else {
    return "UNDEFINED";
  }
}

bool openBlind(bool force)
{
  if(force == false && getBlindStatus() == STATUS_OPENED) {
    return false;
  }

  status = STATUS_UNDEFINED;
  turn_off_hbridge();
  turn_on_hbridge_cw();
  logger("openBlind");

  return true;
}

bool openBlindAndWait(bool force)
{
  if(openBlind(force) == false) {
    return false;
  }
  
  logger("openBlindAndWait for: " + get_opentime());
  stopOpenBlindAfterTime(get_opentime());  

  return true;
}

void stopOpenBlindAfterTime(long milli) {
  String s = String("stopOpenBlindAfterTime after: ") + milli;
  logger(s.c_str());
  openTimerId = openTimer.after(milli, openTimerEvent);
}

bool closeBlind(bool force)
{
  if(force == false && getBlindStatus() == STATUS_CLOSED) {
    return false;
  }

  status = STATUS_UNDEFINED;

  turn_off_hbridge();
  turn_on_hbridge_ccw();
  logger("closeBlind");

  return true;
}

bool closeBlindAndWait(bool force)
{
  if(closeBlind(force) == false) {
    return false;
  }

  logger("closeBlindAndWait for: " + get_closetime());
  stopCloseBlindAfterTime(get_closetime());  

  return true;
}

void stopCloseBlindAfterTime(long milli) {
  String s = String("stopCloseBlindAfterTime after: ") + milli;
  logger(s.c_str());
  closeTimerId = closeTimer.after(milli, closeTimerEvent);
}

void stopBlind() {
   turn_off_hbridge();
}

void motorSetup() {
  turn_off_hbridge();
  // eventTimer.every(1000, checkFaultEvent);
}

void motorUpdate() {

  // eventTimer.update();
  openTimer.update();
  closeTimer.update();
}

void stopTimers() {

  Serial.println("stopping timers");

  openTimer.stop(openTimerId);
  closeTimer.stop(closeTimerId);
}