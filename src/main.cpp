#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>


#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float64.h> 
#if !defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
#error This example is only avaliable for Arduino framework with serial transport.
#endif


// Encoder i2c Dependencies: 
#include <Wire.h>
#define tcaAddress 0x70 
#include <Encoder.h>

// MQTT Dependencies: 
#include <WiFi.h>
#include <PubSubClient.h>


//-------- Wifi variables ------ //
const char *ssid = "RS_NETWORK_1_2.4G"; 
const char *password = "rsautomation2017"; 
//const char *ssid = "BUAP_Estudiantes"; 
//const char *password = "f85ac21de4"; 
//const char *ssid = "MEGACABLE-979F"; 
//const char *password = "8eAYgaeY"; 


// ------------------- mqtt Broker --------------: 
const char *mqtt_broker = "broker.emqx.io";
// TOPICS: 
const char *topic = "esp32/test/rsautomation";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


// UROS Dependencias: 
rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;


// ENCODER Configuration: 
Encoder encoder1(0,0,0); 

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

// Error handle loop
void error_loop() {
  while(1) {
    delay(100);
  }
}


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


void subscription_callback(const void * msgin)
{
  client.publish(topic, "callback"); 
}


void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
  }
}


/// --------------------------  SETUP ---------------------------------- ///
void setup() {

  // ---------------- Serial port Configuration: ---------------- //
  Serial.begin(115200);

  // ----------------- I2c Configuration: ----------------- // 
  Wire.begin(); 

  // connecting to a WiFi network
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
  

  // --------------------- ROS Configuration: ------------------- // 
  set_microros_serial_transports(Serial);
  delay(2000);

  allocator = rcl_get_default_allocator();
  
  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "micro_ros_platformio_node_publisher"));

  // // create publisher
  // RCCHECK(rclc_publisher_init_default(
  //   &publisher,
  //   &node,
  //   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
  //   "debug_function"));


  // //create subscriber
  // RCCHECK(rclc_subscription_init_default(
  //   &subscriber,
  //   &node, 
  //   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
  //   "defuzzified_value"
  // ));


  // create timer,
  const unsigned int timer_timeout = 10;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));
  
  msg.data = 0;

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  //RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg2, &subscription_callback, ON_NEW_DATA));   
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  // --------- Setup Encoder -------- //
    encoder1.setupCero(); 
}

void loop() {
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
  
  encoder1.mapVal(); 
  msg.data = encoder1.SumDegTotal(360); 

  client.publish(topic, "hello");
}
