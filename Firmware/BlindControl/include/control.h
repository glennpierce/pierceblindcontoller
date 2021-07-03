
#ifndef __CONTROL__
#define __CONTROL__

typedef enum
{
  STATUS_CLOSED = 0,
  STATUS_OPENED = 1
} status_t;

void setupMcb23017();
void motorSetup();
void motorUpdate();
bool openBlind(bool force);
bool openBlindAndWait(bool force);
bool closeBlind(bool force);
bool closeBlindAndWait(bool force);
status_t getBlindStatus();
const char* getBlindStatusText();
void stopOpenBlindAfterTime(long milli);
void stopCloseBlindAfterTime(long milli);
void stopBlind();

#endif