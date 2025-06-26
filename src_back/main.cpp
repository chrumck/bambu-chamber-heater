#include "./main.hpp"

Preferences prefs;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

u32_t lightButtonPressTime = 0;
volatile bool loopRunRequested = false;

// All temperatures in DegC if otherwise not specified
float temp = TEMP_ERROR_VALUE;
u8_t tempSet = DEFAULT_TEMP_SET;
u8_t dhtFailCount = DHT_MAX_FAIL_COUNT;

float vRef = 0.0;
float heaterV = 0.0;
float heaterR = 0.0;
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
  switchRelayOn(LIGHT_PIN);  // Light on by default
  pinMode(AUX_FAN_PIN, OUTPUT);
  switchRelayOff(AUX_FAN_PIN);
  pinMode(DOOR_FAN_PIN, OUTPUT);
  switchRelayOff(DOOR_FAN_PIN);
  pinMode(HEATER_FAN_PIN, OUTPUT);
  switchRelayOff(HEATER_FAN_PIN);
  pinMode(HEATER_PIN, OUTPUT);
  switchRelayOff(HEATER_PIN);

  pinMode(LIGHT_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LIGHT_BUTTON_PIN), handleLightButtonPress, FALLING);

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
    AsyncWebServerResponse* response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
    });

  server.begin();
}

