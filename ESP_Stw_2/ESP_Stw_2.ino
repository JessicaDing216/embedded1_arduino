/*

  ESP Steering wheel example 14.11.2019 KN

*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>




// Replace with your network credentials
//const char* ssid = "mokkula_925936";
//const char* password = "12345678";

const char* ssid = "SOURCE";
const char* password = "Pelle!23";


#define TESTFILE "/index.html"
bool    spiffsActive = false;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// Replaces placeholder with LED state value
String processor(const String& var)
{
  ;
}




void setup() {
  // Serial port for debugging purposes
  Serial.begin(9600);

  Serial.println("-------------");

  Serial.println("-------------");

  // Start filing subsystem
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Active");
    Serial.println();
    spiffsActive = true;
  } else {
    Serial.println("Unable to activate SPIFFS");
  }


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());




  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);

    //     Serial.println(" HTML GET ");


    int paramsNr = request->params();

    //   Serial.println(" ------ ");
    //   Serial.print("Param qty: ");
    //   Serial.println(paramsNr);

    for (byte i = 0; i < paramsNr; i++)
    {

      AsyncWebParameter* p = request->getParam(0);

      //  Serial.print("Param name: ");
      //  Serial.println(p->name());

      if ((p->name()) == "dist") {
        Serial.print("Move:");
        Serial.println(p->value());
      };
      if ((p->name()) == "dire") {
        Serial.print("Turn:");
        Serial.println(p->value());
      };
      if ((p->name()) == "danc") {
        Serial.print("Dance:");
        Serial.println(p->value());
      };

      //  Serial.print("Param value: ");
      //  Serial.println(p->value());
    };


  });


  // Route to load style.css file
  server.on("/jquery-1.11.3.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/jquery-1.11.3.min.js", "text/javascript");

    Serial.println(" JS jquery GET ");

  });

  // Route to load style.css file
  server.on("/roundslider.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/roundslider.min.js", "text/javascript");

    Serial.println(" min JS GET ");

  });


  // Route to load style.css file
  server.on("/roundslider.min.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/roundslider.min.css", "text/css");

    Serial.println(" min css GET ");

  });

  // Route to set GPIO to HIGH
  server.on("/G---", HTTP_GET, [](AsyncWebServerRequest * request) {

    request->send(SPIFFS, "/index.html", String(), false, processor);

    Serial.println(" ---- X ----- ");

  });




  // Start server
  server.begin();
}

void loop() {

}
