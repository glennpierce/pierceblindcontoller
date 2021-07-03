#include <ESPAsyncWebServer.h>
#include "config.h"
#include "control.h"
#include "settings.h"

#include <CircularBuffer.h>

AsyncWebServer server(81);

CircularBuffer<String,10> buffer;

void logger(const char* s) {
  buffer.push(String(millis()) + ": " + String(s));
}

String get_log() {
  String s = "";
  const uint8_t len = buffer.size();
  for (uint8_t i = 0; i < len; i++) {
    s.concat(buffer[i]);
    s.concat("<br>");
  }
  return s;
}

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
    if(get_button_pin_flipped()) {
      return "checked";
    }
    else {
      return "";
    }
  }

  if(var == "LOG") {
    return get_log();
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

    server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
        // String s = "";
        // const uint8_t len = buffer.size();
        // for (uint8_t i = 0; i < len; i++) {
        //   s.concat(buffer[i]);
        //   s.concat("<br>");
        // }
        // request->send(200, "text/plain", s);

        request->send(SPIFFS, "/log.html", String(), false, processor);
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
        
        set_button_pin_flipped(request->hasArg("switch_buttons"));
       
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
          openBlind(false);
          stopOpenBlindAfterTime(get_opentime());
        } 
        else if (request->hasParam("close", true)) {
          closeBlind(false);
          stopCloseBlindAfterTime(get_closetime());
        }
        else {
            message = "No message sent";
        }

        request->redirect("/");
    });

    server.onNotFound(notFound);

    Serial.println("Webserver begin serving");
    server.begin();
}