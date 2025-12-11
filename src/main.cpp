#include <Arduino.h>

// Config & Secrets
#include "config.h"
#include "topics.h"
#include "secrets/wifi.h"
#include "secrets/mqtt.h"

// Network
#include "wifi_connect.h"
#include <WiFiClientSecure.h>
#include "ca_cert_emqx.h"
#include <PubSubClient.h>
#include "MQTT.h"

// Libraries
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>

namespace
{
    // WiFi & MQTT
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
    String client_id_string;
    const char *client_id;

    // Hardware Objects
    Adafruit_SHT31 sht31 = Adafruit_SHT31();
    PZEM004Tv30 pzem(Serial2, PZEM_RX, PZEM_TX);
    LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);
    
    WiFiClientSecure tlsClient;
    PubSubClient mqttClient(tlsClient);

    // Tickers
    Ticker dhtTicker;
    Ticker pzemTicker;
    Ticker lcdTicker;
    Ticker ledBlinkTicker;
    Ticker systemInfoTicker;
    
    // State Variables
    bool relayState = false;
    bool ledResetActive = false;
    int ledBlinkCount = 0;
    
    bool lastButtonState = HIGH;
    bool currentButtonState = HIGH;
    unsigned long lastDebounceTime = 0;
    
    int lcdDisplayMode = 0;
    unsigned long lastLcdUpdate = 0;
    
    unsigned long relay_on_time = 0;
    unsigned long relay_off_time = 0;
    unsigned long last_state_change = 0;
    bool last_relay_state = false;
    
    int currentSystemInfoIndex = 0;
    
    // Display Data Struct
    struct DisplayData {
        float voltage = 0.0f;
        float current = 0.0f;
        float power = 0.0f;
        float energy = 0.0f;
        float frequency = 0.0f;
        float powerFactor = 0.0f;
        float temperature = 0.0f;
        float humidity = 0.0f;
        bool relayState = false;
        bool dataValid = false;
    } displayData;
}

// Function Prototypes
void ledBlinkCallback();
void startLedResetIndicator();
void publishSystemInfoByIndex();
void updateRelayStats();
void publishRelayStats();
void updateLCD();
void dhtReadPublish();
void pzemReadPublish();
void controlRelay(bool state);
void toggleRelay();
void resetPzemEnergy();
void handleButton();
void mqttCallback(char *topic, uint8_t *payload, unsigned int length);
void scanI2C();

// LED Blink Callback (cho PZEM reset indicator)
void ledBlinkCallback()
{
    if (ledBlinkCount < LED_BLINK_TOTAL) {
        digitalWrite(LED_RESET_PIN, !digitalRead(LED_RESET_PIN));
        ledBlinkCount++;
    } else {
        ledBlinkTicker.detach();
        digitalWrite(LED_RESET_PIN, LOW);
        ledResetActive = false;
        ledBlinkCount = 0;
    }
}

void startLedResetIndicator()
{
    ledResetActive = true;
    ledBlinkCount = 0;
    digitalWrite(LED_RESET_PIN, HIGH);
    ledBlinkTicker.attach_ms(LED_BLINK_INTERVAL, ledBlinkCallback);
}

// Scan I2C Devices (Debug)
void scanI2C()
{
    Serial.println("🔍 Scanning I2C bus...");
    byte count = 0;
    
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.print("   Found device at 0x");
            if (i < 16) Serial.print("0");
            Serial.print(i, HEX);
            
            // Identify known devices
            if (i == LCD_I2C_ADDR) Serial.print(" (LCD)");
            if (i == SHT31_I2C_ADDR) Serial.print(" (SHT31)");
            if (i == 0x3F) Serial.print(" (LCD alt)");
            if (i == 0x45) Serial.print(" (SHT31 alt)");
            
            Serial.println();
            count++;
        }
    }
    
    if (count == 0) {
        Serial.println("   No I2C devices found!");
        Serial.printf("   Check wiring: SDA=GPIO%d, SCL=GPIO%d\n", SHT31_SDA, SHT31_SCL);
    } else {
        Serial.printf("   Total: %d device(s) found\n", count);
    }
}

