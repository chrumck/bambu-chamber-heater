#include "./main.hpp"

Preferences prefs;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

volatile bool newWsMessage = false;

// All temperatures in DegC if otherwise not specified
float temp = TEMP_ERROR_VALUE;
u8_t tempSet = DEFAULT_TEMP_SET;
u32_t dhtFailCount = DHT_MAX_FAIL_COUNT;

float vRef = 0;
float heaterR = 0;
float heaterLastDutyCycle = 0.0;

// All times are in milliseconds from last boot if otherwise not specified
u32_t heaterOnMaxTime = 0;

bool heaterFanSet = false;
bool doorFanSet = false;
bool auxFanSet = false;

DHTNEW dht(DHT_PIN);

void setup() {
  Serial.setRxBufferSize(SERIAL_BUFFER_SIZE);
  Serial.begin(SERIAL_BAUD_RATE);

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

  prefs.begin(PREFS_NAMESPACE, false);
  String ssid = prefs.getString(PREFS_KEY_WIFI_SSID);
  String pass = prefs.getString(PREFS_KEY_WIFI_PASS);
  prefs.end();

  if (ssid.isEmpty() || pass.isEmpty()) {
    Serial.println("Wifi credentials not available, skipping setting up web server");
    return;
  }

  initWifi(ssid, pass);
  initWebSocket();
  if (!LittleFS.begin(true)) Serial.println("An error has occurred while mounting LittleFS");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
    });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}

void loop() {
  receiveSerial();

  static u32_t lastCycleTime = 0;

  u32_t currentTime = millis();
  if (!newWsMessage && currentTime - lastCycleTime < LOOP_INTERVAL_MS) return;

  newWsMessage = false;
  lastCycleTime = currentTime;

  readChamberTemp();
  readHeaterR();

  controlHeater();
  controlHeaterFan();
  controlAuxFan();
  controlDoorFan();

  notifyWsClients();
  ws.cleanupClients();
}

void initWifi(String ssid, String pass) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void initWebSocket() {
  Serial.println("Initializing web socket");
  ws.onEvent(wsOnEvent);
  server.addHandler(&ws);
}

void wsOnEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s \n", client->id(), client->remoteIP().toString().c_str());
    newWsMessage = true;
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected \n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void handleWebSocketMessage(void* arg, uint8_t* data, size_t length) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (!info->final || info->index != 0 || info->len != length || info->opcode != WS_BINARY) return;

  if (length != 2 && (data[0] == WsRequest_SetHeaterTimeLeft && length != 3)) {
    Serial.printf("Invalid ws request length of %d for requestcode: %x \n", length, data[0]);
    return;
  }

  switch (data[0]) {
  case WsRequest_SetTemp: {
    tempSet = data[1];
    Serial.printf("Setting chamber temp to %d \n", tempSet);
    break;
  }
  case WsRequest_SetHeaterTimeLeft: {
    u16_t timeLeftMins = data[1] | (data[2] << 8);
    heaterOnMaxTime = millis() + timeLeftMins * 60000;
    Serial.printf("Setting heater on time left to %d minutes \n", timeLeftMins);
    break;
  }
  case WsRequest_SetLight: {
    bool lightOn = data[1] == 1;
    Serial.printf("Setting light to %s \n", lightOn ? "ON" : "OFF");
    digitalWrite(LIGHT_PIN, lightOn ? LOW : HIGH);
    break;
  }
  case WsRequest_SetHeaterFan: {
    heaterFanSet = data[1] == 1;
    Serial.printf("Setting heater fan to %s \n", heaterFanSet ? "ON" : "OFF");
    break;
  }
  case WsRequest_SetDoorFan: {
    doorFanSet = data[1] == 1;
    Serial.printf("Setting door vent fan to %s \n", doorFanSet ? "ON" : "OFF");
    break;
  }
  case WsRequest_SetAuxFan: {
    auxFanSet = data[1] == 1;
    Serial.printf("Setting aux fan to %s \n", auxFanSet ? "ON" : "OFF");
    break;
  }
  }

  newWsMessage = true;
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

void savePrefs(const char* prefsKey, String prefsValue) {
  prefs.begin(PREFS_NAMESPACE, false);
  prefs.putString(prefsKey, prefsValue);
  prefs.end();

  Serial.printf("Saved key: %s, value: %s \n", prefsKey, prefsValue.c_str());
  Serial.println("You need to reboot for changes to take effect");
}

void readChamberTemp() {
  int chamberTempReadResult = dht.read();

  if (chamberTempReadResult == DHTLIB_OK) {
    dhtFailCount = 0;
    temp = dht.getTemperature();
    return;
  }

  Serial.printf("Failed chamber temp read, result: %d \n", chamberTempReadResult);

  dhtFailCount++;

  if (dhtFailCount >= DHT_MAX_FAIL_COUNT) {
    Serial.println("Too many failed chamber temp reads, setting temp value to invalid");
    temp = TEMP_ERROR_VALUE;
    setHeater(false);
  }
}

