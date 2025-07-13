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

#define SET_TIME_EXTRA_MS 3000

#define LIGHT_BUTTON_PIN 16
#define LIGHT_BUTTON_DEBOUNCE_MS 1000

#define LIGHT_PIN 14
#define AUX_FAN_PIN 26
#define DOOR_FAN_PIN 12
#define HEATER_FAN_PIN 27
#define HEATER_PIN 13

#define DHT_PIN 32
#define HEATER_R_PIN 36
#define REF_V_PIN 39

#define DHT_MAX_FAIL_COUNT 5
#define TEMP_DEFAULT 30.0
#define TEMP_ERROR_VALUE (-WS_MESSAGE_TEMP_OFFSET)

#define ANALOG_READ_COUNT 100
#define ANALOG_READ_NUM 980
#define ANALOG_READ_DEN 1000
#define ANALOG_READ_OFFSET (-20)
#define ADC_MAX_FAIL_COUNT 10

#define readAdcMilliVolts(_pin) \
   (((int32_t)analogReadMilliVolts(_pin) * ANALOG_READ_NUM / ANALOG_READ_DEN) + ANALOG_READ_OFFSET)

#define isRelayOn(_pin) (digitalRead(_pin) == HIGH)
#define switchRelay(_pin, _on) digitalWrite(_pin, _on ? HIGH : LOW)
#define switchRelayOn(_pin) switchRelay(_pin, true)
#define switchRelayOff(_pin) switchRelay(_pin, false)

#define REF_R 4730
#define REF_V_DEFAULT 3.25
#define REF_V_MIN 3.0
#define REF_V_MAX 3.5
#define REF_V_ADC_RATIO 2.0

#define HEATER_V_MIN 0.05
#define HEATER_V_MAX 3.10
#define HEATER_V_DEFAULT 1.941 // corresponds to ~100 degC @ vRef = 3.3V
#define HEATER_R_MAX 200000
#define HEATER_R_MIN 120
#define HEATER_R_FAN_ON 20900 // 65 degC
#define HEATER_R_DEADBAND 5000
#define HEATER_R_ON 4410  // 115 degC
#define HEATER_R_OFF 3340  // 125 degC

// #define HEATER_R_ON 7784 // 95 degC
// #define HEATER_R_ON 5070 // 110 degC
// #define HEATER_R_OFF 5070 // 110 degC
// #define HEATER_R_OFF 3850 // 120 degC

#define DEFAULT_TEMP_SET 60
#define CHAMBER_TEMP_ON_DEADBAND 0.3
#define AUX_FAN_ON_TEMP 2.0
#define DOOR_FAN_ON_TEMP 1.0

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

void IRAM_ATTR handleLightButtonPress();
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
