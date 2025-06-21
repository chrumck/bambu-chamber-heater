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

Preferences prefs;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

volatile bool wsNewRequest = false;

float tempDegC = TEMP_ERROR_VALUE;
float tempSetDegC = DEFAULT_TEMP_SET;
u32_t dhtFailCount = DHT_MAX_FAIL_COUNT;

float vRef = 0;
float heaterR = 0;

// All times are in milliseconds from last boot if otherwise not specified
u32_t heaterOnMaxTime = 0;
float heaterLastDutyCycle = 0.0;

bool heaterFanSet = false;
bool doorFanSet = false;
bool auxFanSet = false;

DHTNEW dht(DHT_PIN);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setRxBufferSize(SERIAL_BUFFER_SIZE);

    prefs.begin(PREFS_NAMESPACE, false);
    String ssid = prefs.getString(PREFS_KEY_WIFI_SSID);
    String pass = prefs.getString(PREFS_KEY_WIFI_PASS);
    prefs.end();

    if (ssid.isEmpty() || pass.isEmpty()) {
        Serial.println("Wifi credentials not available, skipping setting up web server");
    }
    else {
        initWifi(ssid, pass);

        if (!LittleFS.begin(true)) {
            Serial.println("An error has occurred while mounting LittleFS");
        }
    }

    pinMode(LIGHT_PIN, OUTPUT);
    digitalWrite(LIGHT_PIN, LOW);  // Light on by default
    pinMode(AUX_FAN_PIN, OUTPUT);
    digitalWrite(AUX_FAN_PIN, HIGH);
    pinMode(DOOR_FAN_PIN, OUTPUT);
    digitalWrite(DOOR_FAN_PIN, HIGH);
    pinMode(HEATER_FAN_PIN, OUTPUT);
    digitalWrite(HEATER_FAN_PIN, HIGH);
    pinMode(HEATER_PIN, OUTPUT);
    digitalWrite(HEATER_PIN, HIGH);

    dht.reset();
}

void loop() {
    receiveSerial();

    static u32_t lastCycleTime = 0;

    u32_t currentTime = millis();
    if (wsNewRequest || currentTime - lastCycleTime < LOOP_INTERVAL_MS) return;

    wsNewRequest = false;
    lastCycleTime = currentTime;

    readChamberTemp();
    readHeaterR();

    controlHeater();
    controlHeaterFan();
    controlAuxFan();
    controlDoorFan();

    ws.cleanupClients();
}

void initWifi(String ssid, String pass) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
}

void receiveSerial() {
    static char serialBuf[SERIAL_BUFFER_SIZE];

    if (Serial.available() == 0) return;

    int idx = 0;
    memset(&serialBuf, 0, SERIAL_BUFFER_SIZE);
    while (Serial.available() > 0 && idx < SERIAL_BUFFER_SIZE) {
        char currentChar = Serial.read();
        if (currentChar == STRING_END_MARKER) break;
        serialBuf[idx++] = currentChar;
    }

    String message = String(serialBuf);

    if (message.startsWith("ssid ")) {
        savePrefs(PREFS_KEY_WIFI_SSID, message.substring(5));
        return;
    }

    if (message.startsWith("pass ")) {
        savePrefs(PREFS_KEY_WIFI_PASS, message.substring(5));
        return;
    }

    Serial.println("Command not recognized");
}

void savePrefs(char* prefsKey, String prefsValue) {
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putString(prefsKey, prefsValue);
    prefs.end();

    Serial.print("Saved key: ");
    Serial.print(prefsKey);
    Serial.print(" ,value: ");
    Serial.println(prefsValue.c_str());
    Serial.println("You need to reboot for changes to take effect");
}

void readChamberTemp() {
    int chamberTempReadResult = dht.read();

    if (chamberTempReadResult == DHTLIB_OK) {
        dhtFailCount = 0;
        tempDegC = dht.getTemperature();
        return;
    }

    Serial.print("Failed chamber temp read, result: ");
    Serial.println(chamberTempReadResult);

    dhtFailCount++;

    if (dhtFailCount >= DHT_MAX_FAIL_COUNT) {
        Serial.println("Too many failed chamber temp reads");
        tempDegC = TEMP_ERROR_VALUE;
        setHeater(false);
    }
}

void readHeaterR() {
    vRef = 0;
    heaterR = 0;

    for (int i = 0; i < ANALOG_READ_COUNT; i++) {
        float currentVRef = (analogRead(REF_VOLTAGE_PIN) * ANALOG_READ_CONVERSION_FACTOR * 2);
        if (currentVRef < HEATER_REF_V_MIN || currentVRef > HEATER_REF_V_MAX) {
            Serial.print("Reference voltage out of bounds: ");
            Serial.println(currentVRef);
            vRef = 0;
            heaterR = 0;
            setHeater(false);
            return;
        }

        vRef += currentVRef;

        float heaterV = (analogRead(HEATER_TEMP_PIN) * ANALOG_READ_CONVERSION_FACTOR);
        heaterR += heaterV * HEATER_REF_R / (currentVRef - heaterV);
    }

    vRef /= ANALOG_READ_COUNT;
    heaterR /= ANALOG_READ_COUNT;

    if (heaterR > HEATER_R_MAX || heaterR < HEATER_R_MIN) {
        Serial.print("Heater temperature resistance out of bounds: ");
        Serial.println(heaterR);
        heaterR = 0;
        setHeater(false);
    }
}

