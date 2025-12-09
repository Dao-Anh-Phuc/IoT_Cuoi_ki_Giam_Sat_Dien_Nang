#include <Arduino.h>

#include "secrets/wifi.h"
#include "wifi_connect.h"
#include <WiFiClientSecure.h>

#include "ca_cert_emqx.h"
#include "secrets/mqtt.h"
#include <PubSubClient.h>
#include "MQTT.h"

#include <Ticker.h>

#include <DHT.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define DHT22PIN 13U
#define LED_RESET_PIN 5U     // ✅ SỬA: LED báo hiệu reset energy
#define RELAY_PIN 17U
#define BUTTON_PIN 23U 

namespace
{
    const char *ssid = WiFiSecrets::ssid;
    const char *password = WiFiSecrets::pass;
    const char *client_id = (String("esp32-client") + WiFi.macAddress()).c_str();

    DHT dht(DHT22PIN, DHT22);
    PZEM004Tv30 pzem(Serial2, 26, 27);
    
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    
    WiFiClientSecure tlsClient;
    PubSubClient mqttClient(tlsClient);

    Ticker dhtTicker;
    Ticker pzemTicker;
    Ticker lcdTicker;
    Ticker ledBlinkTicker;  // ✅ THÊM: Timer cho LED nhấp nháy
    
    bool relayState = false;
    
    // ✅ THÊM: LED reset indicator variables
    bool ledResetActive = false;
    int ledBlinkCount = 0;
    const int LED_BLINK_TOTAL = 6;  // Nhấp nháy 3 lần (6 transitions)
    
    // Button variables
    bool lastButtonState = HIGH;
    bool currentButtonState = HIGH;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
    // LCD display variables
    int lcdDisplayMode = 0;
    unsigned long lastLcdUpdate = 0;
    unsigned long lcdUpdateInterval = 3000;
    
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
    
    // DHT Topics
    const char *temperature_topic = "home/temperature";
    const char *humidity_topic = "home/humidity";
    
    // PZEM Topics
    const char *voltage_topic = "home/voltage";
    const char *current_topic = "home/current";
    const char *power_topic = "home/power";
    const char *energy_topic = "home/energy";
    const char *frequency_topic = "home/frequency";
    const char *pf_topic = "home/powerfactor";
    
    // PZEM Reset Topics
    const char *pzem_reset_topic = "home/pzem/reset";
    const char *pzem_status_topic = "home/pzem/status";

    // Relay Topics
    const char *relay_control_topic = "home/relay/control";
    const char *relay_status_topic = "home/relay/status";
}

// ✅ THÊM: Function LED nhấp nháy khi reset
void ledBlinkCallback()
{
    if (ledBlinkCount < LED_BLINK_TOTAL) {
        digitalWrite(LED_RESET_PIN, !digitalRead(LED_RESET_PIN));
        ledBlinkCount++;
    } else {
        ledBlinkTicker.detach();
        digitalWrite(LED_RESET_PIN, LOW);  // Tắt LED
        ledResetActive = false;
        ledBlinkCount = 0;
    }
}

// ✅ THÊM: Function bật LED khi bắt đầu reset
void startLedResetIndicator()
{
    ledResetActive = true;
    ledBlinkCount = 0;
    digitalWrite(LED_RESET_PIN, HIGH);  // Bật LED
    ledBlinkTicker.attach_ms(300, ledBlinkCallback);  // Nhấp nháy mỗi 300ms
}