// Publish System Info (Rotated by Ticker)
void publishSystemInfoByIndex()
{
    if (!mqttClient.connected()) {
        return;
    }
    
    switch (currentSystemInfoIndex) {
        case 0: {
            int rssi = WiFi.RSSI();
            bool ok = mqttClient.publish(MQTTTopics::SYSTEM_RSSI, String(rssi).c_str(), false);
            Serial.printf("%s RSSI: %d dBm\n", ok ? "✅" : "❌", rssi);
            break;
        }
        case 1: {
            String ip = WiFi.localIP().toString();
            bool ok = mqttClient.publish(MQTTTopics::SYSTEM_IP, ip.c_str(), true);
            Serial.printf("%s IP: %s\n", ok ? "✅" : "❌", ip.c_str());
            break;
        }
        case 2: {
            unsigned long uptime = millis() / 1000;
            bool ok = mqttClient.publish(MQTTTopics::SYSTEM_UPTIME, String(uptime).c_str(), false);
            Serial.printf("%s Uptime: %lu seconds\n", ok ? "✅" : "❌", uptime);
            break;
        }
        case 3: {
            float heap = ESP.getFreeHeap() / 1024.0;
            bool ok = mqttClient.publish(MQTTTopics::SYSTEM_HEAP, String(heap, 1).c_str(), false);
            Serial.printf("%s Heap: %.1f KB\n", ok ? "✅" : "❌", heap);
            break;
        }
    }
    
    currentSystemInfoIndex = (currentSystemInfoIndex + 1) % 4;
}

// Update Relay Statistics
void updateRelayStats()
{
    unsigned long current_time = millis();
    unsigned long duration = current_time - last_state_change;
    
    if (last_relay_state) {
        relay_on_time += duration;
    } else {
        relay_off_time += duration;
    }
    
    last_state_change = current_time;
}

// Publish Relay Statistics
void publishRelayStats()
{
    if (mqttClient.connected())
    {
        String stats = "ON:" + String(relay_on_time / 1000) + 
                      ",OFF:" + String(relay_off_time / 1000);
        bool success = mqttClient.publish(MQTTTopics::RELAY_STATS, stats.c_str(), false);
        Serial.printf("%s Relay Stats: %s\n", 
                     success ? "✅" : "❌", stats.c_str());
    }
}

// Update LCD Display (Rotates through 3 screens)
void updateLCD()
{
    lcd.clear();
    
    switch (lcdDisplayMode) {
        case 0: // Voltage & Current
            lcd.setCursor(0, 0);
            lcd.print("V:");
            lcd.print(displayData.voltage, 1);
            lcd.print("V");
            
            lcd.setCursor(10, 0);
            lcd.print("R:");
            lcd.print(displayData.relayState ? "ON" : "OF");
            
            lcd.setCursor(0, 1);
            lcd.print("I:");
            lcd.print(displayData.current, 3);
            lcd.print("A");
            break;
            
        case 1: // Power & Energy
            lcd.setCursor(0, 0);
            lcd.print("P:");
            lcd.print(displayData.power, 1);
            lcd.print("W");
            
            lcd.setCursor(0, 1);
            lcd.print("E:");
            lcd.print(displayData.energy, 3);
            lcd.print("kWh");
            break;
            
        case 2: // Frequency, PF, Temp, Humidity
            lcd.setCursor(0, 0);
            lcd.print("F:");
            lcd.print(displayData.frequency, 1);
            lcd.print("Hz");
            
            lcd.setCursor(9, 0);
            lcd.print("PF:");
            lcd.print(displayData.powerFactor, 2);
            
            lcd.setCursor(0, 1);
            lcd.print("T:");
            lcd.print(displayData.temperature, 1);
            lcd.print("C");
            
            lcd.setCursor(9, 1);
            lcd.print("H:");
            lcd.print(displayData.humidity, 0);
            lcd.print("%");
            break;
    }
    
    if (millis() - lastLcdUpdate >= LCD_DISPLAY_CHANGE_INTERVAL) {
        lcdDisplayMode = (lcdDisplayMode + 1) % 3;
        lastLcdUpdate = millis();
    }
}

// Read & Publish SHT31 Data
void dhtReadPublish()
{
    float temperature = sht31.readTemperature();
    float humidity = sht31.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Failed to read from SHT31 sensor!");
        return;
    }

    Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);

    displayData.temperature = temperature;
    displayData.humidity = humidity;

    mqttClient.publish(MQTTTopics::TEMPERATURE, String(temperature, 1).c_str(), false);
    mqttClient.publish(MQTTTopics::HUMIDITY, String(humidity, 1).c_str(), false);
}

