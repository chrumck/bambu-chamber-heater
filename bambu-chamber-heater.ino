#include <dhtnew.h>

#define SERIAL_BAUD_RATE 115200
#define LOOP_INTERVAL_MS 3e3

#define DHT_PIN 10

#define ANALOG_READ_CONVERSION_FACTOR 0.0048828125

#define ANALOG_READ_COUNT 100
#define REF_VOLTAGE_PIN A1
#define HEATER_TEMP_PIN A0
#define HEATER_TEMP_REF_R 4700

#define HEATER_TEMP_R_MAX 200000
#define HEATER_TEMP_R_MIN 120
#define HEATER_TEMP_R_ON 1770
#define HEATER_TEMP_R_OFF 896

#define CHAMBER_TEMP_ON 60.0
#define CHAMBER_TEMP_OFF_DEADBAND 1.0

#define HEATER_RELAY_PIN 21

float chamberTempOn = CHAMBER_TEMP_ON;

DHTNEW dht(DHT_PIN);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, HIGH);

    dht.reset();
}

void loop() {
    static u32 lastCycleTime = millis();

    u32 currentTime = millis();
    if (currentTime - lastCycleTime < LOOP_INTERVAL_MS) return;
    lastCycleTime = currentTime;

    int chamberTempReadResult = dht.read();
    if (chamberTempReadResult != DHTLIB_OK) {
        Serial.print("Failed to read chamber temp, read result: ");
        Serial.println(chamberTempReadResult);
        digitalWrite(HEATER_RELAY_PIN, HIGH);
        return;
    }

    float chamberTempDegC = dht.getTemperature();

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

    Serial.println(isHeaterOn ? "Heater is ON" : "Heater is OFF");

    if (!isHeaterOn && heaterTempR > HEATER_TEMP_R_ON && chamberTempDegC < (chamberTempOn - CHAMBER_TEMP_OFF_DEADBAND)) {
        Serial.println("Switching heater ON");
        digitalWrite(HEATER_RELAY_PIN, LOW);
        return;
    }

    if (isHeaterOn && (heaterTempR < HEATER_TEMP_R_OFF || chamberTempDegC > chamberTempOn)) {
        Serial.println("Switching heater OFF");
        digitalWrite(HEATER_RELAY_PIN, HIGH);
    }

}

// void receiveSerial() {
//     static const char endMarker = '\n';
//     static char serialBuf[SERIAL_BUFFER_SIZE];
//     static byte ndx;
//     static char rc;

//     if (Serial.available() == 0) return;

//     ndx = 0;
//     memset(&serialBuf, 0, SERIAL_BUFFER_SIZE);
//     while (Serial.available() > 0 && ndx < SERIAL_BUFFER_SIZE) {
//         rc = Serial.read();
//         if (rc == endMarker) break;
//         serialBuf[ndx++] = rc;
//     }

//     if (strcmp(serialBuf, "temp up") == 0) {
//         Serial.println("starting...");
//         isStopped = 0;
//     }

//     if (strcmp(serialBuf, "stop") == 0) {
//         Serial.println("stopping...");
//         isStopped = 1;
//     };

//     if (strcmp(serialBuf, "faster") == 0) {
//         sendInterval = sendInterval / 2;
//         Serial.print("going faster, new interval:");
//         Serial.println(sendInterval);
//     };

//     if (strcmp(serialBuf, "slower") == 0) {
//         sendInterval = sendInterval * 2;
//         Serial.print("going slower, new interval:");
//         Serial.println(sendInterval);
//     };
// }