void updateLCD()
{
    lcd.clear();
    
    switch (lcdDisplayMode) {
        case 0:  // Màn 1: Voltage + Current
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
            
        case 1:  // Màn 2: Power + Energy
            lcd.setCursor(0, 0);
            lcd.print("P:");
            lcd.print(displayData.power, 1);
            lcd.print("W");
            
            lcd.setCursor(0, 1);
            lcd.print("E:");
            lcd.print(displayData.energy, 3);
            lcd.print("kWh");
            break;
            
        case 2:  // Màn 3: Frequency + PF + Temperature
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
    
    if (millis() - lastLcdUpdate >= lcdUpdateInterval) {
        lcdDisplayMode = (lcdDisplayMode + 1) % 3;
        lastLcdUpdate = millis();
    }
}

void dhtReadPublish()
{
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    displayData.temperature = temperature;
    displayData.humidity = humidity;

    mqttClient.publish(temperature_topic, String(temperature).c_str(), false);
    mqttClient.publish(humidity_topic, String(humidity).c_str(), false);
}

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
        Serial.print("Voltage: "); 
        Serial.print(voltage); 
        Serial.println("V");
        mqttClient.publish(voltage_topic, String(voltage, 1).c_str(), false);
    } else {
        Serial.println("Error reading voltage");
    }

    if (!isnan(current)) {
        Serial.print("Current: "); 
        Serial.print(current); 
        Serial.println("A");
        mqttClient.publish(current_topic, String(current, 3).c_str(), false);
    } else {
        Serial.println("Error reading current");
    }

    if (!isnan(power)) {
        Serial.print("Power: "); 
        Serial.print(power); 
        Serial.println("W");
        mqttClient.publish(power_topic, String(power, 1).c_str(), false);
    } else {
        Serial.println("Error reading power");
    }

    if (!isnan(energy)) {
        Serial.print("Energy: "); 
        Serial.print(energy, 3); 
        Serial.println("kWh");
        mqttClient.publish(energy_topic, String(energy, 3).c_str(), false);
    } else {
        Serial.println("Error reading energy");
    }

    if (!isnan(frequency)) {
        Serial.print("Frequency: "); 
        Serial.print(frequency, 1); 
        Serial.println("Hz");
        mqttClient.publish(frequency_topic, String(frequency, 1).c_str(), false);
    } else {
        Serial.println("Error reading frequency");
    }

    if (!isnan(pf)) {
        Serial.print("PF: "); 
        Serial.println(pf);
        mqttClient.publish(pf_topic, String(pf, 2).c_str(), false);
    } else {
        Serial.println("Error reading power factor");
    }

    Serial.println("─────────────────");
}

void controlRelay(bool state)
{
    relayState = state;
    displayData.relayState = state;
    digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
    
    mqttClient.publish(relay_status_topic, relayState ? "ON" : "OFF", true);

    Serial.print("🔌 Relay: ");
    Serial.print(relayState ? "ON" : "OFF");
    Serial.print(" (Pin level: ");
    Serial.print(relayState ? "LOW" : "HIGH");
    Serial.println(")");
}

void toggleRelay()
{
    controlRelay(!relayState);
}

// ✅ SỬA: Reset PZEM với LED indicator
void resetPzemEnergy()
{
    Serial.println("🔄 Resetting PZEM energy...");
    
    // ✅ BẬT LED báo hiệu reset
    startLedResetIndicator();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESETTING...");
    lcd.setCursor(0, 1);
    lcd.print("PZEM ENERGY");
    delay(500);
    
    bool success = pzem.resetEnergy();
    
    if (success) {
        Serial.println("✅ PZEM energy reset successful");
        mqttClient.publish(pzem_status_topic, "RESET_SUCCESS", false);
        mqttClient.publish(energy_topic, "0.000", false);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RESET SUCCESS!");
        lcd.setCursor(0, 1);
        lcd.print("Energy: 0.000kWh");
        delay(1500);
        
        displayData.energy = 0.0f;
    } else {
        Serial.println("❌ PZEM energy reset failed");
        mqttClient.publish(pzem_status_topic, "RESET_FAILED", false);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RESET FAILED!");
        lcd.setCursor(0, 1);
        lcd.print("Check PZEM");
        delay(1500);
    }
}

void handleButton()
{
    bool reading = digitalRead(BUTTON_PIN);
    
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != currentButtonState) {
            currentButtonState = reading;
            
            if (currentButtonState == LOW) {
                Serial.println("🔘 Hardware button pressed - Resetting PZEM energy");
                resetPzemEnergy();
            }
        }
    }
    
    lastButtonState = reading;
}

