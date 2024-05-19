#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Данные для подключения к WiFi
const char* ssid = "Intersvyaz-F036";
const char* password = "73334026";

// Пины для управления устройствами
const int pumpPin = D1;
const int lightPin = D2;
const int fanPin = D3;

// Создаем веб-сервер на порту 80
ESP8266WebServer server(80);

// Настройка времени с помощью NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

// Переменные для хранения расписания
int lightOnHour = 6;
int lightOffHour = 18;

int pumpStartHour1 = 7;
int pumpStartMinute1 = 0;
int pumpDuration1 = 5; // В минутах

int pumpStartHour2 = 19;
int pumpStartMinute2 = 0;
int pumpDuration2 = 5; // В минутах

int fanPeriodicity = 2; // В часах
int fanDuration = 10; // В минутах

// Функция для установки состояния устройства
void setDeviceState(int pin, bool state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

// Функция для создания навигационного меню
String getNavMenu() {
  return "<nav><a href=\"/\">Главная</a> | <a href=\"/status\">Статус</a></nav>";
}

// Главная страница веб-сервера
void handleRoot() {
  String html = "<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\"><title>Управление устройствами</title></head><body>";
  html += getNavMenu();
  html += "<h1>Управление устройствами</h1>";
  html += "<form action=\"/set\" method=\"POST\">";
  html += "Время включения света (час): <input type=\"number\" name=\"lightOnHour\" value=\"" + String(lightOnHour) + "\"><br>";
  html += "Время выключения света (час): <input type=\"number\" name=\"lightOffHour\" value=\"" + String(lightOffHour) + "\"><br>";

  html += "Время начала первого полива (час): <input type=\"number\" name=\"pumpStartHour1\" value=\"" + String(pumpStartHour1) + "\"><br>";
  html += "Время начала первого полива (минута): <input type=\"number\" name=\"pumpStartMinute1\" value=\"" + String(pumpStartMinute1) + "\"><br>";
  html += "Длительность первого полива (минуты): <input type=\"number\" name=\"pumpDuration1\" value=\"" + String(pumpDuration1) + "\"><br>";

  html += "Время начала второго полива (час): <input type=\"number\" name=\"pumpStartHour2\" value=\"" + String(pumpStartHour2) + "\"><br>";
  html += "Время начала второго полива (минута): <input type=\"number\" name=\"pumpStartMinute2\" value=\"" + String(pumpStartMinute2) + "\"><br>";
  html += "Длительность второго полива (минуты): <input type=\"number\" name=\"pumpDuration2\" value=\"" + String(pumpDuration2) + "\"><br>";

  html += "Периодичность включения вентилятора (часов): <input type=\"number\" name=\"fanPeriodicity\" value=\"" + String(fanPeriodicity) + "\"><br>";
  html += "Длительность работы вентилятора (минуты): <input type=\"number\" name=\"fanDuration\" value=\"" + String(fanDuration) + "\"><br>";
  html += "<input type=\"submit\" value=\"Сохранить\">";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Страница статуса устройств
void handleStatus() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  String currentTime = String(currentHour) + ":" + (currentMinute < 10 ? "0" : "") + String(currentMinute);

  String lightStatus = (digitalRead(lightPin) == HIGH) ? "Включен" : "Выключен";
  String pumpStatus = (digitalRead(pumpPin) == HIGH) ? "Включен" : "Выключен";
  String fanStatus = (digitalRead(fanPin) == HIGH) ? "Включен" : "Выключен";

  String html = "<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\"><title>Статус устройств</title></head><body>";
  html += getNavMenu();
  html += "<h1>Статус устройств</h1>";
  html += "<p>Текущее время: " + currentTime + "</p>";
  html += "<p>Свет: " + lightStatus + "</p>";
  html += "<p>Полив: " + pumpStatus + "</p>";
  html += "<p>Вентилятор: " + fanStatus + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Обработка формы настроек
void handleSet() {
  if (server.hasArg("lightOnHour")) lightOnHour = server.arg("lightOnHour").toInt();
  if (server.hasArg("lightOffHour")) lightOffHour = server.arg("lightOffHour").toInt();

  if (server.hasArg("pumpStartHour1")) pumpStartHour1 = server.arg("pumpStartHour1").toInt();
  if (server.hasArg("pumpStartMinute1")) pumpStartMinute1 = server.arg("pumpStartMinute1").toInt();
  if (server.hasArg("pumpDuration1")) pumpDuration1 = server.arg("pumpDuration1").toInt();

  if (server.hasArg("pumpStartHour2")) pumpStartHour2 = server.arg("pumpStartHour2").toInt();
  if (server.hasArg("pumpStartMinute2")) pumpStartMinute2 = server.arg("pumpStartMinute2").toInt();
  if (server.hasArg("pumpDuration2")) pumpDuration2 = server.arg("pumpDuration2").toInt();

  if (server.hasArg("fanPeriodicity")) fanPeriodicity = server.arg("fanPeriodicity").toInt();
  if (server.hasArg("fanDuration")) fanDuration = server.arg("fanDuration").toInt();
  server.send(200, "text/html", "<h1>Настройки сохранены</h1>" + getNavMenu());
}

void setup() {
  // Инициализация последовательного порта
  Serial.begin(115200);

  // Инициализация пинов
  pinMode(pumpPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  // Подключение к WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Подключение к WiFi...");
  }
  Serial.println("WiFi подключен");

  // Вывод IP-адреса
  Serial.print("IP-адрес: ");
  Serial.println(WiFi.localIP());

  // Настройка времени через NTP
  timeClient.begin();
  timeClient.update();

  // Настройка обработчиков HTTP-запросов
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("HTTP-сервер запущен");
}

void loop() {
  server.handleClient();
  timeClient.update();

  // Получаем текущее время
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  // Управление светом
  if (currentHour >= lightOnHour && currentHour < lightOffHour) {
    setDeviceState(lightPin, true);
  } else {
    setDeviceState(lightPin, false);
  }

  // Управление поливом
  static bool pumpRunning1 = false;
  static unsigned long pumpStartTime1 = 0;
  static bool pumpRunning2 = false;
  static unsigned long pumpStartTime2 = 0;

  if (currentHour == pumpStartHour1 && currentMinute == pumpStartMinute1 && !pumpRunning1) {
    setDeviceState(pumpPin, true);
    pumpRunning1 = true;
    pumpStartTime1 = millis();
  }
  if (pumpRunning1 && millis() - pumpStartTime1 >= pumpDuration1 * 60000) {
    setDeviceState(pumpPin, false);
    pumpRunning1 = false;
  }

  if (currentHour == pumpStartHour2 && currentMinute == pumpStartMinute2 && !pumpRunning2) {
    setDeviceState(pumpPin, true);
    pumpRunning2 = true;
    pumpStartTime2 = millis();
  }
  if (pumpRunning2 && millis() - pumpStartTime2 >= pumpDuration2 * 60000) {
    setDeviceState(pumpPin, false);
    pumpRunning2 = false;
  }

  // Управление вентилятором
  static bool fanRunning = false;
  static unsigned long fanStartTime = 0;
  static unsigned long lastFanStart = 0;
  if (millis() - lastFanStart >= fanPeriodicity * 3600000) {
    setDeviceState(fanPin, true);
    fanRunning = true;
    fanStartTime = millis();
    lastFanStart = millis();
  }
  if (fanRunning && millis() - fanStartTime >= fanDuration * 60000) {
    setDeviceState(fanPin, false);
    fanRunning = false;
  }
}
