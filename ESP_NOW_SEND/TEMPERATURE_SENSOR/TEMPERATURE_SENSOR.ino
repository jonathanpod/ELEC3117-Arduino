#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include "Adafruit_MCP9808.h"

// Initialise the Adafruit temperature sensor.
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// ----------------- WI-FI INITIALISATION -----------------
// Broadcast Address of Central Hub
uint8_t broadcastAddress[] = {0xA8, 0x48, 0xFA, 0xE6, 0x58, 0x55};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temp_c;
} struct_message;

typedef struct Threshold {
  int high_temp = 28;
  int low_temp = 10;
} Threshold;

// Create a struct_message called myData
struct_message myData;
Threshold threshold_payload;

// ----------------- GLOBAL VARIABLES -------------------
unsigned long last_time = 0;
int high_temp = 28;
int low_temp = 10;
unsigned long timer_delay = 360000; // Timer delay = 6 minutes initially
int reading_count = 0;
int flag = 0;
float temperature_array[5];

// --------------------- USER-DEFINED FUNCTIONS --------------------------

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

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&threshold_payload, incomingData, sizeof(threshold_payload));
  Serial.print("Received packet!");
  Serial.print(threshold_payload.high_temp);
  Serial.println(threshold_payload.low_temp);
}

// ---------------------- MAIN -----------------------
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

  // Register the callback functions
  // Initialise the role of ESP8266
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Check if sensor address is correct.
  if (!tempsensor.begin(0x18)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1);
  }

  // Temperature Sensor Setup
  tempsensor.setResolution(3);
}

void loop() {
  if ((millis() - last_time) > timer_delay) {    
    
    // Wake up sensor and read temperature.
    tempsensor.wake(); 
    float c = tempsensor.readTempC();

    delay(1000);
    tempsensor.shutdown_wake(1); // shutdown temperature sensor
    delay(200);

    // Set values to send
    myData.temp_c = c;

    // Update threshold temperatures.
    high_temp = threshold_payload.high_temp;
    low_temp = threshold_payload.low_temp;
    Serial.print("New Upper Temperature: ");
    Serial.println(high_temp);
    Serial.print("New Lower Temperature: ");
    Serial.println(low_temp);

    // If reading is higher or lower than threshold.
    if (c >= high_temp || c <= low_temp) {
      // Increase sampling time and alert the receiver.
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));  // Send data to RX
      timer_delay = 60000;  // Set new sampling interval to 1 minute.

      reading_count = 0;

      Serial.print("WARNING! ");
      Serial.print(c, 4);
      Serial.println("*C\t");

      flag = 1; 
      
    } else if (flag == 1 && c <= high_temp && c >= low_temp){
      
      myData.temp_c = c;
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));  // Send data to RX
      flag = 0;
      
    } else {
      timer_delay = 360000; // Else set it to 6 minutes
      
      // Append to temperature_array.
      temperature_array[reading_count] = c;    
      reading_count++;

      // If it has read 5 values, average and reset the counter.
      // Normal state means that temperature value is within the range.
      if (reading_count >= 5) {
        float avg_temp = temperature_array[0] + temperature_array[1] + temperature_array[2] + temperature_array[3] + temperature_array[4];
        avg_temp = avg_temp / 5;

        Serial.println("START");
        Serial.println(temperature_array[0]);
        Serial.println(temperature_array[1]);
        Serial.println(temperature_array[2]);
        Serial.println(temperature_array[3]);
        Serial.println(temperature_array[4]);
        Serial.println("END");

        Serial.print("Avg temp: ");
        Serial.println(avg_temp);
        
        // Set values to send
        myData.temp_c = avg_temp;
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));  // Send data to RX
      
        reading_count = 0;
      }
    }
    
    last_time = millis();  // Update the last_time variable
  }
}
