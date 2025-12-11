#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

namespace MQTT
{
    unsigned long last_reconnect_attempt_ms = 0;
    unsigned long last_heartbeat_ms = 0;
    bool was_mqtt_connected = false;
    
    const unsigned long HEARTBEAT_INTERVAL = 30000;

    void publishStatus(PubSubClient &mqttClient, const char *status_topic, const char *status)
    {
        if (mqttClient.connected())
        {
            bool success = mqttClient.publish(status_topic, status, true);  // retained = true
            if (success) {
                Serial.printf("MQTT Status published: %s → %s\n", status_topic, status);
            } else {
                Serial.printf("MQTT Status publish FAILED: %s\n", status_topic);
            }
        } else {
            Serial.println("Cannot publish status - MQTT not connected!");
        }
    }

    void reconnectWithLWT(PubSubClient &mqttClient, 
                         const char *client_id,
                         const char *username, 
                         const char *password,
                         const char *subscribe_topics[], 
                         int subscribe_count,
                         const char *lwt_topic,
                         const char *lwt_message,
                         const char *online_message)
    {
        bool is_connected = mqttClient.connected();
        
        if (was_mqtt_connected && !is_connected)
        {
            Serial.println("MQTT Disconnected!");
            was_mqtt_connected = false;
        }
        
        if (!is_connected && WiFi.status() == WL_CONNECTED)
        {
            if (millis() - last_reconnect_attempt_ms > 5000)
            {
                last_reconnect_attempt_ms = millis();
                Serial.println("Attempting MQTT connection with LWT...");
                Serial.printf("   Client ID: %s\n", client_id);
                Serial.printf("   LWT Topic: %s\n", lwt_topic);
                Serial.printf("   LWT Message: %s\n", lwt_message);
                
                if (mqttClient.connect(
                    client_id,
                    username,
                    password,
                    lwt_topic,
                    1,
                    true,
                    lwt_message
                ))
                {
                    Serial.print("MQTT Connected: ");
                    Serial.println(client_id);
                    
                    // Subscribe topics
                    for (int i = 0; i < subscribe_count; i++)
                    {
                        bool sub_success = mqttClient.subscribe(subscribe_topics[i]);
                        if (sub_success) {
                            Serial.printf("   Subscribed: %s\n", subscribe_topics[i]);
                        } else {
                            Serial.printf("   Subscribe FAILED: %s\n", subscribe_topics[i]);
                        }
                    }
                    
                    // Publish "Online" status
                    Serial.println("Publishing Online status...");
                    publishStatus(mqttClient, lwt_topic, online_message);
                    
                    was_mqtt_connected = true;
                }
                else
                {
                    Serial.printf("MQTT connection failed, rc=%d\n", mqttClient.state());
                    Serial.println("   Error codes:");
                    Serial.println("   -4: Connection timeout");
                    Serial.println("   -3: Connection lost");
                    Serial.println("   -2: Connect failed");
                    Serial.println("   -1: Disconnected");
                    Serial.println("    0: Connected");
                    Serial.println("    1: Bad protocol");
                    Serial.println("    2: Bad client ID");
                    Serial.println("    3: Server unavailable");
                    Serial.println("    4: Bad credentials");
                    Serial.println("    5: Not authorized");
                }
            }
        }
    }
    
    void heartbeat(PubSubClient &mqttClient, 
                   const char *status_topic, 
                   const char *online_message)
    {
        if (mqttClient.connected())
        {
            if (millis() - last_heartbeat_ms > HEARTBEAT_INTERVAL)
            {
                last_heartbeat_ms = millis();
                Serial.println("Sending heartbeat...");
                publishStatus(mqttClient, status_topic, online_message);
            }
        }
    }

    void reconnect(PubSubClient &mqttClient, const char *client_id,
                   const char *username, const char *password,
                   const char *subscribe_topics[], int subscribe_count)
    {
        if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED)
        {
            if (millis() - last_reconnect_attempt_ms > 5000)
            {
                last_reconnect_attempt_ms = millis();
                Serial.println("Attempting MQTT connection...");
                if (mqttClient.connect(client_id, username, password))
                {
                    Serial.print(client_id);
                    Serial.println(" connected");
                    for (int i = 0; i < subscribe_count; i++)
                    {
                        mqttClient.subscribe(subscribe_topics[i]);
                    }
                }
            }
        }
    }

    void reconnect(PubSubClient &mqttClient, const char *client_id,
                   const char *username, const char *password,
                   const char *subscribe_topic)
    {
        const char *subscribe_topics[] = {subscribe_topic};
        reconnect(mqttClient, client_id, username, password, subscribe_topics, 1);
    }
}