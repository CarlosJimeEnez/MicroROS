#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

//Arduino Framework: 
#if !defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
#error This example is only avaliable for Arduino framework with serial transport.
#endif


#include <Wire.h>
#include <AS5600.h>
#include <Encoder.h>

#include <WiFi.h>
#include <PubSubClient.h>

#define tcaAddress 0x70 

//ROS publisher: 
rcl_publisher_t publisher; 
std_msgs__msg__Int32 msg; 


rclc_executor_t executor; 
rclc_support_t support; 
rcl_allocator_t allocator; 
rcl_node_t node; 
rcl_timer_t timer; 


#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


// Error handle loop
void error_loop() {
  while(1) {
    delay(100);
    Serial.println("Error"); 
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
    msg.data++;
  }
}


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
bool start = false; 

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

    // ------- Conversion de char a int  --------- //
    String topic_string = topic;
    String concatenacion = ""; 
    String auxiliar = "0"; 
    for (size_t i = 0; i < length; i++)
    {
        auxiliar = String((char) payload[i]); 
        concatenacion = concatenacion + auxiliar; 
    }
    int payload_int = concatenacion.toInt(); 


    // ---------- BEGIN -------- //
    if(topic_string == "node_red" && payload_int == 1){
        start = true; 
    }
    else if (topic_string = "node_red" && payload_int == 0)
    {
        start = false; 
    }
    

    Serial.println();
    Serial.println("-----------------------");
}


void setup() {
    // Serial: 
    Serial.begin(115200);
    set_microros_serial_transports(Serial); 
    delay(2000); 

    rcl_allocator_t allocator = rcl_get_default_allocator();

    //create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    // create node
    RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

    // create publisher
    RCCHECK(rclc_publisher_init_default(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "esp32"));

    // create timer,
    const unsigned int timer_timeout = 1000;
    RCCHECK(rclc_timer_init_default(
        &timer,
        &support,
        RCL_MS_TO_NS(timer_timeout),
        timer_callback));

    // create executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer));

    msg.data = 0;


    // I2C: 
    Wire.begin(); 

    // // ---------------- connecting to a WiFi network -------------------------
    // WiFi.begin(ssid, password);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.println("Connecting to WiFi..");
    // }
    // Serial.println("Connected to the WiFi network");
    // //connecting to a mqtt broker
    // client.setServer(mqtt_broker, mqtt_port);
    // client.setCallback(callback);
    // while (!client.connected()) {
    //     String client_id = "esp32-client-";
    //     client_id += String(WiFi.macAddress());
    //     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    //     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    //         Serial.println("Public emqx mqtt broker connected");
    //     } else {
    //         Serial.print("failed with state ");
    //         Serial.print(client.state());
    //         delay(2000);
    //     }
    // }

    // // publish and subscribe
    // client.publish(topic, "Hi EMQX I'm ESP32 ^^");
    // client.subscribe("node_red");

    //-------------- Setup Encoder ------------------- 
    encoder1.setupCero(); 
}


void loop() {

    encoder1.mapVal(); 
    //  arg:360 -> Si el motor de solo una vuelta: 
    // Serial.println(encoder1.SumDegTotal(360)); 
    float encoder_val  = encoder1.SumDegTotal(360);
    
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
    // client.loop();
}


/////////////////////////////////////////////////////////////////