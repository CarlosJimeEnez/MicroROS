#include <Arduino.h>

#include <Wire.h>
#include <AS5600.h>
#include <Encoder.h>

#include <WiFi.h>
#include <PubSubClient.h>

#define tcaAddress 0x70 

// WiFi
//const char *ssid = "RS_NETWORK_1_2.4G"; // Enter your WiFi name
const char *ssid = "MEGACABLE-2.4G-D0A3"; // Enter your WiFi name
//const char *password = "rsautomation2017";  // Enter WiFi password
const char *password = "7CGDTn5fc6";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "python/mqtt";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

float previus_time = 0;  

WiFiClient espClient;
PubSubClient client(espClient);
Encoder encoder1(0,0,0); 

//-------------> Callback to subscribing Topics <------------- //
void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");
}


void setup() {
    // Serial: 
    Serial.begin(115200);
    // I2C: 
    Wire.begin(); 

    // ---------------- connecting to a WiFi network -------------------------
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    // publish and subscribe
    client.publish(topic, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic);

    //-------------- Setup Encoder ------------------- 
    encoder1.setupCero(); 
}


void loop() {

    encoder1.mapVal(); 
    //  arg:360 -> Si el motor de solo una vuelta: 
    Serial.println(encoder1.SumDegTotal(360)); 


//   int adc =  analogRead(GPIO_NUM_35);
//   unsigned long current_miles = millis(); 
//   if (current_miles - previus_time > 100){
//     client.publish(topic, String(adc).c_str());
//     previus_time = current_miles;  
//   } 

    client.loop();
}
