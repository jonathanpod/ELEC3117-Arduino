/*
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include "Adafruit_MCP9808.h"

// Initialise the Adafruit temperature sensor.
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Define the constants
#define HIGH_TEMP 28
#define LOW_TEMP 20

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xA8, 0x48, 0xFA, 0xE6, 0x58, 0x55};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temp_c;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long last_time = 0;  
unsigned long timer_delay = 360000; // Timer delay = 6 minutes initially
unsigned long timer_delay_30_minutes = 1800000;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(9600);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  // Check if sensor address is correct.
  if (!tempsensor.begin(0x18)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1);
  }

  // Temperature Sensor Setup
  tempsensor.setResolution(3);

  // Initialise temperature array
  uint8_t temperature_array[5];
  int reading_count = 0;
}


void loop() {
  if ((millis() - lastTime) > timer_delay) {    
    
    // Wake up sensor and read temperature.
    tempsensor.wake(); 
    float c = tempsensor.readTempC();

    delay(2000);
    tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere, stops temperature sampling
    delay(200);

    // Append to temperature_array.
    temperature_array[reading_count];
    
    // Set values to send
    myData.temp_c = c;
    Serial.print(c, 4); 
    Serial.println("*C\t");
    
    reading_count++;  // Increment the reading counter.

    // If reading is higher or lower than threshold.
    if (c >= HIGH_TEMP || c <= LOW_TEMP) {
      // Increase sampling time and alert the receiver.
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));  // Send data to RX
      timer_delay = 10000;  // Set new sampling interval to 10 seconds.
    } else {
      timer_delay = 360000; // Else set it to 6 minutes.        
    }




    
    // Send message via ESP-NOW
    //esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    // If it has read 5 values, average and reset the array.
    if (reading_count >= 5) {
      float avg_temp = temperature_array[0] + temperature_array[1] + temperature_array[2] + temperature_array[3] + temperature_array[4];
      avg_temp = (1/5) * avg_temp;

      // Set values to send
      myData.temp_c = c;
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));  // Send data to RX
      
      reading_count = 0;
    }

    last_time = millis();  // Update the last_time variable
  }
  
}
