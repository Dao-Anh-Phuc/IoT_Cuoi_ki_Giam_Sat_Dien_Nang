#pragma once

#define LED_RESET_PIN 5U            // LED indicator for PZEM reset
#define RELAY_PIN 18U               // Relay control (Active LOW)
#define BUTTON_PIN 23U              // Button for energy reset
#define SHT31_SDA 21U               // I2C SDA pin
#define SHT31_SCL 22U               // I2C SCL pin
#define PZEM_RX 26U                 // PZEM RX (ESP32 GPIO26)
#define PZEM_TX 27U                 // PZEM TX (ESP32 GPIO27)


#define SHT31_I2C_ADDR 0x44         // SHT31 temperature/humidity sensor
#define LCD_I2C_ADDR 0x27           // LCD 16x2 with I2C backpack

// Sensor reading intervals
#define DHT_READ_INTERVAL 2000      // Read SHT31 every 2 seconds
#define PZEM_READ_INTERVAL 3000     // Read PZEM every 3 seconds

// LCD update intervals
#define LCD_UPDATE_INTERVAL 500              // Refresh LCD display every 0.5s
#define LCD_DISPLAY_CHANGE_INTERVAL 3000     // Change LCD screen every 3s

// System monitoring intervals
#define SYSTEM_INFO_INTERVAL 5000    // Publish system info every 5s (rotated)
#define RELAY_STATS_INTERVAL 60000   // Publish relay stats every 60s
#define WIFI_CHECK_INTERVAL 30000    // Check WiFi connection every 30s

// MQTT intervals
#define MQTT_HEARTBEAT_INTERVAL 30000 // Send MQTT heartbeat every 30s
#define MQTT_RECONNECT_DELAY 5000     // Delay between reconnect attempts

#define MQTT_BUFFER_SIZE 1024        // MQTT packet buffer size
#define MQTT_KEEPALIVE 60            // MQTT keepalive interval (seconds)

#define BUTTON_DEBOUNCE_DELAY 50     // Button debounce time (ms)


#define LED_BLINK_TOTAL 6            // Total blinks for reset indicator
#define LED_BLINK_INTERVAL 300       // Blink interval (ms)

#define TEMP_THRESHOLD              35.0f    // °C - Ngưỡng nhiệt độ tự động tắt relay
#define TEMP_HYSTERESIS            2.0f     // °C - Độ trễ bật lại (bật khi T < threshold - hysteresis)
#define TEMP_CHECK_INTERVAL        2000     // ms - Kiểm tra nhiệt độ mỗi 2s