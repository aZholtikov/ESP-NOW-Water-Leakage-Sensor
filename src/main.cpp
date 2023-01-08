#include "ArduinoJson.h"
#include "ArduinoOTA.h"
#include "ESPAsyncWebServer.h"
#include "ZHNetwork.h"
#include "ZHConfig.h"

#define NORMAL
// #define LITE

void onConfirmReceiving(const uint8_t *target, const bool status);

void loadConfig(void);
void saveConfig(void);
void setupWebServer(void);

void sendConfigMessage(void);
void sendAttributesMessage(void);

const String firmware{"1.0"};

String espnowNetName{"DEFAULT"};

String deviceName{"ESP-NOW water sensor"};

char receivedBytes[128]{0};
uint8_t counter{0};
uint8_t messageLenght{0};
bool dataReceiving{false};
bool dataReceived{false};
bool semaphore{false};
uint8_t count{0};

esp_now_payload_data_t outgoingData{ENDT_SENSOR, ENPT_STATE};
StaticJsonDocument<sizeof(esp_now_payload_data_t::message)> json;
char buffer[sizeof(esp_now_payload_data_t::message)]{0};
char temp[sizeof(esp_now_payload_data_t)]{0};
const char initialMessage[] = {0x55, 0xAA, 0x00, 0x01, 0x00, 0x00, 0x00};
const char connectedMessage[] = {0x55, 0xAA, 0x00, 0x02, 0x00, 0x01, 0x04, 0x06};
const char settingMessage[] = {0x55, 0xAA, 0x00, 0x04, 0x00, 0x00, 0x03};
const char confirmationMessage[] = {0x55, 0xAA, 0x00, 0x05, 0x00, 0x01, 0x00, 0x05};

ZHNetwork myNet;
AsyncWebServer webServer(80);

void setup()
{
#if defined(NORMAL)
  Serial.begin(9600);

  SPIFFS.begin();

  loadConfig();

  myNet.begin(espnowNetName.c_str());

  myNet.setOnConfirmReceivingCallback(onConfirmReceiving);

  sendConfigMessage();
  sendAttributesMessage();

  Serial.write(initialMessage, sizeof(initialMessage));
  Serial.flush();
#endif
#if defined(LITE)
  myNet.begin(espnowNetName.c_str());

  myNet.setOnConfirmReceivingCallback(onConfirmReceiving);

  sendConfigMessage();
  sendAttributesMessage();

  json["state"] = "ALARM";
  serializeJsonPretty(json, buffer);
  memcpy(outgoingData.message, buffer, sizeof(esp_now_payload_data_t::message));
  memcpy(temp, &outgoingData, sizeof(esp_now_payload_data_t));
  myNet.sendBroadcastMessage(temp);
#endif
}

void loop()
{
#if defined(NORMAL)
  if (Serial.available() > 0 && !dataReceived)
  {
    char receivedByte[1];
    Serial.readBytes(receivedByte, 1);
    if (receivedByte[0] == 0x55)
    {
      dataReceiving = true;
      receivedBytes[counter++] = receivedByte[0];
      return;
    }
    if (dataReceiving)
    {
      if (counter == 5)
        messageLenght = 6 + int(receivedByte[0]);
      if (counter == messageLenght)
      {
        receivedBytes[counter] = receivedByte[0];
        counter = 0;
        dataReceiving = false;
        dataReceived = true;
        return;
      }
      receivedBytes[counter++] = receivedByte[0];
    }
  }
  if (dataReceived)
  {
    if (receivedBytes[3] == 0x01)
    {
      Serial.write(connectedMessage, sizeof(connectedMessage));
      Serial.flush();
      dataReceived = false;
    }
    if (receivedBytes[3] == 0x02)
      dataReceived = false;
    if (receivedBytes[3] == 0x04)
    {
      Serial.write(settingMessage, sizeof(settingMessage));
      Serial.flush();
      Serial.end();
      dataReceived = false;
      WiFi.softAP(("ESP-NOW Water " + myNet.getNodeMac()).c_str(), "12345678", 1, 0);
      setupWebServer();
      ArduinoOTA.begin();
    }
    if (receivedBytes[3] == 0x05)
    {
      if (receivedBytes[6] == 0x03)
      {
        Serial.write(confirmationMessage, sizeof(confirmationMessage));
        Serial.flush();
        dataReceived = false;
      }
      if (receivedBytes[6] == 0x01)
      {
        if (receivedBytes[10] == 0x00)
          json["state"] = "ALARM";
        if (receivedBytes[10] == 0x01)
          json["state"] = "DRY";
        dataReceived = false;
        memcpy(outgoingData.message, buffer, sizeof(esp_now_payload_data_t::message));
        memcpy(temp, &outgoingData, sizeof(esp_now_payload_data_t));
        myNet.sendBroadcastMessage(temp);
        semaphore = true;
      }
    }
  }
#endif
  myNet.maintenance();
  ArduinoOTA.handle();
}

