#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// CarePlus Smart Totem + Wearable Step Tracker
// OLED e MPU6050 usam barramentos I2C separados.
// OLED: SDA 25, SCL 26
// MPU6050: SDA 32, SCL 33

const int buttonPin = 18;
const int greenLedPin = 19;
const int redLedPin = 21;

// Barramento I2C do OLED
const int oledSdaPin = 25;
const int oledSclPin = 26;

// Barramento I2C separado do MPU6050
const int mpuSdaPin = 32;
const int mpuSclPin = 33;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

TwoWire mpuWire = TwoWire(1);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MPU6050 mpu;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "35.198.7.130";
const int mqttPort = 1883;
const char* mqttTopic = "/TEF/token001/attrs";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int pointsPerStep = 1;

const float stepThreshold = 0.23;
const unsigned long minStepInterval = 320;
const unsigned long sensorInterval = 120;
const unsigned long telemetryInterval = 5000;
const unsigned long mqttReconnectInterval = 3000;
const unsigned long debounceDelay = 700;

int validationCount = 0;
int steps = 0;
int pendingSteps = 0;
int tokenValue = 0;
int totalPoints = 0;
int batteryLevel = 100;

float lastAx = 0;
float lastAy = 0;
float lastAz = 0;
float lastMagnitudeG = 1.0;
bool stepArmed = true;

unsigned long lastStepTime = 0;
unsigned long lastSensorRead = 0;
unsigned long lastTelemetryPublish = 0;
unsigned long lastPressTime = 0;
unsigned long lastMqttReconnectAttempt = 0;
unsigned long eventNumber = 0;

bool mqttWasConnected = false;

void clearDisplayBase() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
}

void showBootScreen(const char* line) {
  clearDisplayBase();
  display.println("CarePlus Totem");
  display.println("----------------");
  display.println(line);
  display.display();
}

void showTrackingScreen() {
  clearDisplayBase();

  if (pendingSteps > 0) {
    display.println("Pronto validar");
  } else {
    display.println("CarePlus Totem");
  }

  display.println("----------------");
  display.print("Passos: ");
  display.println(steps);

  display.print("Pendentes: ");
  display.println(pendingSteps);

  display.print("Pontos: ");
  display.println(totalPoints);

  display.print("Bateria: ");
  display.print(batteryLevel);
  display.println("%");

  display.println();
  display.println("Botao valida");
  display.display();
}

void showNoStepsScreen() {
  clearDisplayBase();
  display.println("Caminhe antes");
  display.println("----------------");
  display.println("Nenhum passo");
  display.println("pendente.");
  display.println();
  display.println("Missao bloqueada");
  display.display();
}

void showValidatedScreen() {
  clearDisplayBase();
  display.println("MISSAO VALIDADA");
  display.println("----------------");

  display.print("Passos enviados: ");
  display.println(pendingSteps);

  display.print("+");
  display.print(tokenValue);
  display.println(" pontos");

  display.print("Total: ");
  display.println(totalPoints);

  display.display();
}

void showMqttErrorScreen() {
  clearDisplayBase();
  display.println("Erro MQTT");
  display.println("----------------");
  display.println("Verifique FIWARE");
  display.println("e broker MQTT");
  display.display();
}

void logPrefix() {
  Serial.print("[");
  Serial.print(millis() / 1000);
  Serial.print("s] ");
}

void logInfo(const String& message) {
  logPrefix();
  Serial.println(message);
}

String activityLevel() {
  if (pendingSteps >= 30) {
    return "high";
  }

  if (pendingSteps >= 10) {
    return "moderate";
  }

  if (pendingSteps > 0) {
    return "light";
  }

  return "idle";
}

String currentFlowState() {
  if (pendingSteps > 0) {
    return "ready_to_validate";
  }

  return "tracking";
}

String buildPayload(const String& state, int eventPoints) {
  String payload = "s|" + state;

  payload += "|p|" + String(validationCount);
  payload += "|st|" + String(steps);
  payload += "|ps|" + String(pendingSteps);
  payload += "|v|" + String(eventPoints);
  payload += "|tp|" + String(totalPoints);
  payload += "|b|" + String(batteryLevel);
  payload += "|r|" + String(WiFi.RSSI());
  payload += "|al|" + activityLevel();
  payload += "|ax|" + String(lastAx, 2);
  payload += "|ay|" + String(lastAy, 2);
  payload += "|az|" + String(lastAz, 2);

  return payload;
}

void printPayloadPretty(const String& title, const String& state, const String& payload) {
  Serial.println();
  Serial.println("========================================");
  Serial.println(title);
  Serial.println("========================================");

  Serial.print("Estado: ");
  Serial.println(state);

  Serial.print("Validacoes no totem: ");
  Serial.println(validationCount);

  Serial.print("Passos totais: ");
  Serial.println(steps);

  Serial.print("Passos pendentes: ");
  Serial.println(pendingSteps);

  Serial.print("Pontos do ultimo evento: ");
  Serial.println(tokenValue);

  Serial.print("Pontos totais: ");
  Serial.println(totalPoints);

  Serial.print("Nivel de atividade: ");
  Serial.println(activityLevel());

  Serial.print("Aceleracao X/Y/Z: ");
  Serial.print(lastAx, 2);
  Serial.print(" / ");
  Serial.print(lastAy, 2);
  Serial.print(" / ");
  Serial.println(lastAz, 2);

  Serial.print("Payload UltraLight: ");
  Serial.println(payload);

  Serial.println("========================================");
}

