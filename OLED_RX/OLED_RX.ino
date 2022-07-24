// Include Wire Library for I2C
#include <Wire.h>

// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Reset pin not used but needed for library
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// ------------------------- MAIN ------------------------
void setup() {
  // Start Wire library for I2C
  Wire.begin();
  
  // initialize OLED with I2C addr 0x3D
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  
}

void display_temperature(){
  // Delay to allow sensor to stabilize
  delay(2000);

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

void loop() {
  display_temperature();
  display.display();      // Show the display buffer on the screen
}
