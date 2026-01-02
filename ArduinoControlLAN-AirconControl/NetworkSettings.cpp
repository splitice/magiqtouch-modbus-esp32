#include "NetworkSettings.h"
#include <WiFi.h>
#include <WiFiClient.h>
//#include <ArduinoOTA.h>
#include "NetworkSecret.h"  // Contains ssid and password definitions

const char* ssid = WIFI_SSID;
const char* password = WIFI_SECRET;  // Change this to your WiFi password

namespace {
  // Keep the web server responsive by never blocking in DisconnectCheck().
  // WiFi reconnect attempts are handled as a lightweight state machine.
  bool reconnectInProgress = false;
  uint32_t reconnectStartMs = 0;
  uint32_t lastProgressLogMs = 0;
  uint32_t cooldownUntilMs = 0;

  constexpr uint32_t RECONNECT_GIVEUP_MS = 10'000;
  constexpr uint32_t PROGRESS_LOG_MS = 500;
  constexpr uint32_t RECONNECT_COOLDOWN_MS = 5'000;
}

void LanController::Setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Waiting for WiFi to Connect");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  //ArduinoOTA.setPassword("Otppass1234!");
  //ArduinoOTA.begin();
}

void LanController::DisconnectCheck() {
  const uint32_t now = millis();
  if (WiFi.status() == WL_CONNECTED) {
    if (reconnectInProgress) {
      Serial.println("\nWiFi reconnected");
    }
    reconnectInProgress = false;
    return;
  }

  if (now < cooldownUntilMs) {
    return;
  }

  if (!reconnectInProgress) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    reconnectInProgress = true;
    reconnectStartMs = now;
    lastProgressLogMs = now;
    // Start async reconnect; do not block here.
    WiFi.disconnect(false);
    WiFi.begin(ssid, password);
    return;
  }

  // Still reconnecting; periodically print progress without blocking.
  if (now - lastProgressLogMs >= PROGRESS_LOG_MS) {
    Serial.print(".");
    lastProgressLogMs = now;
  }

  // Give up after a bounded amount of time so we don't spam begin().
  if (now - reconnectStartMs >= RECONNECT_GIVEUP_MS) {
    Serial.println("\nWiFi reconnection failed");
    reconnectInProgress = false;
    cooldownUntilMs = now + RECONNECT_COOLDOWN_MS;
  }
}