// Read & Publish PZEM Data
void pzemReadPublish()
{
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    displayData.voltage = voltage;
    displayData.current = current;
    displayData.power = power;
    displayData.energy = energy;
    displayData.frequency = frequency;
    displayData.powerFactor = pf;
    displayData.dataValid = !isnan(voltage) && !isnan(current);

    if (!isnan(voltage)) {
        Serial.printf("Voltage: %.1fV\n", voltage);
        mqttClient.publish(MQTTTopics::VOLTAGE, String(voltage, 1).c_str(), false);
    }

    if (!isnan(current)) {
        Serial.printf("Current: %.3fA\n", current);
        mqttClient.publish(MQTTTopics::CURRENT, String(current, 3).c_str(), false);
    }

    if (!isnan(power)) {
        Serial.printf("Power: %.1fW\n", power);
        mqttClient.publish(MQTTTopics::POWER, String(power, 1).c_str(), false);
    }

    if (!isnan(energy)) {
        Serial.printf("Energy: %.3fkWh\n", energy);
        mqttClient.publish(MQTTTopics::ENERGY, String(energy, 3).c_str(), false);
    }

    if (!isnan(frequency)) {
        Serial.printf("Frequency: %.1fHz\n", frequency);
        mqttClient.publish(MQTTTopics::FREQUENCY, String(frequency, 1).c_str(), false);
    }

    if (!isnan(pf)) {
        Serial.printf("PF: %.2f\n", pf);
        mqttClient.publish(MQTTTopics::POWER_FACTOR, String(pf, 2).c_str(), false);
    }

    Serial.println("─────────────────");
}

// Control Relay
void controlRelay(bool state)
{
    updateRelayStats();
    
    relayState = state;
    displayData.relayState = state;
    digitalWrite(RELAY_PIN, relayState ? LOW : HIGH); // Active LOW
    
    mqttClient.publish(MQTTTopics::RELAY_STATUS, relayState ? "ON" : "OFF", true);
    
    String event = String(relayState ? "ON" : "OFF");
    mqttClient.publish(MQTTTopics::RELAY_EVENT, event.c_str(), false);
    
    last_relay_state = relayState;

    Serial.printf("Relay: %s\n", relayState ? "ON" : "OFF");
}

// Toggle Relay
void toggleRelay()
{
    controlRelay(!relayState);
}

// Reset PZEM Energy
void resetPzemEnergy()
{
    Serial.println("Resetting PZEM energy...");
    
    startLedResetIndicator();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESETTING...");
    lcd.setCursor(0, 1);
    lcd.print("PZEM ENERGY");
    delay(500);
    
    bool success = pzem.resetEnergy();
    
    if (success) {
        Serial.println("PZEM energy reset successful");
        mqttClient.publish(MQTTTopics::PZEM_STATUS, "RESET_SUCCESS", false);
        mqttClient.publish(MQTTTopics::ENERGY, "0.000", false);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RESET SUCCESS!");
        lcd.setCursor(0, 1);
        lcd.print("Energy: 0.000kWh");
        delay(1500);
        
        displayData.energy = 0.0f;
    } else {
        Serial.println("PZEM energy reset failed");
        mqttClient.publish(MQTTTopics::PZEM_STATUS, "RESET_FAILED", false);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RESET FAILED!");
        lcd.setCursor(0, 1);
        lcd.print("Check PZEM");
        delay(1500);
    }
}

// Handle Button Press (with debounce)
void handleButton()
{
    bool reading = digitalRead(BUTTON_PIN);
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
        if (reading != currentButtonState) {
            currentButtonState = reading;
            
            if (currentButtonState == LOW) {
                Serial.println("Button pressed - Resetting energy");
                resetPzemEnergy();
            }
        }
    }
    
    lastButtonState = reading;
}

// MQTT Callback
void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
    char command[length + 1];
    memcpy(command, payload, length);
    command[length] = '\0';
    
    Serial.printf("MQTT Message: %s → %s\n", topic, command);
    
    if (strcmp(topic, MQTTTopics::RELAY_CONTROL) == 0)
    {
        if (strcmp(command, "ON") == 0 || strcmp(command, "1") == 0) {
            controlRelay(true);
        }
        else if (strcmp(command, "OFF") == 0 || strcmp(command, "0") == 0) {
            controlRelay(false);
        }
        else if (strcmp(command, "TOGGLE") == 0) {
            toggleRelay();
        }
    }
    else if (strcmp(topic, MQTTTopics::PZEM_RESET) == 0)
    {
        if (strcmp(command, "RESET") == 0 || strcmp(command, "reset") == 0 || 
            strcmp(command, "RESET_ENERGY") == 0) {
            resetPzemEnergy();
        }
    }
}

