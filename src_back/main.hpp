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
#define HEATER_PIN 33

#define HEATER_TEMP_PIN 36
#define DHT_PIN 23
#define REF_VOLTAGE_PIN 22

#define DHT_MAX_FAIL_COUNT 5
#define TEMP_ERROR_VALUE -100

#define ANALOG_READ_CONVERSION_FACTOR 0.0048828125

#define ANALOG_READ_COUNT 100

#define HEATER_REF_R 4700
#define HEATER_REF_V_MIN 4.8
#define HEATER_REF_V_MAX 5.2

#define HEATER_R_MAX 200000
#define HEATER_R_MIN 120
#define HEATER_R_FAN_ON 35899 // 50 degC
#define HEATER_R_DEADBAND 1000

// #define HEATER_R_ON 7784 // 95 degC
// #define HEATER_R_ON 5070 // 110 degC
#define HEATER_R_ON 4410  // 115 degC
// #define HEATER_R_OFF 5070 // 110 degC
// #define HEATER_R_OFF 3850 // 120 degC
#define HEATER_R_OFF 3340  // 125 degC

#define DEFAULT_TEMP_SET 30
#define CHAMBER_TEMP_ON_DEADBAND 0.3
#define AUX_FAN_ON_TEMP 1.0

#define WS_MESSAGE_LENGTH 9
#define WS_MESSAGE_TEMP_FACTOR 100
#define WS_MESSAGE_TEMP_OFFSET 100

enum class WsMessageBytes {
  TempDegC1 = 0,
  TempDegC2 = 1,
  TempSetDegC = 2,
  HeaterOnTimeLeftMins1 = 3,
  HeaterOnTimeLeftMins2 = 4,
  HeaterR1 = 5,
  HeaterR2 = 6,
  HeaterDutyCycle = 7,
  Flags = 8,
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
