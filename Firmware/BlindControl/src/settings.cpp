#include <ESPAsyncWebServer.h>
#include "config.h"
#include "control.h"
#include "settings.h"

AsyncWebServer server(81);

String processor(const String& var) {

  Serial.println("Processing: " + var);

  if(var == "VERSION") {
    return FIRMWARE_VERSION;
  }

  if(var == "DEVICE_NAME") {
    return get_devicename();
  }

  if(var == "OPENTIME") {
    char buf[12]; 
    return itoa(get_opentime(), buf, 10);
  }

  if(var == "CLOSETIME") {
    char buf[12];
    return itoa(get_closetime(), buf, 10);
  }

  if(var == "SWITCH_BUTTONS_CHECKED") {
    if(get_pin_flipped()) {
      return "checked";
    }
    else {
      return "";
    }
  }
}

void notFound(AsyncWebServerRequest *request) {
    Serial.println("No page found");
    request->send(404, "text/plain", "Not found");
}

void serve() {

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Serving index");
        request->send(SPIFFS, "/index.html", String(), false, processor);
        // request->send(SPIFFS, "/index.html", String(), true);
    });

    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/styles.css", String(), false);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
        String result;
        if (request->hasParam("opentime", true)) {
            result = request->getParam("opentime", true)->value();
            set_opentime(result.toInt());
        }
        
        if (request->hasParam("closetime", true)) {
            result = request->getParam("closetime", true)->value();
            set_closetime(result.toInt());
        } 
        
        set_pin_flipped(request->hasArg("switch_buttons"));
       
        if (request->hasParam("hostname", true)) {
            result = request->getParam("hostname", true)->value();
            set_devicename(result.c_str());
        } 
        
        save_config();
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/control", HTTP_POST, [](AsyncWebServerRequest *request) {

        String message;
        if (request->hasParam("open", true)) {
          openBlind();
          stopOpenBlindAfterTime(get_opentime());
        } 
        else if (request->hasParam("close", true)) {
          closeBlind();
          stopCloseBlindAfterTime(get_closetime());
        }
        else {
            message = "No message sent";
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.onNotFound(notFound);

    Serial.println("Webserver begin serving");
    server.begin();
}