#pragma once

// ════════════════════════════════════════════════════════════════
// MQTT TOPIC DEFINITIONS
// ════════════════════════════════════════════════════════════════

namespace MQTTTopics
{
    // ════════════════════════════════════════════════════════════
    // SYSTEM TOPICS
    // ════════════════════════════════════════════════════════════
    constexpr const char* MQTT_STATUS = "home/system/mqtt";
    constexpr const char* MQTT_LWT = "0";           // Last Will Testament (Offline)
    constexpr const char* MQTT_ONLINE = "1";        // Online status
    
    constexpr const char* SYSTEM_RSSI = "home/system/rssi";
    constexpr const char* SYSTEM_IP = "home/system/ip";
    constexpr const char* SYSTEM_UPTIME = "home/system/uptime";
    constexpr const char* SYSTEM_HEAP = "home/system/heap";
    
    // ════════════════════════════════════════════════════════════
    // SENSOR TOPICS
    // ════════════════════════════════════════════════════════════
    constexpr const char* TEMPERATURE = "home/temperature";
    constexpr const char* HUMIDITY = "home/humidity";
    
    // ════════════════════════════════════════════════════════════
    // PZEM POWER MONITOR TOPICS
    // ════════════════════════════════════════════════════════════
    constexpr const char* VOLTAGE = "home/voltage";
    constexpr const char* CURRENT = "home/current";
    constexpr const char* POWER = "home/power";
    constexpr const char* ENERGY = "home/energy";
    constexpr const char* FREQUENCY = "home/frequency";
    constexpr const char* POWER_FACTOR = "home/powerfactor";
    
    // ════════════════════════════════════════════════════════════
    // RELAY CONTROL TOPICS
    // ════════════════════════════════════════════════════════════
    constexpr const char* RELAY_CONTROL = "home/relay/control";
    constexpr const char* RELAY_STATUS = "home/relay/status";
    constexpr const char* RELAY_EVENT = "home/relay/event";
    constexpr const char* RELAY_STATS = "home/relay/stats";
    
    // ════════════════════════════════════════════════════════════
    // PZEM RESET TOPICS
    // ════════════════════════════════════════════════════════════
    constexpr const char* PZEM_RESET = "home/pzem/reset";
    constexpr const char* PZEM_STATUS = "home/pzem/status";
}