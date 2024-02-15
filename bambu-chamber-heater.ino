#include <dhtnew.h>

#define SERIAL_BAUD_RATE 115200
#define LOOP_INTERVAL_MS 2500
#define MAX_HEATER_TIME_MS 43200e3

#define DHT_PIN 10
#define DHT_MAX_FAIL_COUNT 2

#define ANALOG_READ_CONVERSION_FACTOR 0.0048828125

#define ANALOG_READ_COUNT 100
#define REF_VOLTAGE_PIN A1
#define HEATER_TEMP_PIN A0
#define HEATER_TEMP_REF_R 4700

#define HEATER_TEMP_R_MAX 200000
#define HEATER_TEMP_R_MIN 120
#define HEATER_TEMP_R_ON 1770
#define HEATER_TEMP_R_OFF 896

#define CHAMBER_TEMP_OFF 60.0
#define CHAMBER_TEMP_ON_DEADBAND 1.0

#define HEATER_RELAY_PIN 21

u32 maxHeaterTimeMs = MAX_HEATER_TIME_MS;
float chamberTempOff = CHAMBER_TEMP_OFF;
u32 lastCycleTime = millis();
int dhtReadFailCount = 0;
float chamberTempDegC = 0.0;

DHTNEW dht(DHT_PIN);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, HIGH);

    dht.reset();
}

void loop() {
    receiveSerial();

    u32 currentTime = millis();
    if (currentTime - lastCycleTime < LOOP_INTERVAL_MS) return;
    lastCycleTime = currentTime;

    int chamberTempReadResult = dht.read();
    if (chamberTempReadResult != DHTLIB_OK) {
        Serial.print("Failed to read chamber temp, read result: ");
        Serial.println(chamberTempReadResult);

        dhtReadFailCount++;

        if (dhtReadFailCount >= DHT_MAX_FAIL_COUNT) {
            Serial.println("Too many failed chamber temp reads, stopping heater");
            digitalWrite(HEATER_RELAY_PIN, HIGH);
            return;
        }
    }
    else {
        dhtReadFailCount = 0;
        chamberTempDegC = dht.getTemperature();
    }

    Serial.print("Chamber temp degC: ");
    Serial.println(chamberTempDegC);

    float heaterTempR = 0;
    for (int i = 0; i < ANALOG_READ_COUNT; i++)
    {
        float vRef = (analogRead(REF_VOLTAGE_PIN) * ANALOG_READ_CONVERSION_FACTOR * 2);

        if (vRef < 4.9 || vRef > 5.1) {
            Serial.print("Reference voltage out of bounds: ");
            Serial.println(vRef);
            digitalWrite(HEATER_RELAY_PIN, HIGH);
            return;
        }

        float heaterTempVoltage = (analogRead(HEATER_TEMP_PIN) * ANALOG_READ_CONVERSION_FACTOR);
        heaterTempR += heaterTempVoltage * HEATER_TEMP_REF_R / (vRef - heaterTempVoltage);
    }

    heaterTempR /= ANALOG_READ_COUNT;

    if (heaterTempR > HEATER_TEMP_R_MAX || heaterTempR < HEATER_TEMP_R_MIN) {
        Serial.print("Heater temperature resistance out of bounds: ");
        Serial.println(heaterTempR);
        digitalWrite(HEATER_RELAY_PIN, HIGH);
        return;
    }

    Serial.print("Heater temp R: ");
    Serial.println(heaterTempR);

    bool isHeaterOn = digitalRead(HEATER_RELAY_PIN) == LOW;

    if (currentTime > maxHeaterTimeMs) {
        if (isHeaterOn) {
            Serial.println("Max heater time reached, stopping heater");
            digitalWrite(HEATER_RELAY_PIN, HIGH);
        }
        else {
            Serial.println("Max heater time reached, heater already off");
        }

        return;
    }

    Serial.println(isHeaterOn ? "Heater is ON" : "Heater is OFF");

    if (!isHeaterOn && heaterTempR > HEATER_TEMP_R_ON && chamberTempDegC < (chamberTempOff - CHAMBER_TEMP_ON_DEADBAND)) {
        Serial.println("Switching heater ON");
        digitalWrite(HEATER_RELAY_PIN, LOW);
        return;
    }

    if (isHeaterOn && (heaterTempR < HEATER_TEMP_R_OFF || chamberTempDegC > chamberTempOff)) {
        Serial.println("Switching heater OFF");
        digitalWrite(HEATER_RELAY_PIN, HIGH);
    }

}

void receiveSerial() {
    static const char endMarker = '\n';
    static char serialBuf[SERIAL_BUFFER_SIZE];
    static byte ndx;
    static char rc;

    if (Serial.available() == 0) return;

    ndx = 0;
    memset(&serialBuf, 0, SERIAL_BUFFER_SIZE);
    while (Serial.available() > 0 && ndx < SERIAL_BUFFER_SIZE) {
        rc = Serial.read();
        if (rc == endMarker) break;
        serialBuf[ndx++] = rc;
    }

    String message = String(serialBuf);

    if (message.length() < 6) {
        Serial.println("Invalid request");
        return;
    }

    if (message.startsWith("time ")) {
        maxHeaterTimeMs = (u32)message.substring(5).toInt() * 3600e3;
        Serial.print("new max heater time in hours: ");
        Serial.println(maxHeaterTimeMs / 3600e3);
    }

    if (message.startsWith("temp ")) {
        chamberTempOff = message.substring(5).toInt();
        Serial.print("new chamber max temp:");
        Serial.println(chamberTempOff);
    }
}
