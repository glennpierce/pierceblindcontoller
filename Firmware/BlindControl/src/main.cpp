#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <fauxmoESP.h>
//#include <WiFiManager.h>
#include <ArduinoJson.h>

#include "config.h"
#include "Timer.h"
#include "buttons.h"
#include "control.h"
#include "settings.h"

// const char* ssid = "NimhNoT";
// const char* password = "77G3PPTF";
const char* ssid = "NimhIoT";
const char* password = "746GDT42";

Button* openButton = NULL;
Button* closeButton = NULL;

long previousMillis = 0;
long interval = 1000;

fauxmoESP fauxmo;

bool failSafeMode = false;

//flag for saving data
bool shouldSaveConfig = false;

static char devicename[40] = DEVICE_NAME;
static uint32 opentime = 8000;
static uint32 closetime = 8000;
static char hostname[24] = "";

void set_devicename(const char *name) {
  strncpy(devicename, name, strlen(name) + 1);
}

void set_opentime(uint32 t) {
  opentime = t;
  Serial.println("Setting opentime to " + String(opentime));
}

void set_closetime(uint32 t) {
  closetime = t;
  Serial.println("Setting closetime to " + String(closetime));
}

const char* get_devicename()
{
  return devicename;
}

uint32 get_opentime() {
  Serial.println("Getting opentime: " + String(opentime));
  return opentime;
}

uint32 get_closetime() {
  Serial.println("Getting closetime: " + String(closetime));
  return closetime;
}

void WifiSetup() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Blind_80:7D:3A:6E:49:33
  sprintf(hostname, "Blind_%s", WiFi.macAddress().c_str());
  String tmpHostname = String(hostname);
  tmpHostname.replace(":", "");
  Serial.println(tmpHostname);
  WiFi.hostname(tmpHostname);

  WiFi.mode(WIFI_STA);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println(WiFi.SSID());
  Serial.println(WiFi.psk());
}

void OTASetup() {

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("NewBlind");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void save_config() {
  File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }

    DynamicJsonDocument doc(1024);
    doc["opentime"] = get_opentime();
    doc["closetime"] = get_closetime();
    doc["devicename"] = get_devicename();

    serializeJson(doc, Serial);
    serializeJson(doc, configFile);
    configFile.close();
}

void load_config() {
    File configFile = SPIFFS.open("/config.json", "r+");
    Serial.println("configFile: " + String(configFile));

    if (configFile)
    {
      DynamicJsonDocument doc(1024);
      String json = configFile.readString();
      Serial.println(json);
      DeserializationError error = deserializeJson(doc, json);
      if (!error) {

        set_opentime(doc["opentime"]);
        set_closetime(doc["closetime"]);
        set_devicename(doc["devicename"]);
      }
    }
    else
    {
      Serial.println("failed to open config file");
    }    
}

void setup() {

    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    delay(2000);
    digitalWrite(LED, LOW);

    // Initialize SPIFFS
    if (SPIFFS.begin()) {
      Serial.println("mounted file system");
    } else {
      Serial.println("failed to mount FS");
    }

    load_config();             

    WifiSetup();

    openButton = button_new(OPEN_BUTTON_PIN);
    closeButton = button_new(CLOSE_BUTTON_PIN);

    BUTTON_EVENT openButtonStatus = checkButton(openButton);
    BUTTON_EVENT closeButtonStatus = checkButton(closeButton);

    if(openButtonStatus == CLICK && closeButtonStatus == CLICK) {
      failSafeMode = true;
      OTASetup();
      return;
    }

    pinMode(OPEN_MOTOR, OUTPUT);
    pinMode(CLOSE_MOTOR, OUTPUT);
    digitalWrite(OPEN_MOTOR, LOW);
    digitalWrite(CLOSE_MOTOR, LOW);

    OTASetup();

    Serial.print("setting fauxmo device name: ");
    Serial.println(get_devicename());
    fauxmo.addDevice(get_devicename());

    fauxmo.createServer(true); // not needed, this is the default value
    fauxmo.setPort(80);        // required for gen3 devices
    fauxmo.enable(true);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        
        if(state) {
          openBlind();
          stopOpenBlindAfterTime(get_opentime());
        }
        else {
          closeBlind();
          stopCloseBlindAfterTime(get_closetime());
        }

        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    });

    Serial.println("Flashing");
    for(int i=0; i <= 5; i++) {
      digitalWrite(LED, HIGH);
      delay(250);
      digitalWrite(LED, LOW);
      delay(250);
    }

    serve();

    Serial.println("Started");
}

void loop() {

    if(failSafeMode) {
      ArduinoOTA.handle();
      return;
    }

    motorUpdate();

    fauxmo.handle();

    BUTTON_EVENT status = checkButton(openButton);

    if(status == CLICK) {

      if(getBlindStatus() != STATUS_OPENED) {
        digitalWrite(LED, HIGH);
        openBlind();
        Serial.println("Open clicked");
        stopOpenBlindAfterTime(get_opentime());
      }
    }
    else if(status == HOLD || status == LONG_HOLD) {
      Serial.println("Open long hold");
      digitalWrite(LED, HIGH);
      openBlind();
    }
    else if(status == HOLD_RELEASE || status == LONG_HOLD_RELEASE) {
      Serial.println("Stopping motor");
      digitalWrite(LED, LOW);
      stopBlind();
    }

    status = checkButton(closeButton);

    if(status == CLICK) {
      if(getBlindStatus() != STATUS_CLOSED) {
        digitalWrite(LED, HIGH);
        closeBlind();

        Serial.println("Close clicked");
        stopCloseBlindAfterTime(get_closetime());
      }
    }
    else if(status == HOLD || status == LONG_HOLD) {
      Serial.println("Close long hold");
      digitalWrite(LED, HIGH);
      closeBlind();
    }
    else if(status == HOLD_RELEASE || status == LONG_HOLD_RELEASE) {
      Serial.println("Stopping motor");
      digitalWrite(LED, LOW);
      stopBlind();
    }

    // delay(2000);

    // digitalWrite(PUSH_MOTOR, LOW);
    // digitalWrite(PULL_MOTOR, LOW);

    ArduinoOTA.handle();
}