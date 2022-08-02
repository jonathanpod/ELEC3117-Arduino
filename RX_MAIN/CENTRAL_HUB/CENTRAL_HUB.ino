#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ----------------- WI-FI INITIALISATION -----------------
// MAC address of temperature sensor.
uint8_t broadcastAddress[] = {0xA4, 0xE5, 0x7C, 0x2C, 0xF5, 0x72};

typedef struct struct_message {
    float temp_c;
} struct_message;

typedef struct Threshold {
  int high_temp = 28;
  int low_temp = 10;
} Threshold;

// ----------------- OLED INITIALISATION ------------------
#define OLED_RESET -1                      // Reset pin set to -1 (no reset pin)
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

//
// Create a struct_message called myData
struct_message myData;
Threshold threshold_payload;

// ----------------- CONSTANTS -------------------
#define LEFT_BT   14
#define RIGHT_BT  12
#define SELECT_BT 13
#define DL 100

// ----------------- GLOBAL VARIABLES -------------------
int button_flag = 0;
int state = 0;                    // Set default state to 0
const long interval = 2000;
unsigned long prev_millis = 0;

int high_temp_first_digit = 2;
int high_temp_second_digit = 8;
int low_temp_first_digit = 1;
int low_temp_second_digit = 0;
int high_temp = 28;
int low_temp = 10;

int t_high_temp_first_digit = high_temp_first_digit;
int t_high_temp_second_digit = high_temp_second_digit;
int t_low_temp_first_digit = low_temp_first_digit;
int t_low_temp_second_digit = low_temp_second_digit;

int buzzer_ringing = 0;

// --------------------- USER-DEFINED FUNCTIONS --------------------------

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Temperature: ");
  Serial.print(myData.temp_c); 
  Serial.print("*C");
  Serial.println();
}

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

// Display the temperature in the OLED display.
void display_temperature(){
    
  //Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0);
  display.print("     EnviroHub");
  display.setCursor(0,10); 
  display.print("Temperature: "); 
  display.print(myData.temp_c);
  display.print(" C");
  display.setCursor(0,20); 
  display.print("Upper Limit: "); 
  display.print(high_temp);
  display.print(" C");
  display.setCursor(0,30); 
  display.print("Lower Limit: "); 
  display.print(low_temp);
  display.print(" C");

  display.display();
  
  return;
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
  display.display();

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
  display.display();
  
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
  display.display();
  
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
  display.display();
  
  return;
}

