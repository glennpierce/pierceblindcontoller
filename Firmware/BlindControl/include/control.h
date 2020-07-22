
#ifndef __CONTROL__
#define __CONTROL__

typedef enum
{
  STATUS_CLOSED = 0,
  STATUS_OPENED = 1
} status_t;

void motorSetup();
void motorUpdate();
void openBlind();
void closeBlind();
status_t getBlindStatus();
void stopOpenBlindAfterTime(long milli);
void stopCloseBlindAfterTime(long milli);
void stopBlind();

#endif