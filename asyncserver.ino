#include <WiFi.h>
//#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESP32Servo.h>
#include <WebSocketsServer.h>
#include <WiFiMulti.h>
#include <WebServer.h>

Servo myservo;  // create servo object to control a servo

static const int servoPin = 18;

const char* ssid = "WDR4300";
const char* password = "julyhouse3565472";

long ESP32_send_LastTime;
int ESP32_send_Rate = 20;

WebServer server(80);
WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);
 
void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {
 
  // Figure out the type of WebSocket event
  switch(type) {
 
    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
 
    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;
 
    // Echo text message back to client
    case WStype_TEXT:
      Serial.printf("[%u] Text: %s\n", num, payload);
      webSocket.sendTXT(num, payload);
      break;
 
    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path); // 在序列埠顯示路徑

  if (path.endsWith("/")) {
    path += "joystick.html";
  }

  String contentType = getContentType(path);
  
  if (SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();

    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
 
  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
  }
  
WiFiMulti.addAP(ssid, password);

  Serial.println("Connecting");
  while (WiFiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
 
  Serial.println("\r\n");
  if(WiFi.softAPgetStationNum() == 0) {      // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println("\r\n");


//  ws.onEvent(onWsEvent);
//  server.addHandler(&ws);

//  server.on("/html", HTTP_GET, [](){
//    request->send(SPIFFS, "/joystick.html", "text/html");
//  });

  server.onNotFound([](){
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
//server.on("/", rootRouter);
//  server.on("/joystick.html", rootRouter);
 
  server.begin();
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}


void loop() {
  server.handleClient();
  if(millis()-ESP32_send_LastTime > ESP32_send_Rate) {
  webSocket.loop();
  ESP32_send_LastTime = millis();
  }
 }