void controlHeater() {
    bool heaterOn = digitalRead(HEATER_PIN) == LOW;
    if (heaterR == 0 || tempDegC == TEMP_ERROR_VALUE) {
        if (heaterOn) setHeater(false);
        return;
    }

    if (heaterOn && (heaterR < HEATER_R_OFF || tempDegC > tempSetDegC)) {
        setHeater(false);
        return;
    }

    u32_t currentTime = millis();
    u32_t timeLeftToRunMs = currentTime > heaterOnMaxTime ? 0 : heaterOnMaxTime - currentTime;

    if (heaterOn && timeLeftToRunMs == 0) {
        Serial.println("Max heater time reached");
        setHeater(false);
        return;
    }

    if (!heaterOn &&
        timeLeftToRunMs > 0 &&
        heaterR > HEATER_R_ON &&
        tempDegC < (tempSetDegC - CHAMBER_TEMP_ON_DEADBAND)
        ) {
        setHeater(true);
    }
}

void controlHeaterFan() {
    bool fanOn = digitalRead(HEATER_FAN_PIN) == LOW;

    if (heaterR == 0) {
        if (fanOn) return;
        Serial.println("Heater R unknown, switching heater fan ON");
        digitalWrite(HEATER_FAN_PIN, LOW);
        return;
    }

    bool heaterOn = digitalRead(HEATER_PIN) == LOW;
    u32_t currentTime = millis();
    u32_t timeLeftToRun = currentTime > heaterOnMaxTime ? 0 : heaterOnMaxTime - currentTime;

    bool shouldBeOn = heaterOn || heaterFanSet || timeLeftToRun > 0 || heaterR < HEATER_R_FAN_ON;
    if (fanOn && shouldBeOn) return;

    bool shouldBeOff = !heaterOn && !heaterFanSet && timeLeftToRun == 0 && heaterR > HEATER_R_FAN_ON + HEATER_R_DEADBAND;
    if (!fanOn && shouldBeOff) return;

    digitalWrite(HEATER_FAN_PIN, fanOn ? HIGH : LOW);
    Serial.print("Switching heater fan ");
    Serial.println(fanOn ? "OFF" : "ON");
}

void controlAuxFan() {
    bool auxFanOn = digitalRead(AUX_FAN_PIN) == LOW;

    if (!auxFanOn && auxFanSet) {
        Serial.println("Aux fan requested by user, switching aux fan ON");
        digitalWrite(AUX_FAN_PIN, LOW);
        return;
    }

    if (tempDegC == TEMP_ERROR_VALUE) {
        if (auxFanOn) return;
        Serial.println("Chamber temp unknown, switching aux fan ON");
        digitalWrite(AUX_FAN_PIN, LOW);
        return;
    }

    float auxFanTemp = tempDegC - AUX_FAN_ON_TEMP;

    if (!auxFanOn && auxFanTemp > tempSetDegC) {
        Serial.println("Chamber temp too high, switching aux fan ON");
        digitalWrite(AUX_FAN_PIN, LOW);
        return;
    }

    if (auxFanOn && !auxFanSet && auxFanTemp < tempSetDegC - CHAMBER_TEMP_ON_DEADBAND) {
        Serial.println("Chamber temp within bounds, switching aux fan OFF");
        digitalWrite(AUX_FAN_PIN, HIGH);
    }
}

void controlDoorFan() {
    bool doorFanOn = digitalRead(DOOR_FAN_PIN) == LOW;
    bool auxFanOn = digitalRead(AUX_FAN_PIN) == LOW;

    bool shouldBeOn = doorFanSet || auxFanOn;

    if (doorFanOn == shouldBeOn) return;

    Serial.print("Switching door fan ");
    Serial.println(doorFanOn ? "OFF" : "ON");
    digitalWrite(DOOR_FAN_PIN, doorFanOn ? HIGH : LOW);
}


void setHeater(bool on) {
    static u32_t heaterLastTimeOn = 0;
    static u32_t heaterLastTimeOff = 0;

    bool heaterOn = digitalRead(HEATER_PIN) == LOW;
    if (heaterOn == on) { return; }

    u32_t currentTime = millis();

    if (on) {
        Serial.println("Switching heater ON");
        digitalWrite(HEATER_PIN, LOW);
        heaterLastTimeOn = currentTime;
        return;
    }

    Serial.println("Switching heater OFF");
    digitalWrite(HEATER_PIN, HIGH);

    if (heaterLastTimeOff > 0 && heaterLastTimeOn > 0) {
        u32_t lastOffCycle = heaterLastTimeOn - heaterLastTimeOff;
        u32_t lastOnCycle = currentTime - heaterLastTimeOn;
        heaterLastDutyCycle = (float)lastOnCycle / (lastOffCycle + lastOnCycle);
        Serial.print("Last duty cycle:");
        Serial.println(heaterLastDutyCycle);
    }

    heaterLastTimeOff = currentTime;
}
