#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ----------------- WI-FI INITIALISATION -----------------
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    float temp_c;
} struct_message;

// ----------------- OLED INITIALISATION ------------------
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//
// Create a struct_message called myData
struct_message myData;

// ----------------- CONSTANTS -------------------
#define HIGH_TEMP 28
#define LOW_TEMP 20

// --------------------- USER-DEFINED FUNCTIONS --------------------------

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Temperature: ");
  Serial.print(myData.temp_c); 
  Serial.print("*C");
  Serial.println();
}

// Display the temperature in the OLED display.
void display_temperature(float temp_c){
  
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print("EnviroHub");
  display.setCursor(0,10); 
  display.print("Temperature:    "); 
  display.print(temp_c);
  display.print(" C");

}

// ----------------- GLOBAL VARIABLES -------------------
int button_flag = 0;

// ---------------------- MAIN -----------------------
void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Begin Wire library for I2C
  Wire.begin();
  
  // Initialize OLED with I2C address 0x3D
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  
  // Initialise pins
  pinMode(15, OUTPUT);  // Buzzer Output
  pinMode(13, INPUT);   // Button Input
  pinMode(16, OUTPUT);  // LED Output
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialise ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  // Initialise temp_c to room temperature.
  myData.temp_c = 24;
}

void loop() {
  //Serial.print("button state "); Serial.println(button_flag);

  // ------------ Buzzer Alert ------------
  if (button_flag == 0 && (myData.temp_c >= HIGH_TEMP || myData.temp_c <= LOW_TEMP)){
    digitalWrite(15, HIGH);
    delay(2);
    digitalWrite(15, LOW);
    delay(2);
  } 
  if(digitalRead(13)== LOW) {
    button_flag = 1;
    delay(10);    // This delay is used for debouncing.
  }
  if (!(myData.temp_c >= HIGH_TEMP || myData.temp_c <= LOW_TEMP)){
    button_flag = 0;
  }

  // ------------- LED Alert -------------
  if (myData.temp_c >= HIGH_TEMP || myData.temp_c <= LOW_TEMP) {
    digitalWrite(16, HIGH);
  } else {
    digitalWrite(16, LOW);
  }

  // --------- Display in OLED -----------
  display_temperature(myData.temp_c);
  display.display();    // Show the display buffer on the screen 
}
