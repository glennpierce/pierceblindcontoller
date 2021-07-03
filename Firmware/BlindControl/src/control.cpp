#include <Arduino.h>

#include "config.h"
#include "settings.h"
#include "Timer.h"
#include "control.h"

#include <Wire.h>
#include <MCP23017.h>

MCP23017 mcp = MCP23017(0x20);

Timer openTimer;
Timer closeTimer;

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

#define N1G 0
#define P1G 1
#define N2G 2
#define P2G 3

#define NCHANNEL_ON 0  // Opto is 0 Therefore Reciever is pulled high NChannel so on
#define NCHANNEL_OFF 1 // Opto is 1 Therefore Reciever is pulled low NChannel so off

#define PCHANNEL_ON 1  // Opto is 1 Therefore Reciever is pulled low PChannel so on
#define PCHANNEL_OFF 0 // Opto is 0 Therefore Reciever is pulled high 0 PChannel so off

static String Misc_DecTo8bitBinary(int dec) {
 String result = "";
 for (unsigned int i = 0x80; i; i >>= 1) {
 result.concat(dec  & i ? '1' : '0');
 }
 return result;
}

// static void turn_off_n1g_and_n2g() {

//   mcp.digitalWrite(N1G, NCHANNEL_OFF);
//   delay(1); // Delay for a millisecond. Lets try to avoid situation where two mosfets are turned
//             // on at once

//   mcp.digitalWrite(N2G, NCHANNEL_OFF);
//   delay(1);

//   Serial.println("turn_off_n1g_and_n2g");

//   // Serial.println("turn_off_n1g_and_n2g:" + Misc_DecTo8bitBinary(hbridge_state));
// }

// static void turn_off_p1g_and_p2g() {
//   // BIT_SET(hbridge_state, 1);
//   // mcp.writePort(MCP23017Port::A, hbridge_state);
//   mcp.digitalWrite(P1G, PCHANNEL_OFF);
//   delay(1);
//   mcp.digitalWrite(P2G, PCHANNEL_OFF);
//   delay(1);

//   Serial.println("turn_off_p1g_and_p2g");
// }

static void turn_off_hbridge() {
  // turn_off_n1g_and_n2g();
  // turn_off_p1g_and_p2g();

  mcp.digitalWrite(N1G, NCHANNEL_OFF);
  delay(10); // Delay for a millisecond. Lets try to avoid situation where two mosfets are turned
            // on at once

  mcp.digitalWrite(N2G, NCHANNEL_OFF);
  delay(10);

  mcp.digitalWrite(P1G, PCHANNEL_OFF);
  delay(10); // Delay for a millisecond. Lets try to avoid situation where two mosfets are turned
            // on at once

  mcp.digitalWrite(P2G, PCHANNEL_OFF);
  delay(10);

  Serial.println("turn_off_hbridge");
}


// Turn motor direction 1
// static  void turn_on_n1g_and_p2g() {

//   turn_off_hbridge();

//   mcp.digitalWrite(N1G, NCHANNEL_ON);   // n channel n1g
//   delay(1); // Delay 1 ms rise fall time of opto can't be more than 10-100 us

//   mcp.digitalWrite(P2G, PCHANNEL_ON);
//   delay(1);

//   Serial.println("turn_on_n1g_and_p2g");
//   logger("turn_on_n1g_and_p2g");
// }

// // Turn motor direction 2
// static void turn_on_n2g_and_p1g() {

//   turn_off_hbridge();

//   mcp.digitalWrite(N2G, NCHANNEL_ON);
//   delay(1);

//   mcp.digitalWrite(P1G, PCHANNEL_ON);
//   delay(1);

//   Serial.println("turn_on_n2g_and_p1g");
//   logger("turn_on_n2g_and_p1g");
// }


static void turn_on_hbridge_dir1() {

  // turn_off_hbridge();

  mcp.digitalWrite(P1G, PCHANNEL_OFF);
  delay(10);

  mcp.digitalWrite(N2G, NCHANNEL_OFF);   // n channel n1g
  delay(10); // Delay 1 ms rise fall time of opto can't be more than 10-100 us

  mcp.digitalWrite(N1G, NCHANNEL_ON);   // n channel n1g
  delay(10);

  mcp.digitalWrite(P2G, PCHANNEL_ON);
  delay(10);

  Serial.println("turn_on_hbridge_dir1");
  logger("turn_on_hbridge_dir1");
}


static  void turn_on_hbridge_dir2() {

  // turn_off_hbridge();

  mcp.digitalWrite(N1G, NCHANNEL_OFF);   // n channel n1g
  delay(10);

  mcp.digitalWrite(P2G, PCHANNEL_OFF);
  delay(10);

  mcp.digitalWrite(P1G, PCHANNEL_ON);
  delay(10);

  mcp.digitalWrite(N2G, NCHANNEL_ON);   // n channel n1g
  delay(10); // Delay 1 ms rise fall time of opto can't be more than 10-100 us

  Serial.println("turn_on_hbridge_dir2");
  logger("turn_on_hbridge_dir2");
}

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
  if(force == false && getBlindStatus() == STATUS_OPENED) {
    return false;
  }

  digitalWrite(LED, HIGH);
  // turn_on_n2g_and_p1g();
  turn_on_hbridge_dir1();
  logger("openBlind");

  return true;
}

bool openBlindAndWait(bool force)
{
  if(openBlind(force) == false) {
    return false;
  }
  
  digitalWrite(LED, HIGH);
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

  digitalWrite(LED, HIGH);
  // turn_on_n1g_and_p2g();
  turn_on_hbridge_dir2();
  logger("closeBlind");

  return true;
}

bool closeBlindAndWait(bool force)
{
  if(closeBlind(force) == false) {
    return false;
  }

  digitalWrite(LED, HIGH);
  stopCloseBlindAfterTime(get_closetime());  

  return true;
}

void stopCloseBlindAfterTime(long milli) {
  closeTimer.after(milli, closeTimerEvent);
}

void setupMcb23017()
{
  Wire.begin();
  mcp.init();

  mcp.portMode(MCP23017Port::A, 0);          // Port A as output
  mcp.portMode(MCP23017Port::B, 0); 
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  // Reset port A 
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  // Reset port B
  // mcp.writePort(MCP23017Port::A, 0b00001010);
  // mcp.writePort(MCP23017Port::A, 0b00000000);
  // mcp.writePort(MCP23017Port::A, 0b00000101);
  mcp.writePort(MCP23017Port::B, 0b00000000);

  turn_off_hbridge();
}

void stopBlind()
{
   turn_off_hbridge();


   digitalWrite(LED, LOW);
}

void motorUpdate() {

  openTimer.update();
  closeTimer.update();
}