void connectWiFi() {
  showBootScreen("Conectando Wi-Fi");

  logInfo("Conectando ao Wi-Fi Wokwi-GUEST...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  logInfo("Wi-Fi conectado.");

  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

bool connectMQTT(bool showDetails = true) {
  if (mqttClient.connected()) {
    return true;
  }

  if (showDetails) {
    showBootScreen("Conectando MQTT");
    logInfo("Conectando ao broker MQTT...");
  }

  String clientId = "careplus-step-tracker-";
  clientId += String(random(0xffff), HEX);

  bool connected = mqttClient.connect(clientId.c_str());

  if (connected) {
    mqttWasConnected = true;

    if (showDetails) {
      logInfo("MQTT conectado.");
    }

    return true;
  }

  mqttWasConnected = false;

  logPrefix();
  Serial.print("Falha MQTT. Codigo: ");
  Serial.println(mqttClient.state());

  if (showDetails) {
    showMqttErrorScreen();
  }

  return false;
}

void waitForMQTT() {
  while (!connectMQTT(true)) {
    delay(2000);
  }
}

void publishTelemetry(const String& state, int eventPoints) {
  if (!mqttClient.connected()) {
    logInfo("MQTT desconectado antes da publicacao. Tentando reconectar...");

    if (!connectMQTT(false)) {
      logInfo("Nao foi possivel publicar: MQTT continua desconectado.");
      showMqttErrorScreen();
      return;
    }
  }

  String payload = buildPayload(state, eventPoints);
  bool published = mqttClient.publish(mqttTopic, payload.c_str());

  String title = "CAREPLUS TELEMETRIA | " + state;
  printPayloadPretty(title, state, payload);

  if (published) {
    logInfo("FIWARE/MQTT: telemetria publicada com sucesso.");
  } else {
    logInfo("FIWARE/MQTT: erro ao publicar telemetria.");
    showMqttErrorScreen();
  }
}

void readStepSensor() {
  if (millis() - lastSensorRead < sensorInterval) {
    return;
  }

  lastSensorRead = millis();

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  mpu.getEvent(&accel, &gyro, &temp);

  lastAx = accel.acceleration.x;
  lastAy = accel.acceleration.y;
  lastAz = accel.acceleration.z;

  float magnitudeG = sqrt(lastAx * lastAx + lastAy * lastAy + lastAz * lastAz) / 9.80665;
  float movement = abs(magnitudeG - lastMagnitudeG);

  lastMagnitudeG = magnitudeG;

  if (movement < 0.08) {
    stepArmed = true;
  }

  if (
    stepArmed &&
    movement > stepThreshold &&
    millis() - lastStepTime > minStepInterval
  ) {
    steps++;
    pendingSteps++;

    lastStepTime = millis();
    stepArmed = false;

    logPrefix();
    Serial.print("Passo detectado pelo MPU6050. Total: ");
    Serial.println(steps);

    showTrackingScreen();
  }
}

void validateMissionAtTotem() {
  if (pendingSteps == 0) {
    logInfo("Validacao bloqueada: caminhe antes de validar a missao.");

    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);

    showNoStepsScreen();
    publishTelemetry("no_steps", tokenValue);

    delay(1500);

    showTrackingScreen();

    while (digitalRead(buttonPin) == LOW) {
      delay(10);
    }

    return;
  }

  validationCount++;
  eventNumber++;

  tokenValue = pendingSteps * pointsPerStep;
  totalPoints += tokenValue;

  if (batteryLevel > 0) {
    batteryLevel--;
  }

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);

  showValidatedScreen();
  publishTelemetry("validated", tokenValue);

  pendingSteps = 0;

  delay(1500);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);

  showTrackingScreen();
  publishTelemetry(currentFlowState(), tokenValue);

  while (digitalRead(buttonPin) == LOW) {
    delay(10);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);

  // I2C do OLED
  Wire.begin(oledSdaPin, oledSclPin);

  // I2C separado do MPU6050
  mpuWire.begin(mpuSdaPin, mpuSclPin);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha ao iniciar o display OLED.");

    while (true) {
      delay(10);
    }
  }

  showBootScreen("Iniciando...");

  if (!mpu.begin(0x68, &mpuWire)) {
    Serial.println("Falha ao iniciar o MPU6050.");
    showBootScreen("Erro MPU6050");

    while (true) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println();
  Serial.println("========================================");
  Serial.println("CAREPLUS SMART TOTEM + STEP TRACKER");
  Serial.println("OLED e MPU6050 em barramentos I2C separados.");
  Serial.println("OLED SDA: GPIO 25 | OLED SCL: GPIO 26");
  Serial.println("MPU SDA: GPIO 32 | MPU SCL: GPIO 33");
  Serial.println("========================================");

  connectWiFi();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(15);

  waitForMQTT();

  showTrackingScreen();
  publishTelemetry(currentFlowState(), tokenValue);
}

void loop() {
  if (!mqttClient.connected()) {
    if (mqttWasConnected) {
      logInfo("MQTT desconectado. Tentando reconectar...");
      mqttWasConnected = false;
    }

    if (millis() - lastMqttReconnectAttempt > mqttReconnectInterval) {
      lastMqttReconnectAttempt = millis();

      if (connectMQTT(false)) {
        logInfo("MQTT reconectado.");
        showTrackingScreen();
      }
    }
  } else {
    mqttClient.loop();
  }

  readStepSensor();

  if (mqttClient.connected() && millis() - lastTelemetryPublish > telemetryInterval) {
    lastTelemetryPublish = millis();
    publishTelemetry(currentFlowState(), tokenValue);
  }

  if (digitalRead(buttonPin) == LOW && millis() - lastPressTime > debounceDelay) {
    lastPressTime = millis();

    logInfo("Totem detectou aproximacao. Validando passos pendentes...");
    validateMissionAtTotem();
  }
}