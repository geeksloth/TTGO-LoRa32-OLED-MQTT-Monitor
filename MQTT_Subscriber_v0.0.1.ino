#include <WiFi.h>
#include "SSD1306.h"
#include <PubSubClient.h>

#define msg_blue_cnt 5
const bool debug = false;
uint8_t ledPin = 16; // Onboard LED reference
SSD1306 display(0x3c, 5, 4); // instance for the OLED. Addr, SDA, SCL

const char* ssid = "your ssid goes here";
const char* password = "your wifi password";
const char* mqtt_server = "your mqtt broker ip"; #eg. 192.168.1.10
const char* mqtt_sub_topic = "#"; # use hash sign for all topics, or define a specific topic
String callbackMsg = "";

const uint8_t yellow_height = 14; // pixcels
const uint8_t blue_line_y = 13;
const uint8_t blue_line_height = 10; // default = 8
uint8_t blue_line_y_current = blue_line_y;
String msg_yellow = "";
String msg_blue_tmp = "";
String msg_blue[msg_blue_cnt] = {"", "", "", "", ""};

WiFiClient espClient;
PubSubClient client(espClient);

void refreshScreen(){
  display.clear();
  // draw yellow message
  display.drawString(0, 0, msg_yellow);

  //draw blue message lines
  int tmp_blue_line_y = blue_line_y;
  for (int i=0; i<msg_blue_cnt; i++){
    display.drawString(0, tmp_blue_line_y, msg_blue[i]);
    tmp_blue_line_y += blue_line_height;
    delay(50);
  }
  display.display();
}

void updateBlueLines(String msg){
  if (debug) Serial.println(msg);
  display.clear();
  display.drawString(0, 0, msg_yellow);
  
  int tmp_blue_line_y = blue_line_y;
  //update blue messages index
  for (int i=0; i<(msg_blue_cnt-1); i++){
    msg_blue[i] = msg_blue[i+1];
  }
  msg_blue[(msg_blue_cnt-1)] = msg;

  refreshScreen();
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  if (debug) Serial.println();
  msg_blue_tmp = "Connecting to " +String(ssid);
  updateBlueLines(msg_blue_tmp);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debug) Serial.print(".");
  }

  if (debug) Serial.println("");
  msg_blue_tmp = "WiFi connected";
  updateBlueLines(msg_blue_tmp);
  
  msg_blue_tmp = "IP: " + WiFi.localIP().toString();
  updateBlueLines(msg_blue_tmp);
}

void callback(char* topic, byte* message, unsigned int length) {
  if (debug) Serial.print("[");
  if (debug) Serial.print(topic);
  if (debug) Serial.print("]: ");
  callbackMsg = "";

  for (int i = 0; i < length; i++) {
    //if (debug) Serial.print((char)message[i]);
    callbackMsg += (char)message[i];
  }
  updateBlueLines(callbackMsg);
  
  /*
  if (String(topic) == "esp32/output") {
    if (debug) Serial.print("Changing output to ");
    if(messageTemp == "on"){
      if (debug) Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      if (debug) Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
  */
}

void reconnect() {
  while (!client.connected()) {
    msg_blue_tmp = "MQTT connecting...";
    updateBlueLines(msg_blue_tmp);
    if (client.connect("ESP8266Client")) {
      msg_blue_tmp = "connected: " + String(mqtt_server);
      client.subscribe(mqtt_sub_topic);
      msg_yellow = "Broker: " + String(mqtt_server);
      updateBlueLines(msg_blue_tmp);
    } else {
      msg_blue_tmp = "failed, rc=" + String(client.state());
      updateBlueLines(msg_blue_tmp);
      delay(5000);
    }
  }
}

void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH); // HIGH -> off, LOW -> on
    if (debug) Serial.begin(115200);
    display.init(); // initialise the OLED
    display.flipScreenVertically();

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    delay(1500);
    // display.setFont(ArialMT_Plain_24);
    // Set the origin of text to top left
    // display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    delay(1000);
    //msg_blue_tmp = "ready.";
    //updateBlueLines(msg_blue_tmp);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
