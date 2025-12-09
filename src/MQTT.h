/**
 * MQTT Utilities:
 * - reconnect: to handle MQTT reconnection logic in the main loop().
 */

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

namespace MQTT
{
    unsigned long last_reconnect_attempt_ms = 0;

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
} // namespace MQTT