void onConfirmReceiving(const uint8_t *target, const bool status)
{
#if defined(NORMAL)
  if (semaphore)
  {
    Serial.write(confirmationMessage, sizeof(confirmationMessage));
    Serial.flush();
    ESP.deepSleep(0);
  }
#endif
#if defined(LITE)
  ++count;
  if (count == 3)
    ESP.deepSleep(0);
#endif
}

void loadConfig()
{
  if (!SPIFFS.exists("/config.json"))
    saveConfig();
  File file = SPIFFS.open("/config.json", "r");
  String jsonFile = file.readString();
  StaticJsonDocument<512> json;
  deserializeJson(json, jsonFile);
  espnowNetName = json["espnowNetName"].as<String>();
  deviceName = json["deviceName"].as<String>();
  file.close();
}

void saveConfig()
{
  StaticJsonDocument<512> json;
  json["firmware"] = firmware;
  json["espnowNetName"] = espnowNetName;
  json["deviceName"] = deviceName;
  json["system"] = "empty";
  File file = SPIFFS.open("/config.json", "w");
  serializeJsonPretty(json, file);
  file.close();
}

void setupWebServer()
{
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(SPIFFS, "/index.htm"); });

  webServer.on("/setting", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        deviceName = request->getParam("deviceName")->value();
        espnowNetName = request->getParam("espnowNetName")->value();
        request->send(200);
        saveConfig(); });

  webServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        request->send(200);
        ESP.restart(); });

  webServer.onNotFound([](AsyncWebServerRequest *request)
                       { 
        if (SPIFFS.exists(request->url()))
        request->send(SPIFFS, request->url());
        else
        {
        request->send(404, "text/plain", "File Not Found");
        } });

  webServer.begin();
}

void sendConfigMessage()
{
  esp_now_payload_data_t outgoingData{ENDT_SENSOR, ENPT_CONFIG};
  StaticJsonDocument<sizeof(esp_now_payload_data_t::message)> json;
  json["name"] = deviceName;
  json["unit"] = 1;
  json["type"] = HACT_BINARY_SENSOR;
  json["class"] = HABSDC_MOISTURE;
  json["payload_on"] = "ALARM";
  json["payload_off"] = "DRY";
  char buffer[sizeof(esp_now_payload_data_t::message)]{0};
  serializeJsonPretty(json, buffer);
  memcpy(outgoingData.message, buffer, sizeof(esp_now_payload_data_t::message));
  char temp[sizeof(esp_now_payload_data_t)]{0};
  memcpy(&temp, &outgoingData, sizeof(esp_now_payload_data_t));
  myNet.sendBroadcastMessage(temp);
}

void sendAttributesMessage()
{
  esp_now_payload_data_t outgoingData{ENDT_SENSOR, ENPT_ATTRIBUTES};
  StaticJsonDocument<sizeof(esp_now_payload_data_t::message)> json;
  json["Type"] = "ESP-NOW Water Sensor";
  json["MCU"] = "ESP8266";
  json["MAC"] = myNet.getNodeMac();
  json["Firmware"] = firmware;
  json["Library"] = myNet.getFirmwareVersion();
  char buffer[sizeof(esp_now_payload_data_t::message)]{0};
  serializeJsonPretty(json, buffer);
  memcpy(outgoingData.message, buffer, sizeof(esp_now_payload_data_t::message));
  char temp[sizeof(esp_now_payload_data_t)]{0};
  memcpy(temp, &outgoingData, sizeof(esp_now_payload_data_t));
  myNet.sendBroadcastMessage(temp);
}