void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
    // ✅ XÓA: LED brightness control (không dùng nữa)
    
    // Relay control
    if (strcmp(topic, relay_control_topic) == 0)
    {
        char command[length + 1];
        memcpy(command, payload, length);
        command[length] = '\0';
        
        Serial.print("📱 Relay command received: ");
        Serial.println(command);
        
        if (strcmp(command, "ON") == 0 || strcmp(command, "1") == 0) {
            controlRelay(true);
        }
        else if (strcmp(command, "OFF") == 0 || strcmp(command, "0") == 0) {
            controlRelay(false);
        }
        else if (strcmp(command, "TOGGLE") == 0) {
            toggleRelay();
        }
        else {
            Serial.println("❌ Invalid relay command");
        }
    }
    
    // PZEM Reset control
    else if (strcmp(topic, pzem_reset_topic) == 0)
    {
        char command[length + 1];
        memcpy(command, payload, length);
        command[length] = '\0';
        
        Serial.print("📱 PZEM reset command: ");
        Serial.println(command);
        
        if (strcmp(command, "RESET") == 0 || strcmp(command, "reset") == 0) {
            resetPzemEnergy();
        }
        else if (strcmp(command, "RESET_ENERGY") == 0) {
            resetPzemEnergy();
        }
        else {
            Serial.println("❌ Invalid PZEM reset command");
            Serial.println("💡 Valid commands: RESET, RESET_ENERGY");
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    
    Serial.println("🚀 ESP32 IoT System Starting...");
    Serial.println("⚡ PZEM using Serial2 (RX=GPIO26, TX=GPIO27)");
    Serial.println("🔧 Debug using Serial (USB)");
    
    // Khởi tạo LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    lcd.setCursor(0, 0);
    lcd.print("ESP32 IoT System");
    lcd.setCursor(0, 1);
    lcd.print("Starting...");
    delay(2000);
    
    // ✅ Setup pins
    pinMode(LED_RESET_PIN, OUTPUT);      // LED báo reset
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // ✅ Khởi tạo LED tắt
    digitalWrite(LED_RESET_PIN, LOW);
    
    // Khởi tạo relay OFF
    digitalWrite(RELAY_PIN, HIGH);
    relayState = false;
    displayData.relayState = false;
    
    Serial.println("🔌 Relay initialized: OFF (HIGH level - active LOW)");
    Serial.println("💡 LED Reset Indicator: GPIO5");
    Serial.println("🔘 Reset button: GPIO23 (Press to reset energy)");
    
    setup_wifi(ssid, password);
    tlsClient.setCACert(ca_cert);

    mqttClient.setCallback(mqttCallback);
    mqttClient.setServer(EMQX::broker, EMQX::port);
    
    // Setup timers
    dhtTicker.attach(2, dhtReadPublish);
    pzemTicker.attach(3, pzemReadPublish);
    lcdTicker.attach_ms(500, updateLCD);
    
    Serial.println("✅ System ready!");
    Serial.println("💡 Controls:");
    Serial.println("   - MQTT: home/relay/control → ON/OFF/TOGGLE");
    Serial.println("   - MQTT: home/pzem/reset → RESET");
    Serial.println("   - Hardware: Press button GPIO23");
    Serial.println("   - LCD: Auto-rotating display every 3s");
    Serial.println("   - LED: Blinks 3 times when resetting energy");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM READY");
    lcd.setCursor(0, 1);
    lcd.print("Relay: OFF");
    delay(1000);
}

void loop()
{
    // ✅ SỬA: Chỉ còn 2 topics (bỏ LED_brightness_topic)
    const char *subscribe_topics[] = {
        relay_control_topic,
        pzem_reset_topic
    };
    
    MQTT::reconnect(mqttClient, client_id, EMQX::username, EMQX::password, 
                   subscribe_topics, 2);  // ✅ Chỉ 2 topics
    
    mqttClient.loop();
    handleButton();
    
    delay(10);
}