void readHeaterR() {
  vRef = 0;
  heaterR = 0;

  for (int i = 0; i < ANALOG_READ_COUNT; i++) {
    float currentVRef = (analogRead(REF_VOLTAGE_PIN) * ANALOG_READ_CONVERSION_FACTOR * 2);
    if (currentVRef < HEATER_REF_V_MIN || currentVRef > HEATER_REF_V_MAX) {
      Serial.printf("Reference voltage out of bounds: %.2f \n", currentVRef);
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
    Serial.printf("Heater temperature resistance out of bounds: %.0f \n", heaterR);
    heaterR = 0;
    setHeater(false);
  }
}

void controlHeater() {
  bool heaterOn = digitalRead(HEATER_PIN) == LOW;
  if (heaterR == 0 || temp == TEMP_ERROR_VALUE) {
    if (heaterOn) setHeater(false);
    return;
  }

  if (heaterOn && (heaterR < HEATER_R_OFF || temp > tempSet)) {
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
    temp < (tempSet - CHAMBER_TEMP_ON_DEADBAND)
    ) {
    setHeater(true);
  }
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
    Serial.printf("Last duty cycle: %f \n", heaterLastDutyCycle);
  }

  heaterLastTimeOff = currentTime;
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
  Serial.printf("Switching heater fan %s \n", fanOn ? "OFF" : "ON");
}

void controlAuxFan() {
  bool auxFanOn = digitalRead(AUX_FAN_PIN) == LOW;

  if (!auxFanOn && auxFanSet) {
    Serial.println("Aux fan requested by user, switching aux fan ON");
    digitalWrite(AUX_FAN_PIN, LOW);
    return;
  }

  if (temp == TEMP_ERROR_VALUE) {
    if (auxFanOn) return;
    Serial.println("Chamber temp unknown, switching aux fan ON");
    digitalWrite(AUX_FAN_PIN, LOW);
    return;
  }

  float auxFanTemp = temp - AUX_FAN_ON_TEMP;

  if (!auxFanOn && auxFanTemp > tempSet) {
    Serial.println("Chamber temp too high, switching aux fan ON");
    digitalWrite(AUX_FAN_PIN, LOW);
    return;
  }

  if (auxFanOn && !auxFanSet && auxFanTemp < tempSet - CHAMBER_TEMP_ON_DEADBAND) {
    Serial.println("Chamber temp below set, switching aux fan OFF");
    digitalWrite(AUX_FAN_PIN, HIGH);
  }
}

void controlDoorFan() {
  bool fanOn = digitalRead(DOOR_FAN_PIN) == LOW;
  bool auxFanOn = digitalRead(AUX_FAN_PIN) == LOW;

  bool shouldBeOn = doorFanSet || auxFanOn;

  if (fanOn == shouldBeOn) return;

  Serial.printf("Switching door fan %s \n", fanOn ? "OFF" : "ON");
  digitalWrite(DOOR_FAN_PIN, fanOn ? HIGH : LOW);
}

void notifyWsClients() {
  static u8_t wsMessage[WS_MESSAGE_LENGTH];
  memset(&wsMessage, 0, WS_MESSAGE_LENGTH);

  u16_t tempBytes = (u16_t)((temp + WS_MESSAGE_TEMP_OFFSET) * WS_MESSAGE_TEMP_FACTOR);
  wsMessage[Byte_Temp_1] = tempBytes & 0xFF;
  wsMessage[Byte_Temp_2] = (tempBytes >> 8) & 0xFF;
  wsMessage[Byte_TempSet] = tempSet;

  u32_t currentTime = millis();
  u16_t heaterTimeLeftMins = heaterOnMaxTime > currentTime ? (heaterOnMaxTime - currentTime) / 60000 : 0;
  wsMessage[Byte_HeaterOnTimeLeftMins1] = heaterTimeLeftMins & 0xFF;
  wsMessage[Byte_HeaterOnTimeLeftMins2] = (heaterTimeLeftMins >> 8) & 0xFF;

  u16_t heaterRBytes = (u16_t)heaterR;
  wsMessage[Byte_HeaterR_1] = heaterRBytes & 0xFF;
  wsMessage[Byte_HeaterR_2] = (heaterRBytes >> 8) & 0xFF;

  wsMessage[Byte_HeaterDutyCycle] = (u8_t)(heaterLastDutyCycle * 255);

  bool heaterOn = digitalRead(HEATER_PIN) == LOW;
  bool lightOn = digitalRead(LIGHT_PIN) == LOW;
  bool heaterFanOn = digitalRead(HEATER_FAN_PIN) == LOW;
  bool doorFanOn = digitalRead(DOOR_FAN_PIN) == LOW;
  bool auxFanOn = digitalRead(AUX_FAN_PIN) == LOW;

  wsMessage[Byte_Flags] |=
    (heaterOn << Flag_HeaterOn) |
    (lightOn << Flag_LightOn) |
    (heaterFanSet << Flag_HeaterFanSet) |
    (heaterFanOn << Flag_HeaterFanOn) |
    (doorFanSet << Flag_DoorVentFanSet) |
    (doorFanOn << Flag_DoorVentFanOn) |
    (auxFanSet << Flag_AuxFanSet) |
    (auxFanOn << Flag_AuxFanOn);

  ws.binaryAll(wsMessage, WS_MESSAGE_LENGTH);

  Serial.printf(
    "Temp: %.2f, Set:%d, TimeLeftMins:%d, Vref:%.2f, heaterR:%d, DutyCycle:%.2f, heaterOn:%d, lightOn: %d, heaterFanSet:%d, heaterFanOn:%d, doorFanSet:%d, doorFanOn:%d, auxFanSet:%d, auxFanOn:%d \n",
    temp, tempSet, heaterTimeLeftMins, vRef, (u16_t)heaterR, heaterLastDutyCycle, heaterOn, lightOn, heaterFanSet, heaterFanOn, doorFanSet, doorFanOn, auxFanSet, auxFanOn);
}