void loop() {
  receiveSerial();

  static u32_t lastCycleTime = 0;

  u32_t currentTime = millis();
  if (!loopRunRequested && currentTime - lastCycleTime < LOOP_INTERVAL_MS) return;

  loopRunRequested = false;
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

void IRAM_ATTR handleLightButtonPress() {
  u32_t currentTime = millis();
  bool buttonOn = digitalRead(LIGHT_BUTTON_PIN) == LOW;
  if (!buttonOn || currentTime - lightButtonPressTime < LIGHT_BUTTON_DEBOUNCE_MS) return;

  switchRelay(LIGHT_PIN, !isRelayOn(LIGHT_PIN));
  loopRunRequested = true;
  lightButtonPressTime = currentTime;
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
    loopRunRequested = true;
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
    heaterOnMaxTime = millis() + timeLeftMins * 60000 + 10000; // Add 10 seconds extra
    Serial.printf("Setting heater on time left to %d minutes \n", timeLeftMins);
    break;
  }
  case WsRequest_SetLight: {
    bool lightOn = data[1] == 1;
    Serial.printf("Setting light to %s \n", lightOn ? "ON" : "OFF");
    switchRelay(LIGHT_PIN, lightOn);
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

  loopRunRequested = true;
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

  if (chamberTempReadResult == DHTLIB_WAITING_FOR_READ) { return; }

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
  heaterV = 0;
  heaterR = 0;

  u16_t adcFailCount = 0;
  float vRefSum = 0;
  float heaterVSum = 0;

  for (int i = 0; i < ANALOG_READ_COUNT; i++) {
    float currentVRef = readAdcMilliVolts(REF_V_PIN) * REF_V_ADC_RATIO / 1000.0;
    if (currentVRef >= REF_V_MIN && currentVRef <= REF_V_MAX) {
      vRefSum += currentVRef;
    }
    else {
      adcFailCount++;
      vRefSum += REF_V_DEFAULT;
    }

    float currentHeaterV = readAdcMilliVolts(HEATER_V_PIN) / 1000.0;
    if (currentHeaterV >= HEATER_V_MIN && currentHeaterV <= HEATER_V_MAX) {
      heaterVSum += currentHeaterV;
    }
    else {
      adcFailCount++;
      heaterVSum += HEATER_V_DEFAULT;
    }

    if (adcFailCount >= ADC_MAX_FAIL_COUNT) {
      Serial.println("Too many ADC read failures, setting ADC data to 0");
      vRef = 0.0;
      heaterV = 0.0;
      heaterR = 0.0;
      setHeater(false);
      return;
    }
  }

  vRef = vRefSum / ANALOG_READ_COUNT;
  heaterV = heaterVSum / ANALOG_READ_COUNT;
  heaterR = heaterV * REF_R / (vRef - heaterV);

  if (heaterR > HEATER_R_MAX || heaterR < HEATER_R_MIN) {
    Serial.printf("Heater R out of bounds: %.0f \n", heaterR);
    heaterR = 0;
    setHeater(false);
  }
}

void controlHeater() {
  bool heaterOn = isRelayOn(HEATER_PIN);
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

  bool heaterOn = isRelayOn(HEATER_PIN);
  if (heaterOn == on) { return; }

  u32_t currentTime = millis();

  if (on) {
    Serial.println("Switching heater ON");
    switchRelayOn(HEATER_PIN);
    heaterLastTimeOn = currentTime;
    return;
  }

  Serial.println("Switching heater OFF");
  switchRelayOff(HEATER_PIN);

  if (heaterLastTimeOff > 0 && heaterLastTimeOn > 0) {
    u32_t lastOffCycle = heaterLastTimeOn - heaterLastTimeOff;
    u32_t lastOnCycle = currentTime - heaterLastTimeOn;
    heaterLastDutyCycle = (float)lastOnCycle / (lastOffCycle + lastOnCycle);
    Serial.printf("Last duty cycle: %f \n", heaterLastDutyCycle);
  }

  heaterLastTimeOff = currentTime;
}

void controlHeaterFan() {
  bool fanOn = isRelayOn(HEATER_FAN_PIN);

  if (heaterR == 0) {
    if (fanOn) return;
    Serial.println("Heater R unknown, switching heater fan ON");
    switchRelayOn(HEATER_FAN_PIN);
    return;
  }

  bool heaterOn = isRelayOn(HEATER_PIN);
  u32_t currentTime = millis();
  u32_t timeLeftToRun = currentTime > heaterOnMaxTime ? 0 : heaterOnMaxTime - currentTime;

  bool shouldBeOn = heaterOn || heaterFanSet || timeLeftToRun > 0 || (!fanOn && heaterR < HEATER_R_FAN_ON);
  if (fanOn && shouldBeOn) return;

  bool shouldBeOff = !heaterOn && !heaterFanSet && timeLeftToRun == 0 && heaterR > HEATER_R_FAN_ON + HEATER_R_DEADBAND;
  if (!fanOn && shouldBeOff) return;

  Serial.printf("Switching heater fan %s \n", fanOn ? "OFF" : "ON");
  switchRelay(HEATER_FAN_PIN, !fanOn);
}

void controlAuxFan() {
  bool fanOn = isRelayOn(AUX_FAN_PIN);

  if (auxFanSet || temp == TEMP_ERROR_VALUE) {
    if (fanOn) return;
    Serial.println("Switching aux fan ON");
    switchRelayOn(AUX_FAN_PIN);
    return;
  }

  float auxFanTemp = temp - AUX_FAN_ON_TEMP;
  if (!fanOn && auxFanTemp > tempSet) {
    Serial.println("Chamber temp too high, switching aux fan ON");
    switchRelayOn(AUX_FAN_PIN);
    return;
  }

  if (fanOn && auxFanTemp < tempSet - CHAMBER_TEMP_ON_DEADBAND) {
    Serial.println("Chamber temp low enough, switching aux fan OFF");
    switchRelayOff(AUX_FAN_PIN);
    return;
  }
}

void controlDoorFan() {
  bool fanOn = isRelayOn(DOOR_FAN_PIN);
  bool auxFanOn = isRelayOn(AUX_FAN_PIN);

  if (doorFanSet || auxFanOn || temp == TEMP_ERROR_VALUE) {
    if (fanOn) return;
    Serial.println("Switching door fan ON");
    switchRelayOn(AUX_FAN_PIN);
    return;
  }

  float doorFanTemp = temp - DOOR_FAN_ON_TEMP;
  if (!fanOn && doorFanTemp > tempSet) {
    Serial.println("Chamber temp too high, switching door fan ON");
    switchRelayOn(DOOR_FAN_PIN);
    return;
  }

  if (fanOn && doorFanTemp < tempSet - CHAMBER_TEMP_ON_DEADBAND) {
    Serial.println("Chamber temp low enough, switching door fan OFF");
    switchRelayOff(DOOR_FAN_PIN);
    return;
  }
}

void notifyWsClients() {
  static u8_t wsMessage[WS_MESSAGE_LENGTH];
  memset(&wsMessage, 0, WS_MESSAGE_LENGTH);

  u16_t tempBytes = (u16_t)((temp + WS_MESSAGE_TEMP_OFFSET) * WS_MESSAGE_TEMP_FACTOR);
  wsMessage[Byte_Temp_1] = tempBytes & 0xFF;
  wsMessage[Byte_Temp_2] = (tempBytes >> 8) & 0xFF;
  wsMessage[Byte_TempSet] = tempSet;

  u32_t currentTime = millis();
  u32_t heaterTimeLeftSeconds = heaterOnMaxTime > currentTime ? (heaterOnMaxTime - currentTime) / 1000 : 0;
  u16_t heaterTimeLeftMins = heaterTimeLeftSeconds / 60;
  wsMessage[Byte_HeaterOnTimeLeftMins1] = heaterTimeLeftMins & 0xFF;
  wsMessage[Byte_HeaterOnTimeLeftMins2] = (heaterTimeLeftMins >> 8) & 0xFF;

  u16_t heaterRBytes = heaterR > 0xFFFF ? 0xFFFF : (u16_t)heaterR;
  wsMessage[Byte_HeaterR_1] = heaterRBytes & 0xFF;
  wsMessage[Byte_HeaterR_2] = (heaterRBytes >> 8) & 0xFF;

  wsMessage[Byte_HeaterDutyCycle] = (u8_t)(heaterLastDutyCycle * 255);

  bool heaterOn = isRelayOn(HEATER_PIN);
  bool lightOn = isRelayOn(LIGHT_PIN);
  bool heaterFanOn = isRelayOn(HEATER_FAN_PIN);
  bool doorFanOn = isRelayOn(DOOR_FAN_PIN);
  bool auxFanOn = isRelayOn(AUX_FAN_PIN);

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
    "Temp: %.2f, Set:%d, TimeLeftSec:%d, vRef:%.4f, heaterV:%.4f, heaterR:%d, DutyCycle:%.2f, heaterOn:%d, lightOn: %d, heaterFanSet:%d, heaterFanOn:%d, doorFanSet:%d, doorFanOn:%d, auxFanSet:%d, auxFanOn:%d \n",
    temp, tempSet, heaterTimeLeftSeconds, vRef, heaterV, (u32_t)heaterR, heaterLastDutyCycle, heaterOn, lightOn, heaterFanSet, heaterFanOn, doorFanSet, doorFanOn, auxFanSet, auxFanOn);
}


