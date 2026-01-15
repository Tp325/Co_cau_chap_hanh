#include "Communication.h"
DynamicJsonDocument doc(1024);

WiFiManager wm;
WiFiClient espClient;
PubSubClient client(espClient);
Communication* Communication::instance = nullptr;
bool isWifiConnect = 0;

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("[WiFiEvent] Connected to WiFi!");
      isWifiConnect = 1;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("[WiFiEvent] WiFi lost, reconnecting");
      isWifiConnect = 0;
      break;
    default:
      break;
  }
}
Communication::Communication(uint16_t SID, String clientmqtt,
                             String topicReceive, String topicSend,
                             char* mqtt_server, int mqtt_port) {
  Communication::instance = this;
  this->SID = SID;
  this->clientmqtt = clientmqtt;

  this->topicReceive = topicReceive;
  this->topicSend = topicSend;

  this->mqtt_server = mqtt_server;
  this->mqtt_port = mqtt_port;
}
void Communication::process() {
  processWiFi();
  processMQTT();
}
void Communication::blocking() {
  reconnectWifi();
  reconnectMQTT();
}
void Communication::begin() {
  WiFi.onEvent(onWiFiEvent);
  WiFi.mode(WIFI_STA);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callbackWrapper);
  timeOutStartWeb = millis();
  wm.setConfigPortalBlocking(false);
  wm.setDarkMode(true);
  isWebAPStart = 1;
  if (wm.autoConnect("DEN GIAO THONG")) {
    Serial.println("WIFI connected");
  } else {
    Serial.println("Web AP running");
  }
}
bool Communication::hasMSG() {
  if (!isEmpty(buffDataToServer))
    return 1;
  else
    return 0;
}


void Communication::sendToServer() {
  while (!isEmpty(buffDataToServer)) {
    msgToServer = "";
    Serial.print("send to server: ");
    msgToServer = dequeue(buffDataToServer);
    Serial.println(msgToServer);
    client.publish(topicSend.c_str(), msgToServer.c_str());
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void Communication::sendToServer(String msg) {
  if (!isFull(buffDataToServer)) {
    enqueueData(buffDataToServer, msg.c_str());
  }
}
void Communication::receiveFromServer() {
  if (msgFromServer != "") {
    if (!isFull(buffDataFromServer)) {
      enqueueData(buffDataFromServer, msgFromServer.c_str());
    }
    Serial.print("receive From Server: ");
    Serial.println(msgFromServer);
    msgFromServer = "";
  }
}

void Communication::analizeData() {
  while (!isEmpty(buffDataFromServer)) {
    buffMsgFromServer = dequeue(buffDataFromServer);
    deserializeJson(doc, buffMsgFromServer);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void Communication::processWiFi() {
  if (millis() - timeOutStartWeb >= 1000 * 60 * 5 && isWebAPStart == 1) {
    wm.stopConfigPortal();
    isWebAPStart = 0;
  }
  if (isWebAPStart == 1) {
    wm.process();
  }
  if (isWifiConnect == 1 || !wm.getConfigPortalActive()) {
    isWebAPStart = 0;
  }
}
void Communication::reconnectWifi() {
  if (millis() - timeOutReconnectWiFi > 20000 && isWifiConnect == 0) {
    timeOutReconnectWiFi = millis();
    if (isWebAPStart == 0) {
      Serial.print("wait");
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}
void Communication::processMQTT() {
  if (isWifiConnect == 1 && client.connected()) {
    client.loop();
  }
}
void Communication::reconnectMQTT() {
  if (isWifiConnect == 1 && !client.connected()) {
    connectMqttWithTimeOut();
  }
}
void Communication::connectMqttWithTimeOut() {
  if (millis() - timeOutReconnectMQTT > 2000) {
    timeOutReconnectMQTT = millis();
    Serial.println("Attempting MQTT connection...");
    if (client.connect(clientmqtt.c_str())) {
      Serial.println("connected");
      client.subscribe(topicReceive.c_str());
    }
  } else {
    Serial.println("reconnect MQTT...");
  }
}

void Communication::callbackmqtt(char* topic, byte* message, unsigned int length) {
  msgFromServer = "";
  for (int i = 0; i < length; i++) {
    msgFromServer += (char)message[i];
  }
}