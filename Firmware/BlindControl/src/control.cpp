#include <Arduino.h>

#include "config.h"
#include "Timer.h"
#include "control.h"

Timer openTimer;
Timer closeTimer;

static void openTimerEvent()
{
  digitalWrite(OPEN_MOTOR, LOW);
  digitalWrite(LED, LOW);
}

static void closeTimerEvent()
{
  digitalWrite(CLOSE_MOTOR, LOW);
  digitalWrite(LED, LOW);
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