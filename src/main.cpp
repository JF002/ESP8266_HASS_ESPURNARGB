#include <ESP8266wifi.h>
#include <PubSubClient.h>

#include <ArduinoJson.h>
#include "EspurnaLight.hpp"

extern "C" {
#include "user_interface.h"
}


void printWifiStatus();
void GetUniqueId();
void BuildTopicNames();

String uniqueId;
WiFiClient espClient;
PubSubClient client(espClient);
int cpt;

String topic_rgb_color;
String topic_rgb_temperature;
String topic_rgb_brightness;
String topic_rgb_white;

String topic_rgb_color_state;
String topic_rgb_temperature_state;
String topic_rgb_brightness_state;
String topic_rgb_white_state;

String topic_diag_freeMemory;
String topic_diag_uptime;
uint32_t uptime = 0;
EspurnaLight espurnaLight;

bool isModified = false;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char buffer[length+1];
  memcpy(buffer, payload, length);
  buffer[length] = '\0';
  String value = (String((char*)buffer));

  if(String(topic) == String(topic_rgb_color.c_str())) {
    espurnaLight.SetColor(value);
    isModified = true;
  }

  if(String(topic) == String(topic_rgb_temperature.c_str())) {
    Serial.println(value.c_str());
    espurnaLight.SetTemperature(value);
    isModified = true;
  }

  if(String(topic) == String(topic_rgb_white.c_str())) {
    Serial.println(value.c_str());
    espurnaLight.SetWhite(value);
    isModified = true;
  }

  if(String(topic) == String(topic_rgb_brightness.c_str())) {
    Serial.println(value.c_str());
    espurnaLight.SetBrightness(value);
    isModified = true;
  }

  espurnaLight.PrintStatus();



/*
  if (String(topic) == String(topic_rgbLedCommand.c_str())) {
    StaticJsonBuffer<200> jsonBuffer;
    Serial.println(String((char*)buffer).c_str());
    JsonObject& root = jsonBuffer.parseObject(buffer);
    if (root.success()) {
      jsonRgbLed.Apply(root);
    }
  }
  */
}

void stationModeConnectedHandler(const WiFiEventStationModeConnected & event) {
  Serial.println("onStationModeConnected");
}

void stationModeDisconnectedHandler(const WiFiEventStationModeDisconnected& event) {
  Serial.println("onStationModeDisconnected");
  int status = WiFi.status();
  Serial.println("Wifi status : " + String(status));
}

void stationModeAuthModeChangedHandler(const WiFiEventStationModeAuthModeChanged& event) {
  Serial.println("onStationModeAuthModeChanged");
}

void stationModeGotIPHandler(const WiFiEventStationModeGotIP& event) {
  Serial.println("onStationModeGotIP");
}

WiFiEventHandler stationConnectedHandler_;
WiFiEventHandler stationDisconnectedHandler_;
WiFiEventHandler stationModeAuthModeChangedHandler_;
WiFiEventHandler stationModeGotIPHandler_;

void setup() {
  Serial.begin(921600);

  stationConnectedHandler_ = WiFi.onStationModeConnected(&stationModeConnectedHandler);
  stationDisconnectedHandler_ = WiFi.onStationModeDisconnected(&stationModeDisconnectedHandler);
  stationModeAuthModeChangedHandler_ = WiFi.onStationModeAuthModeChanged(&stationModeAuthModeChangedHandler);
  stationModeGotIPHandler_ = WiFi.onStationModeGotIP(&stationModeGotIPHandler);

  Serial.println("Hello!");

  GetUniqueId();
  BuildTopicNames();

  Serial.println("Unique Id : " + uniqueId);
  Serial.println("Topic Diag - Free Memory : " +topic_diag_freeMemory);
  Serial.println("Topic Diag - Uptime : " + topic_diag_uptime);

  Serial.println("Setup finished");
  cpt = 0;
}

void DiagnosticLoop() {
  uptime++;
  client.publish(topic_diag_freeMemory.c_str(), String(system_get_free_heap_size()).c_str());
  client.publish(topic_diag_uptime.c_str(), String(uptime).c_str());
}

