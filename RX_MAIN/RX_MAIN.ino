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
#define OLED_RESET -1                      // Reset pin set to -1 (no reset pin)
Adafruit_SSD1306 display(OLED_RESET);

//
// Create a struct_message called myData
struct_message myData;

// ----------------- CONSTANTS -------------------
#define LEFT_BT   14
#define RIGHT_BT  12
#define SELECT_BT 13

// ----------------- GLOBAL VARIABLES -------------------
int button_flag = 0;
int state = 0;                    // Set default state to 0

int high_temp_first_digit = 2;
int high_temp_second_digit = 8;
int low_temp_first_digit = 1;
int low_temp_second_digit = 0;
int high_temp = 28;
int low_temp = 10;

// --------------------- USER-DEFINED FUNCTIONS --------------------------

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));      // Copies incomingData into myData
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

void display_set_first_digit_ut() {
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print("Upper Threshold");
  display.setCursor(0,10);
  display.print("Set First Digit");
  display.setCursor(0,20);
  display.print("Temperature:    "); 
  display.print(high_temp_first_digit);
  display.print(high_temp_second_digit);
  display.print(" C");

  return;
}

void display_set_second_digit_ut() {
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print("Upper Threshold");
  display.setCursor(0,10);
  display.print("Set Second Digit");
  display.setCursor(0,20);
  display.print("Temperature:    "); 
  display.print(high_temp_first_digit);
  display.print(high_temp_second_digit);
  display.print(" C");
  
  return;
}

void display_set_first_digit_lt() {
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print("Lower Threshold");
  display.setCursor(0,10);
  display.print("Set First Digit");
  display.setCursor(0,20);
  display.print("Temperature:    "); 
  display.print(low_temp_first_digit);
  display.print(low_temp_second_digit);
  display.print(" C");
  
  return;
}

void display_set_second_digit_lt() {
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print("Lower Threshold");
  display.setCursor(0,10);
  display.print("Set Second Digit");
  display.setCursor(0,20);
  display.print("Temperature:    "); 
  display.print(low_temp_first_digit);
  display.print(low_temp_second_digit);
  display.print(" C");
  
  return;
}

// ---------------------- MAIN -----------------------
void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Begin Wire library for I2C
  Wire.begin();
  
  // Initialize OLED with I2C address 0x3C
  // Change to 0x3D if doesn't work.
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  // Initialise pins
  pinMode(15, OUTPUT);        // Buzzer Output
  pinMode(13, INPUT);         // Button Input
  pinMode(16, OUTPUT);        // LED Output
  pinMode(LEFT_BT, INPUT);    // Left button Input
  pinMode(RIGHT_BT, INPUT);   // Right button Input
  pinMode(SELECT_BT, INPUT);  // Select button Input
  
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

  // ------------ Buzzer Alert ------------
  if (button_flag == 0 && (myData.temp_c >= high_temp || myData.temp_c <= low_temp)){
    digitalWrite(15, HIGH);
    delay(2);
    digitalWrite(15, LOW);
    delay(2);
  } 
  if(digitalRead(13) == LOW) {
    button_flag = 1;
    delay(10);                    // This delay is used for debouncing.
  }
  if (!(myData.temp_c >= high_temp || myData.temp_c <= low_temp)){
    button_flag = 0;
  }

  // ------------- LED Alert -------------
  if (myData.temp_c >= high_temp || myData.temp_c <= low_temp) {
    digitalWrite(16, HIGH);
  } else {
    digitalWrite(16, LOW);
  }

  // -------------- State Check ----------
  if (state == 0) {
    display_temperature(myData.temp_c);
    display.display();              // Show the display buffer on the screen 

    // If user presses SEL button, set the upper threshold bounds.
    // Move to state 3.
    if (digitalRead(SELECT_BT) == LOW) {
      state = 3;
    }
    
  } else if (state == 3) {
    display_set_first_digit_ut();
    display.display();

    // If user presses right button, increment first digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      high_temp_first_digit++;
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement first digit.
      high_temp_first_digit--;
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to setting second digit.
      // Move to state 4
      state = 4;
    }
    
  } else if (state == 4){
    display_set_second_digit_ut();
    display.display();
    
    // If user presses right button, increment second digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      high_temp_second_digit++;
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement second digit.
      high_temp_second_digit--;
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to setting lower threshold.
      // Move to state 5
      state = 5;
    }
    
  } else if (state == 5) {
    display_set_first_digit_lt();
    display.display();

    // If user presses right button, increment first digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      low_temp_first_digit++;
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement first digit.
      low_temp_first_digit--;
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to setting second digit.
      // Move to state 6
      state = 6;
    }
    
  } else if (state == 6) {
    display_set_second_digit_lt();
    display.display();

    // If user presses right button, increment second digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      low_temp_second_digit++;
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement second digit.
      low_temp_second_digit--;
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to display temperature.
      // Update high_temp and low_temp
      high_temp = (10 + high_temp_first_digit) + high_temp_second_digit;
      low_temp = (10 + low_temp_first_digit) + low_temp_second_digit;
      
      // Move to state 0
      state = 0;
    }
    
  } else {
    Serial.println("State not defined!");
  }
  
}
