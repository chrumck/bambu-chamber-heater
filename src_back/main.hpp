#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "LittleFS.h"
#include <dhtnew.h>

#define SERIAL_BAUD_RATE 115200
#define SERIAL_BUFFER_SIZE 256
#define STRING_END_MARKER '\n'

#define PREFS_NAMESPACE "KnurToryTroller"
#define PREFS_KEY_WIFI_SSID "wifiSsid"
#define PREFS_KEY_WIFI_PASS "wifiPass"

#define LOOP_INTERVAL_MS 2500

#define LIGHT_PIN 13
#define AUX_FAN_PIN 12
#define DOOR_FAN_PIN 14
#define HEATER_FAN_PIN 27
#define HEATER_PIN 26

#define DHT_PIN 23
#define HEATER_TEMP_PIN 36
#define REF_VOLTAGE_PIN 39

#define DHT_MAX_FAIL_COUNT 5
#define TEMP_ERROR_VALUE (-WS_MESSAGE_TEMP_OFFSET)

#define ANALOG_READ_CONVERSION_FACTOR 0.0048828125

#define ANALOG_READ_COUNT 100

#define HEATER_REF_R 4700
#define HEATER_REF_V_MIN 4.8
#define HEATER_REF_V_MAX 5.2

#define HEATER_R_MAX 200000
#define HEATER_R_MIN 120
#define HEATER_R_FAN_ON 35899 // 50 degC
#define HEATER_R_DEADBAND 1000
#define HEATER_R_ON 4410  // 115 degC
#define HEATER_R_OFF 3340  // 125 degC
// #define HEATER_R_ON 7784 // 95 degC
// #define HEATER_R_ON 5070 // 110 degC
// #define HEATER_R_OFF 5070 // 110 degC
// #define HEATER_R_OFF 3850 // 120 degC

#define DEFAULT_TEMP_SET 30
#define CHAMBER_TEMP_ON_DEADBAND 0.3
#define AUX_FAN_ON_TEMP 1.0

#define WS_MESSAGE_LENGTH 9
#define WS_MESSAGE_TEMP_FACTOR 100
#define WS_MESSAGE_TEMP_OFFSET 50

enum WsMessageBytes {
  Byte_Temp_1 = 0,
  Byte_Temp_2 = 1,
  Byte_TempSet = 2,
  Byte_HeaterOnTimeLeftMins1 = 3,
  Byte_HeaterOnTimeLeftMins2 = 4,
  Byte_HeaterR_1 = 5,
  Byte_HeaterR_2 = 6,
  Byte_HeaterDutyCycle = 7,
  Byte_Flags = 8,
};

enum WsMessageFlags {
  Flag_HeaterOn = 0,
  Flag_LightOn = 1,
  Flag_HeaterFanSet = 2,
  Flag_HeaterFanOn = 3,
  Flag_DoorVentFanSet = 4,
  Flag_DoorVentFanOn = 5,
  Flag_AuxFanSet = 6,
  Flag_AuxFanOn = 7,
};

enum WsRequestCode {
  WsRequest_SetTemp = 0xa1,
  WsRequest_SetHeaterTimeLeft = 0xa2,
  WsRequest_SetLight = 0xa3,
  WsRequest_SetHeaterFan = 0xa4,
  WsRequest_SetDoorFan = 0xa5,
  WsRequest_SetAuxFan = 0xa6,
};

void initWifi(String ssid, String pass);
void initWebSocket();
void wsOnEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
void receiveSerial();
void savePrefs(const char* prefsKey, String prefsValue);
void readChamberTemp();
void readHeaterR();
void setHeater(bool on);
void controlHeater();
void controlHeaterFan();
void controlAuxFan();
void controlDoorFan();
void notifyWsClients();
