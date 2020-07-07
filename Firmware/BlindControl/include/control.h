
#ifndef __CONTROL__
#define __CONTROL__

void motorSetup();
void motorUpdate();
void openBlind();
void closeBlind();
void stopOpenBlindAfterTime(long milli);
void stopCloseBlindAfterTime(long milli);
void stopBlind();

#endif