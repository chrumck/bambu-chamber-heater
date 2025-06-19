#include <Preferences.h>
#include <dhtnew.h>

#define SERIAL_BAUD_RATE 115200
#define SERIAL_BUFFER_SIZE 256

#define PREFS_NAMESPACE "KnurToryTroller"
#define PREFS_KEY_WIFI_SSID "wifiSsid"
#define PREFS_KEY_WIFI_PASS "wifiPass"

#define LOOP_INTERVAL_MS 2500
#define MAX_HEATER_TIME_MS 3600e3

#define DHT_PIN 34
#define DHT_MAX_FAIL_COUNT 5

#define ANALOG_READ_CONVERSION_FACTOR 0.0048828125

#define ANALOG_READ_COUNT 100
#define REF_VOLTAGE_PIN 23
#define HEATER_TEMP_PIN 35
#define HEATER_TEMP_REF_R 4700
#define HEATER_TEMP_REF_V_MIN 4.8
#define HEATER_TEMP_REF_V_MAX 5.2

#define HEATER_TEMP_R_MAX 200000
#define HEATER_TEMP_R_MIN 120
// #define HEATER_TEMP_R_ON 7784 // 95 degC
// #define HEATER_TEMP_R_ON 5070 // 110 degC
#define HEATER_TEMP_R_ON 4410 // 115 degC
// #define HEATER_TEMP_R_OFF 5070 // 110 degC
// #define HEATER_TEMP_R_OFF 3850 // 120 degC
#define HEATER_TEMP_R_OFF 3340 // 125 degC

#define DEFAULT_CHAMBER_TEMP_OFF 60.0
#define CHAMBER_TEMP_ON_DEADBAND 0.3

#define HEATER_RELAY_PIN 32

#define switchHeaterOff()\
    if (isHeaterOn) {\
        digitalWrite(HEATER_RELAY_PIN, HIGH);\
        lastTimeOff = currentTime;\
    }\

Preferences prefs;

u32_t maxHeaterTimeMs = MAX_HEATER_TIME_MS;
float chamberTempOff = DEFAULT_CHAMBER_TEMP_OFF;
u32_t lastCycleTime = millis();
int dhtReadFailCount = 0;
float chamberTempDegC = 0.0;

u32_t lastTimeOn = 0;
u32_t lastTimeOff = 0;

DHTNEW dht(DHT_PIN);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setRxBufferSize(SERIAL_BUFFER_SIZE);

    prefs.begin(PREFS_NAMESPACE, false);
    String wifiSsid = prefs.getString(PREFS_KEY_WIFI_SSID);
    String wifiPass = prefs.getString(PREFS_KEY_WIFI_PASS);
    prefs.end();

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, HIGH);

    dht.reset();
}

void loop() {
    receiveSerial();

    u32_t currentTime = millis();
    if (currentTime - lastCycleTime < LOOP_INTERVAL_MS) return;
    lastCycleTime = currentTime;

    bool isHeaterOn = digitalRead(HEATER_RELAY_PIN) == LOW;
    Serial.print(isHeaterOn ? "Heater is ON" : "Heater is OFF");

    int chamberTempReadResult = dht.read();
    if (chamberTempReadResult != DHTLIB_OK) {
        Serial.print("; Failed chamber temp read, result: ");
        Serial.print(chamberTempReadResult);

        dhtReadFailCount++;

        if (dhtReadFailCount >= DHT_MAX_FAIL_COUNT) {
            Serial.println("Too many failed chamber temp reads");
            switchHeaterOff();
            return;
        }
    }
    else {
        dhtReadFailCount = 0;
        chamberTempDegC = dht.getTemperature();
    }

    Serial.print("; chamber degC: ");
    Serial.print(chamberTempDegC);

    float vRefAvg = 0;
    float heaterTempR = 0;
    for (int i = 0; i < ANALOG_READ_COUNT; i++)
    {
        float vRef = (analogRead(REF_VOLTAGE_PIN) * ANALOG_READ_CONVERSION_FACTOR * 2);
        vRefAvg += vRef;

        if (vRef < HEATER_TEMP_REF_V_MIN || vRef > HEATER_TEMP_REF_V_MAX) {
            Serial.println();
            Serial.print("Reference voltage out of bounds: ");
            Serial.println(vRef);
            switchHeaterOff();
            return;
        }

        float heaterTempVoltage = (analogRead(HEATER_TEMP_PIN) * ANALOG_READ_CONVERSION_FACTOR);
        heaterTempR += heaterTempVoltage * HEATER_TEMP_REF_R / (vRef - heaterTempVoltage);
    }

    vRefAvg /= ANALOG_READ_COUNT;
    heaterTempR /= ANALOG_READ_COUNT;

    if (heaterTempR > HEATER_TEMP_R_MAX || heaterTempR < HEATER_TEMP_R_MIN) {
        Serial.println();
        Serial.print("Heater temperature resistance out of bounds: ");
        Serial.println(heaterTempR);
        switchHeaterOff();
        return;
    }

    Serial.print("; vRef: ");
    Serial.print(vRefAvg);

    Serial.print("; heater R: ");
    Serial.print(heaterTempR);

    u32_t timeLeftToRunMs = currentTime > maxHeaterTimeMs ? 0 : maxHeaterTimeMs - currentTime;
    Serial.print("; time left mins: ");
    Serial.print(timeLeftToRunMs / 60000);

    if (timeLeftToRunMs == 0) {
        if (isHeaterOn) {
            Serial.println("; Max heater time reached, stopping heater");
            switchHeaterOff();
        }
        else {
            Serial.println("; Max heater time reached, heater off");
        }

        return;
    }
    else {
        Serial.println();
    }

    if (isHeaterOn && (heaterTempR < HEATER_TEMP_R_OFF || chamberTempDegC > chamberTempOff)) {
        Serial.println("Switching heater OFF");
        switchHeaterOff();
        return;
    }

    if (!isHeaterOn && heaterTempR > HEATER_TEMP_R_ON && chamberTempDegC < (chamberTempOff - CHAMBER_TEMP_ON_DEADBAND)) {

        if (lastTimeOff > 0 && lastTimeOn > 0) {
            u32_t lastOffCycle = currentTime - lastTimeOff;
            u32_t lastOnCycle = lastTimeOff - lastTimeOn;
            float dutyCycle = (float)lastOnCycle / (lastOffCycle + lastOnCycle);
            Serial.print("Last duty cycle:");
            Serial.println(dutyCycle);
        }

        Serial.println("Switching heater ON");
        digitalWrite(HEATER_RELAY_PIN, LOW);
        lastTimeOn = currentTime;
    }
}

void receiveSerial() {
    static const char endMarker = '\n';
    static char serialBuf[SERIAL_BUFFER_SIZE];
    static uint16_t idx;
    static char currentChar;

    if (Serial.available() == 0) return;

    idx = 0;
    memset(&serialBuf, 0, SERIAL_BUFFER_SIZE);
    while (Serial.available() > 0 && idx < SERIAL_BUFFER_SIZE) {
        currentChar = Serial.read();
        if (currentChar == endMarker) break;
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
    Serial.println("Please reboot the device to apply changes");
    Serial.println("You need to reboot for changes to take effect");
}