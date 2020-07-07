/* 4-Way Button:  Click, Double-Click, Press+Hold, and Press+Long-Hold Test Sketch

To keep a physical interface as simple as possible, this sketch demonstrates generating four output events from a single push-button->
1) Click:  rapid press and release
2) Double-Click:  two clicks in quick succession
3) Press and Hold:  holding the button down
4) Long Press and Hold:  holding the button for a long time
*/

// Assumes buttons pull low when pressed

#include <Arduino.h>
#include "buttons.h"

#define UP 13
#define DOWN 14

//=================================================

struct Button {
    int pin = -1;
    // Button timing variables
    unsigned int debounce;        
    unsigned int DCgap;           
    unsigned int holdTime;       
    unsigned int longHoldTime;   

    // Button variables
    bool buttonVal;
    bool buttonLast;
    bool DCwaiting;
    bool DConUp;
    bool singleOK;
    long downTime;
    long upTime;
    bool ignoreUp;  
    bool waitForUp;   
    bool holdEventPast;  
    bool longHoldEventPast;
    BUTTON_EVENT last_event;
}; 

Button* button_new(int pin) {
    Button* b = (Button*) malloc(sizeof(Button));

    b->pin = pin;
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
   
    b->debounce = 20;               // ms debounce period to prevent flickering when pressing or releasing the button
    b->DCgap = 250;                 // max ms between clicks for a double click event
    b->holdTime = 1000;             // ms hold period: how long to wait for press+hold event
    b->longHoldTime = 3000;         // ms long hold period: how long to wait for press+hold event

    b->buttonVal = HIGH;            // value read from button
    b->buttonLast = HIGH;           // buffered value of the button's previous state
    b->DCwaiting = false;           // whether we're waiting for a double click (down)
    b->DConUp = false;              // whether to register a double click on next release, or whether to wait and click
    b->singleOK = true;             // whether it's OK to do a single click
    b->downTime = -1;               // time the button was pressed down
    b->upTime = -1;                 // time the button was released
    b->ignoreUp = false;            // whether to ignore the button release because the click+hold was triggered
    b->waitForUp = false;           // when held, whether to wait for the up event
    b->holdEventPast = false;       // whether or not the hold event happened already
    b->longHoldEventPast = false;   // whether or not the long hold event happened already

    return b;
}

//=================================================
//  MULTI-CLICK:  One Button, Multiple Events
BUTTON_EVENT checkButton(Button* button) {

   BUTTON_EVENT event = NONE;
   button->buttonVal = digitalRead(button->pin);
   // Button pressed down
   if (button->buttonVal == LOW && button->buttonLast == HIGH && (millis() - button->upTime) > button->debounce)
   {
       button->downTime = millis();
       button->ignoreUp = false;
       button->waitForUp = false;
       button->singleOK = true;
       button->holdEventPast = false;
       button->longHoldEventPast = false;
       if ((millis()-button->upTime) < button->DCgap && button->DConUp == false && button->DCwaiting == true) {
           button->DConUp = true;
       }
       else {
           button->DConUp = false;
       }
       button->DCwaiting = false;
   }
   // Button released
   else if (button->buttonVal == HIGH && button->buttonLast == LOW && (millis() - button->downTime) > button->debounce)
   {       
       if (not button->ignoreUp)
       {
           button->upTime = millis();
           if (button->DConUp == false) {
               button->DCwaiting = true;
           }
           else
           {
               event = DOUBLE_CLICK;
               button->DConUp = false;
               button->DCwaiting = false;
               button->singleOK = false;
           }
       }
   }
   // Test for normal click event: DCgap expired
   if ( button->buttonVal == HIGH && (millis()-button->upTime) >=button-> DCgap && button->DCwaiting == true && button->DConUp == false && button->singleOK == true && event != 2)
   {
       event = CLICK;
       button->DCwaiting = false;
   }
   // Test for hold
   if (button->buttonVal == LOW && (millis() - button->downTime) >= button->holdTime) {
       // Trigger "normal" hold
        event = HOLD;
        button->waitForUp = true;
        button->ignoreUp = true;
        button->DConUp = false;
        button->DCwaiting = false;
        button->holdEventPast = true;
        // Trigger "long" hold
        if ((millis() - button->downTime) >= button->longHoldTime)
        {
                event = LONG_HOLD;
                button->longHoldEventPast = true;
        }
   }
   button->buttonLast = button->buttonVal;

   if(event == NONE) {
       if(button->last_event == HOLD) {
           button->last_event = NONE;
           return HOLD_RELEASE;
       }
       else if(button->last_event == LONG_HOLD) {
           button->last_event = NONE;
           return LONG_HOLD_RELEASE;
       }
   }

   button->last_event = event;

   return event;
}