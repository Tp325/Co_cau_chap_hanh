#ifndef Communication_h
#define Communication_h
#include "config.h"
#include <driver/uart.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFiManager.h>
#include <strings_en.h>
#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <PubSubClient.h>
#include "CircularQueue.h"

extern DynamicJsonDocument doc;
extern WiFiManager wm;
extern WiFiClient espClient;
extern PubSubClient mqttClient;

extern volatile bool receiveFlag;
extern void setReceiveFlag();
extern int state;
extern int trasmitState;
extern bool isSended;

extern bool isWifiConnect;


class Communication {
private:

  char* mqtt_server;
  int mqtt_port;
  char* mqtt_user;
  char* mqtt_pass;

  String clientmqtt;

  String topicReceive;
  String topicSend;

  String msgFromServer;
  String msgToServer;

  String buffMsgFromServer;
  String buffMsgToServer;


  unsigned long timeOutStartWeb = 0;
  unsigned long timeOutReconnectMQTT = 0;
  unsigned long timeOutReconnectWiFi = 0;
  bool isWebAPStart = 0;

  CircularQueue* buffDataFromServer = createQueue(25);
  CircularQueue* buffDataToServer = createQueue(25);

  void connectMqttWithTimeOut();
  void callbackmqtt(char* topic, byte* message, unsigned int length);
  static Communication* instance;
  static void callbackWrapper(char* topic, byte* payload, unsigned int length) {
    if (instance != nullptr) {
      instance->callbackmqtt(topic, payload, length);
    }
  }
  void reconnectWifi();
  void reconnectMQTT();
  void processWiFi();
  void processMQTT();
public:
  Communication(uint16_t SID = 5, String clientmqtt = "ghjksdjlgkwermnklmg",
                String topicReceive = "GIAOTHONG/Sub", String topicSend = "GIAOTHONG/Pub",
                char* mqtt_server = "broker.hivemq.com", int mqtt_port = 1883);
  void begin();
  void sendToServer();
  void sendToServer(String msg);
  void receiveFromServer();
  void analizeData();
  void process();
  void blocking();
  bool hasMSG();
  uint16_t SID;
  bool haveToReset;
};
#endif