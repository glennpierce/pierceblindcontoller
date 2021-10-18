#include <ESPAsyncWebServer.h>
#include "config.h"
#include "control.h"
#include "settings.h"

#include <ArduinoJson.h>
#include <CircularBuffer.h>

AsyncWebServer server(81);
AsyncWebSocket ws("/ws");

CircularBuffer<String,10> buffer;

void logger(const char* s) {
  buffer.push(String(millis()) + ": " + String(s));
}

void clearBuffer() {
    buffer.clear();
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

// String processor(const String& var) {

  // Serial.println("Processing: " + var);

  // if(var == "VERSION") {
  //   return FIRMWARE_VERSION;
  // }

  // if(var == "DEVICE_NAME") {
  //   return get_devicename();
  // }

  // if(var == "OPENTIME") {
  //   char buf[12]; 
  //   return itoa(get_opentime(), buf, 10);
  // }

  // if(var == "CLOSETIME") {
  //   char buf[12];
  //   return itoa(get_closetime(), buf, 10);
  // }

  // if(var == "SWITCH_BUTTONS_CHECKED") {
  //   if(get_button_pin_flipped()) {
  //     return "checked";
  //   }
  //   else {
  //     return "";
  //   }
  // }

  // if(var == "SPEED") {
  //   char buf[12];
  //   return itoa(get_speed(), buf, 10);
  // }

  // if(var == "OPEN_BUTTON_DISABLED") {
  //   if(getBlindStatus() == STATUS_OPENED) {
  //     return "disabled";
  //   }
  //   else {
  //     return "";
  //   }
  // }

  // if(var == "CLOSE_BUTTON_DISABLED") {
  //   if(getBlindStatus() == STATUS_CLOSED) {
  //     return "disabled";
  //   }
  //   else {
  //     return "";
  //   }
  // }

  // if(var == "LOG") {
  //   return get_log();
  // }
// }

void notFound(AsyncWebServerRequest *request) {
    Serial.println("No page found");
    request->send(404, "text/plain", "Not found");
}


// void notifyClients() {
//   ws.textAll(String(ledState));
// }

// void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
//   AwsFrameInfo *info = (AwsFrameInfo*)arg;
//   if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
//     data[len] = 0;
//     if (strcmp((char*)data, "toggle") == 0) {
//       ledState = !ledState;
//       notifyClients();
//     }
//   }
// }

// void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
//  void *arg, uint8_t *data, size_t len) {
//   switch (type) {
//     case WS_EVT_CONNECT:
//       Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
//       break;
//     case WS_EVT_DISCONNECT:
//       Serial.printf("WebSocket client #%u disconnected\n", client->id());
//       break;
//     case WS_EVT_DATA:
//       handleWebSocketMessage(arg, data, len);
//       break;
//     case WS_EVT_PONG:
//     case WS_EVT_ERROR:
//       break;
//   }
// }

// void initWebSocket() {
//   ws.onEvent(onEvent);
//   server.addHandler(&ws);
// }

void serve() {

    // initWebSocket();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Serving index");
        request->send(SPIFFS, "/index.html", String(), false);
        // request->send(SPIFFS, "/index.html", String(), false, processor);
        // request->send(SPIFFS, "/index.html", String(), true);
    });

    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Serving index");
        request->send(SPIFFS, "/index.html", String(), false);
        // request->send(SPIFFS, "/index.html", String(), false, processor);
        // request->send(SPIFFS, "/index.html", String(), true);
    });

    server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Serving css");
        request->send(SPIFFS, "/index.css", String(), false);
        // request->send(SPIFFS, "/index.css", String(), false, processor);
    });

    server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("Serving index js");
        request->send(SPIFFS, "/index.js", String(), false);
        // request->send(SPIFFS, "/index.js", String(), false, processor);
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

        String log = get_log();
        // request->send(SPIFFS, "/log.html", String(), false, processor);
        request->send(200, "application/text", log);
    });

    server.on("/clearlog", HTTP_GET, [](AsyncWebServerRequest *request){

        clearBuffer();

        String log = get_log();
        // request->send(SPIFFS, "/log.html", String(), false, processor);
        request->send(200, "application/text", log);
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
        
        set_button_pin_flipped(request->hasArg("buttons_switched"));
       
        if (request->hasParam("speed", true)) {
            result = request->getParam("speed", true)->value();
            set_speed(result.toInt());
        } 

        if (request->hasParam("devicename", true)) {
            result = request->getParam("devicename", true)->value();
            set_devicename(result.c_str());
        } 
        
        save_config();
        request->send(SPIFFS, "/index.html", String(), false);
        // request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    // server.on("/control", HTTP_POST, [](AsyncWebServerRequest *request) {

    //     String message;
    //     if (request->hasParam("open", true)) {
    //       openBlind(false);
    //       stopOpenBlindAfterTime(get_opentime());
    //     } 
    //     else if (request->hasParam("close", true)) {
    //       closeBlind(false);
    //       stopCloseBlindAfterTime(get_closetime());
    //     }
    //     else {
    //         message = "No message sent";
    //     }

    //     request->redirect("/");
    // });

    server.on("/open", HTTP_POST, [](AsyncWebServerRequest *request) {

        openBlind(false);
        stopOpenBlindAfterTime(get_opentime());
        request->send(200, "application/json", "{\"stats\": \"success\"}");
    });

    server.on("/close", HTTP_POST, [](AsyncWebServerRequest *request) {

        closeBlind(false);
        stopCloseBlindAfterTime(get_closetime());
        request->send(200, "application/json", "{\"stats\": \"success\"}");
    });

    server.on("/status", HTTP_POST, [](AsyncWebServerRequest *request) {

        DynamicJsonDocument doc(1024);
        doc["uptime"] = String(millis());
        doc["status"] = getBlindStatusText();
        doc["pwm"] = analogRead(PWM);
        doc["dis"] = digitalRead(DIS);
        doc["dir"] = digitalRead(DIR);
        doc["cw"] = CW;
        doc["ccw"] = CCW;

        String json_string;
        serializeJson(doc, json_string);

        request->send(200, "application/json", json_string);
    });

    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){

        DynamicJsonDocument doc(1024);
        doc["firmware"] = FIRMWARE_VERSION;
        doc["opentime"] = get_opentime();
        doc["closetime"] = get_closetime();
        doc["devicename"] = get_devicename();
        doc["buttons_switched"] = get_button_pin_flipped();
        doc["speed"] = get_speed();

        String json_string;
        serializeJson(doc, json_string);

        request->send(200, "application/json", json_string);
    });

    server.onNotFound(notFound);

    Serial.println("Webserver begin serving");
    server.begin();
}

void ws_loop() {
  ws.textAll(String(getBlindStatusText()));
  ws.cleanupClients();
}