// ---------------------- MAIN -----------------------
void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Begin Wire library for I2C
  Wire.begin();
  
  // Initialize OLED with I2C address 0x3C
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
  display.clearDisplay();
  
  // Initialise ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callbacks
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
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

    // Overwrite state to go to main temperature menu.
    state = 0;
    buzzer_ringing = 1;
  } 
  if (digitalRead(SELECT_BT) == LOW) {
    delay(DL);
    button_flag = 1;
    buzzer_ringing = 0;
  }
  if (!(myData.temp_c >= high_temp || myData.temp_c <= low_temp)){
    button_flag = 0;
    buzzer_ringing = 0;
  }

  // ------------- LED Alert -------------
  if (myData.temp_c >= high_temp || myData.temp_c <= low_temp) {
    digitalWrite(16, HIGH);
  } else {
    digitalWrite(16, LOW);
  }

  // -------------- State Check ----------
  if (state == 0) {
    display_temperature();

    // Reset high temperature and low temperature if state was overwritten to state 0.
    if (buzzer_ringing == 1) {
      high_temp_first_digit = t_high_temp_first_digit;
      high_temp_second_digit = t_high_temp_second_digit;
      low_temp_first_digit = t_low_temp_first_digit;
      low_temp_second_digit = t_low_temp_second_digit;
    }

    // If user presses SEL button, set the upper threshold bounds.
    // Move to state 3.
    if (digitalRead(SELECT_BT) == LOW && buzzer_ringing == 0) {
      delay(DL);
      t_high_temp_first_digit = high_temp_first_digit;
      t_high_temp_second_digit = high_temp_second_digit;
      t_low_temp_first_digit = low_temp_first_digit;
      t_low_temp_second_digit = low_temp_second_digit;
      state = 3;
    }
    
  } else if (state == 3) {
    display_set_first_digit_ut();

    // If user presses right button, increment first digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      delay(DL);
      high_temp_first_digit++;

      // Wrap around the digit. Limit digit to 0-9
      if (high_temp_first_digit > 9) {
        high_temp_first_digit = 0;
      } else if (high_temp_first_digit < 0) {
        high_temp_first_digit = 9;
      }
      
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement first digit.
      delay(DL);
      high_temp_first_digit--;

      // Wrap around the digit. Limit digit to 0-9
      if (high_temp_first_digit > 9) {
        high_temp_first_digit = 0;
      } else if (high_temp_first_digit < 0) {
        high_temp_first_digit = 9;
      }
      
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to setting second digit.
      // Move to state 4
      delay(DL);
      state = 4;
    }
    
  } else if (state == 4){
    display_set_second_digit_ut();
    
    // If user presses right button, increment second digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      delay(DL);
      high_temp_second_digit++;
      // Wrap around the digit. Limit digit to 0-9
      if (high_temp_second_digit > 9) {
        high_temp_second_digit = 0;
      } else if (high_temp_second_digit < 0) {
        high_temp_second_digit = 9;
      }
      
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement second digit.
      delay(DL);
      high_temp_second_digit--;
      // Wrap around the digit. Limit digit to 0-9
      if (high_temp_second_digit > 9) {
        high_temp_second_digit = 0;
      } else if (high_temp_second_digit < 0) {
        high_temp_second_digit = 9;
      }
      
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to setting lower threshold.
      // Move to state 5
      delay(DL);
      state = 5;
    }
    
  } else if (state == 5) {
    display_set_first_digit_lt();

    // If user presses right button, increment first digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      delay(DL);
      low_temp_first_digit++;
      // Wrap around the digit. Limit digit to 0-9
      if (low_temp_first_digit > 9) {
        low_temp_first_digit = 0;
      } else if (low_temp_first_digit < 0) {
        low_temp_first_digit = 9;
      }
      
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement first digit.
      delay(DL);
      low_temp_first_digit--;
      // Wrap around the digit. Limit digit to 0-9
      if (low_temp_first_digit > 9) {
        low_temp_first_digit = 0;
      } else if (low_temp_first_digit < 0) {
        low_temp_first_digit = 9;
      }
      
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to setting second digit.
      // Move to state 6
      state = 6;
    }
    
  } else if (state == 6) {
    display_set_second_digit_lt();

    // If user presses right button, increment second digit.
    if (digitalRead(RIGHT_BT) == LOW) {
      delay(DL);
      low_temp_second_digit++;
      // Wrap around the digit. Limit digit to 0-9
      if (low_temp_second_digit > 9) {
        low_temp_second_digit = 0;
      } else if (low_temp_second_digit < 0) {
        low_temp_second_digit = 9;
      }
      
    } else if (digitalRead(LEFT_BT) == LOW) {
      // If user presses left button, decrement second digit.
      delay(DL);
      low_temp_second_digit--;
      // Wrap around the digit. Limit digit to 0-9
      if (low_temp_second_digit > 9) {
        low_temp_second_digit = 0;
      } else if (low_temp_second_digit < 0) {
        low_temp_second_digit = 9;
      }
      
    } else if (digitalRead(SELECT_BT) == LOW) {
      // If user presses SEL button, move to display temperature.
      // Update high_temp and low_temp
      delay(DL);
      high_temp = (10 * high_temp_first_digit) + high_temp_second_digit;
      low_temp = (10 * low_temp_first_digit) + low_temp_second_digit;
      
      // Move to state 0
      state = 0;
      Serial.print("New upper threshold: ");
      Serial.println(high_temp);
      Serial.print("New lower threshold: ");
      Serial.println(low_temp);

      // Send the new threshold to temperature sensor.
      threshold_payload.high_temp = high_temp;
      threshold_payload.low_temp = low_temp;
      esp_now_send(broadcastAddress, (uint8_t *) &threshold_payload, sizeof(threshold_payload));
      
    }
    
  } else {
    Serial.println("State not defined!");
  }
}
