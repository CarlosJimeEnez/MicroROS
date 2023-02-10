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
#define LED_BUILTIN 2

// UROS Dependencias: 
rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;
std_msgs__msg__Int32 msg2;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

// Subscription: 
rcl_subscription_t subscriber; 

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


void subscription_callback(const void * msgin)
{  
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;  
  //digitalWrite(LED_BUILTIN, (msg->data > 0) ? LOW : HIGH);  
}


void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
  }
}

void setup() {
  // ------------ Serial port Configuration: ---------------- //
  Serial.begin(115200);
  // ------------ I2c Configuration: ----------------- // 
  Wire.begin(); 

  pinMode(LED_BUILTIN, OUTPUT); 

  // ----------- ROS Configuration: ------------------- // 
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


  // create subscriber
  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node, 
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "defuzzified_value"
  ));


  // create timer,
  const unsigned int timer_timeout = 10;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // create executor

  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));   
  
  msg.data = 0;
  // --------- Setup Encoder -------- //
    encoder1.setupCero(); 
}

void loop() {
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));

    encoder1.mapVal(); 
    msg.data = encoder1.SumDegTotal(360); 
}