void ConnectMqttClient() {
  Serial.println("Connecting to MQTT borker");
  client.setServer("192.168.1.109", 1883);
  client.connect("WemosD1");
  client.setCallback(mqttCallback);
  client.subscribe(topic_rgb_color.c_str());
  client.subscribe(topic_rgb_temperature.c_str());
  client.subscribe(topic_rgb_white.c_str());
  client.subscribe(topic_rgb_brightness.c_str());
  DiagnosticLoop();
}

bool WifiStateMachine() {
  static bool isConnecting = false;
  bool ret = false;
  switch(WiFi. status()) {
    case WL_CONNECTED:
      if(isConnecting) {
        Serial.println("Wifi connection established");
        printWifiStatus();
      }
      isConnecting = false;
      ret =  true;
      break;
    case WL_IDLE_STATUS:
      break;
    case WL_NO_SSID_AVAIL:
    case WL_CONNECT_FAILED:
    case WL_DISCONNECTED:
      if(!isConnecting) {
        WiFi.mode(WIFI_STA);
        //WiFi.begin("OpenWrt", "mlkjhgfd");
        WiFi.begin("toto", "1234568789");
        isConnecting = true;
        Serial.println("Connecting to Wifi...");
      }
    default:
      break;
  }
  return ret;
}

void MqttClientStateMachine() {

}

void loop() {
  static bool previousWifiState = false;
  static bool previousMqttState = false;
  bool wifiConnected = WifiStateMachine();

  if(previousWifiState != wifiConnected) {
    if(wifiConnected) {
      Serial.println("Wifi is now Connected");
    }
    else {
      Serial.println("Wifi is now Disconnected");
    }
  }
  previousWifiState = wifiConnected;

  if(wifiConnected) {
    bool mqttConnected = client.connected();
    if(previousMqttState != mqttConnected) {
      if(mqttConnected) {
        Serial.println("MQTT is now Connected");
      }
      else {
        Serial.println("MQTT is now Disconnected");
      }
    }
    previousMqttState = mqttConnected;


    if(!mqttConnected) {
      if(cpt % 500 == 0) {
        ConnectMqttClient();
      }
    }

    if(mqttConnected) {
      client.loop();

      if(isModified) {
        EspurnaLight::Rgb color = espurnaLight.GetColor();
        String colorStr = String(color.r) + "," + String(color.g) + "," + String(color.b);
        client.publish(topic_rgb_color_state.c_str(), colorStr.c_str());
        client.loop();
        client.publish(topic_rgb_brightness_state.c_str(), String(espurnaLight.GetBrightness()).c_str());
        client.loop();
        client.publish(topic_rgb_white_state.c_str(), String(espurnaLight.GetWhite()).c_str());
        client.loop();
        client.publish(topic_rgb_temperature_state.c_str(), String(espurnaLight.GetTemperature()).c_str());
        client.loop();
        isModified = false;
      }

      if(cpt % 600 == 0) {
        DiagnosticLoop();
      }
    }
  }
  cpt++;
  delay(10);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void GetUniqueId() {
  byte mac[6];
  WiFi.macAddress(mac);

  uniqueId = "";
  uniqueId += String(mac[5], HEX) + ":" +
              String(mac[4], HEX) + ":" +
              String(mac[3], HEX) + ":" +
              String(mac[2], HEX) + ":" +
              String(mac[1], HEX) + ":" +
              String(mac[0], HEX);
}

void BuildTopicNames() {
  topic_rgb_color = "/light/AI_LIGHT_TEST/color";
  topic_rgb_temperature = "/light/AI_LIGHT_TEST/temperature";
  topic_rgb_brightness = "/light/AI_LIGHT_TEST/brightness";
  topic_rgb_white =  "/light/AI_LIGHT_TEST/white";

  topic_rgb_color_state = "/light/AI_LIGHT_TEST/color_state";
  topic_rgb_temperature_state = "/light/AI_LIGHT_TEST/temperature_state";
  topic_rgb_brightness_state = "/light/AI_LIGHT_TEST/brightness_state";
  topic_rgb_white_state =  "/light/AI_LIGHT_TEST/white_state";

  topic_diag_freeMemory = "/" + uniqueId + "/diag/freeMemory";
  topic_diag_uptime = "/" + uniqueId + "/diag/uptime";

}