// SETUP
void setup()
{
    Serial.begin(115200);
    delay(10);
    
    Serial.println("\n\nESP32 IoT System Starting...");
    Serial.println("════════════════════════════════════════");
    Serial.printf("Core: %d, CPU: %dMHz\n", xPortGetCoreID(), ESP.getCpuFreqMHz());
    Serial.printf("Flash: %dMB, Free Heap: %.1fKB\n", 
                  ESP.getFlashChipSize() / 1024 / 1024, 
                  ESP.getFreeHeap() / 1024.0);
    Serial.printf("SDK: %s\n", ESP.getSdkVersion());
    Serial.println("════════════════════════════════════════");
    
    // Hardware Init
    Wire.begin(SHT31_SDA, SHT31_SCL);
    Serial.printf("I2C initialized (SDA=GPIO%d, SCL=GPIO%d)\n", SHT31_SDA, SHT31_SCL);
    
    // Scan I2C Bus
    scanI2C();
    
    // SHT31 Init
    if (!sht31.begin(SHT31_I2C_ADDR)) {
        Serial.printf("SHT31 not found at 0x%02X\n", SHT31_I2C_ADDR);
    } else {
        Serial.printf("SHT31 sensor found at 0x%02X\n", SHT31_I2C_ADDR);
    }
    
    // LCD Init
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP32 IoT System");
    lcd.setCursor(0, 1);
    lcd.print("Starting...");
    Serial.printf("LCD initialized at 0x%02X\n", LCD_I2C_ADDR);
    delay(2000);
    
    // GPIO Init
    pinMode(LED_RESET_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    digitalWrite(LED_RESET_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH); // Active LOW - OFF
    relayState = false;
    displayData.relayState = false;
    last_relay_state = false;
    last_state_change = millis();
    
    Serial.println("Relay: OFF (active LOW)");
    Serial.printf("LED Reset: GPIO%d\n", LED_RESET_PIN);
    Serial.printf("Button: GPIO%d\n", BUTTON_PIN);
    Serial.printf("PZEM: Serial2 (RX=GPIO%d, TX=GPIO%d)\n", PZEM_RX, PZEM_TX);
    
    // WiFi Setup
    Serial.println("════════════════════════════════════════");
    setup_wifi(ssid, password);
    
    // MQTT Setup
    client_id_string = "esp32-" + WiFi.macAddress();
    client_id_string.replace(":", "");
    client_id = client_id_string.c_str();
    
    Serial.println("════════════════════════════════════════");
    Serial.printf(" MQTT Client ID: %s\n", client_id);
    
    tlsClient.setCACert(ca_cert);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setServer(EMQX::broker, EMQX::port);
    mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
    mqttClient.setKeepAlive(MQTT_KEEPALIVE);
    
    Serial.printf("MQTT Buffer: %d bytes\n", MQTT_BUFFER_SIZE);
    Serial.printf("MQTT Keepalive: %ds\n", MQTT_KEEPALIVE);
    
    // Start Tickers
    dhtTicker.attach_ms(DHT_READ_INTERVAL, dhtReadPublish);
    pzemTicker.attach_ms(PZEM_READ_INTERVAL, pzemReadPublish);
    lcdTicker.attach_ms(LCD_UPDATE_INTERVAL, updateLCD);
    systemInfoTicker.attach_ms(SYSTEM_INFO_INTERVAL, publishSystemInfoByIndex);
    
    Serial.println("════════════════════════════════════════");
    Serial.println("Ticker Intervals:");
    Serial.printf("   SHT31: %dms\n", DHT_READ_INTERVAL);
    Serial.printf("   PZEM: %dms\n", PZEM_READ_INTERVAL);
    Serial.printf("   LCD: %dms\n", LCD_UPDATE_INTERVAL);
    Serial.printf("   System Info: %dms\n", SYSTEM_INFO_INTERVAL);
    Serial.printf("   Relay Stats: %dms\n", RELAY_STATS_INTERVAL);
    
    Serial.println("════════════════════════════════════════");
    Serial.println("System ready!");
    Serial.println("MQTT Topics:");
    Serial.println("   System: home/system/* (mqtt, rssi, ip, uptime, heap)");
    Serial.println("   Relay:  home/relay/* (control, status, event, stats)");
    Serial.println("   Sensors: home/* (temperature, humidity, voltage, etc.)");
    Serial.println("════════════════════════════════════════\n");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM READY");
    lcd.setCursor(0, 1);
    lcd.print("Connecting MQTT");
    delay(1000);
}

// MAIN LOOP
void loop()
{
    const char *subscribe_topics[] = {
        MQTTTopics::RELAY_CONTROL,
        MQTTTopics::PZEM_RESET
    };
    
    MQTT::reconnectWithLWT(
        mqttClient, 
        client_id, 
        EMQX::username, 
        EMQX::password,
        subscribe_topics, 
        2,
        MQTTTopics::MQTT_STATUS,
        MQTTTopics::MQTT_LWT,
        MQTTTopics::MQTT_ONLINE
    );
    
    mqttClient.loop();
    handleButton();
    
    MQTT::heartbeat(mqttClient, MQTTTopics::MQTT_STATUS, MQTTTopics::MQTT_ONLINE);
    checkWiFiConnection();
    
    // Relay Stats (every 60s)
    static unsigned long lastStatsPublish = 0;
    if (mqttClient.connected() && (millis() - lastStatsPublish > RELAY_STATS_INTERVAL)) {
        lastStatsPublish = millis();
        publishRelayStats();
    }
    
    delay(10);
}