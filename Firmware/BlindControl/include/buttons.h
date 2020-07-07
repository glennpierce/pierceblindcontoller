#ifndef __BUTTONS__
#define __BUTTONS__

#define FOREACH_BUTTON_EVENT(ITEM) \
        ITEM(NONE)   \
        ITEM(CLICK)  \
        ITEM(DOUBLE_CLICK)   \
        ITEM(HOLD)  \
        ITEM(LONG_HOLD)  \
        ITEM(HOLD_RELEASE)   \
        ITEM(LONG_HOLD_RELEASE)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) String(#STRING),

enum BUTTON_EVENT {
    FOREACH_BUTTON_EVENT(GENERATE_ENUM)
};

static const String BUTTON_EVENT_STRING[] = {
    FOREACH_BUTTON_EVENT(GENERATE_STRING)
};


/*
typedef enum {
  NONE,
  CLICK,
  DOUBLE_CLICK,
  HOLD,
  LONG_HOLD,
  HOLD_RELEASE,
  LONG_HOLD_RELEASE,
} BUTTON_EVENT; 
*/


typedef struct Button Button;

Button* button_new(int pin);
BUTTON_EVENT checkButton(Button* button);

#endif