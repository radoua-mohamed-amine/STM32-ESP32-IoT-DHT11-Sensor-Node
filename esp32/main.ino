/*  esp32_main.ino  —  ESP32 UART slave + MQTT publisher
 *
 *
 *  Libraries needed (Library Manager):
 *    - DHT sensor library   (Adafruit)
 *    - Adafruit Unified Sensor
 *    - PubSubClient         (Nick O'Leary)
 *    - ArduinoJson          (Benoit Blanchon)
 *
 *  Configure the 4 lines below then upload.
 */

#define WIFI_SSID    "your wifi"
#define WIFI_PASS    "your password"
#define MQTT_SERVER  "ip adress"
#define MQTT_PORT    1883

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

/* ── Hardware ──────────────────────────────────────────────────────── */
#define DHT_PIN  4
#define DHT_TYPE DHT11

HardwareSerial stm32(2);        /* UART2: RX=GPIO16, TX=GPIO17 */
DHT dht(DHT_PIN, DHT_TYPE);

/* ── MQTT ──────────────────────────────────────────────────────────── */
#define MQTT_TOPIC_SENSORS "stm32/sensors"
#define MQTT_TOPIC_STATUS  "stm32/status"
#define MQTT_CLIENT_ID     "esp32_node"

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

/* ── State ─────────────────────────────────────────────────────────── */
float    g_temp      = 0.0f;
float    g_hum       = 0.0f;
uint32_t g_last_send = 0;
String   stm_buf     = "";

/* ══════════════════════════════════════════════════════════════════════
 *  MQTT reconnect (non-blocking)
 * ══════════════════════════════════════════════════════════════════════*/
void mqttReconnect()
{
    if (mqtt.connected()) return;
    static uint32_t lastAttempt = 0;
    if (millis() - lastAttempt < 5000) return;
    lastAttempt = millis();

    Serial.print("MQTT connect... ");
    if (mqtt.connect(MQTT_CLIENT_ID)) {
        Serial.println("OK");
        mqtt.publish(MQTT_TOPIC_STATUS, "online", true);
    } else {
        Serial.printf("failed rc=%d\n", mqtt.state());
    }
}

/* ── Publish sensor data ───────────────────────────────────────────── */
void mqttPublish()
{
    StaticJsonDocument<64> doc;
    doc["temperature"] = serialized(String(g_temp, 1));
    doc["humidity"]    = serialized(String(g_hum,  0));
    char payload[64];
    serializeJson(doc, payload, sizeof(payload));
    mqtt.publish(MQTT_TOPIC_SENSORS, payload);
    Serial.printf("MQTT → %s : %s\n", MQTT_TOPIC_SENSORS, payload);
}

/* ══════════════════════════════════════════════════════════════════════
 *  setup()
 * ══════════════════════════════════════════════════════════════════════*/
void setup()
{
    Serial.begin(115200);
    stm32.begin(115200, SERIAL_8N1, 16, 17);
    dht.begin();

    /* WiFi */
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.printf("\nConnected — IP: %s\n", WiFi.localIP().toString().c_str());

    /* MQTT */
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setKeepAlive(30);
}

/* ══════════════════════════════════════════════════════════════════════
 *  loop()
 * ══════════════════════════════════════════════════════════════════════*/
void loop()
{
    mqttReconnect();
    mqtt.loop();

    /* ── Read UART ACK from STM32 (non-blocking) ─────────────────── */
    while (stm32.available()) {
        char c = stm32.read();
        if (c == '\n') {
            stm_buf.trim();
            if (stm_buf == "ACK") {
                Serial.println("STM32 ACK received");
            }
            stm_buf = "";
        } else if (c != '\r') {
            stm_buf += c;
        }
    }

    /* ── Send sensor data every 3 s ─────────────────────────────── */
    if (millis() - g_last_send >= 3000) {
        g_last_send = millis();

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t) && !isnan(h)) {
            g_temp = t;
            g_hum  = h;

            /* JSON to STM32 */
            char json[48];
            snprintf(json, sizeof(json), "{\"t\":%.1f,\"h\":%.1f}", g_temp, g_hum);
            stm32.println(json);
            Serial.printf("→ STM32: %s\n", json);

            /* MQTT to broker */
            if (mqtt.connected()) mqttPublish();

        } else {
            Serial.println("DHT11 read error");
        }
    }
}
