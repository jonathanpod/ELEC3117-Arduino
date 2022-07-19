#include <OLEDDisplay.h>
#include <OLEDDisplayFonts.h>
#include <OLEDDisplayUi.h>
#include <SH1106.h>
#include <SH1106Brzo.h>
#include <SH1106Spi.h>
#include <SH1106Wire.h>
#include <SSD1306.h>
#include <SSD1306Brzo.h>
#include <SSD1306I2C.h>
#include <SSD1306Spi.h>
#include <SSD1306Wire.h>

void Initial_SSD1306(void){
  Write_command(0xAE); // Display Off
  
  Write_command(0xD5); //SET DISPLAY CLOCK
  Write_command(0x80); //105HZ
  
  Write_command(0xA8); // Select Multiplex Ratio
  Write_command(0x1F);
  
  Write_command(0xD3); //Setting Display Offset
  Write_command(0x00); //00H Reset, set common start
  
  Write_command(0x40); //Set Display Start Line
  
  Write_command(0x8D); // Set Charge Pump
  Write_command(0x14); // Enable Charge Pump
  
  Write_command(0xA1); //Set Segment Re-Map Default
  
  Write_command(0xC8); //Set COM Output Scan Direction
  
  Write_command(0xDA); //Set COM Hardware Configuration
  Write_command(0x12);
  
  Write_command(0x81); //Set Contrast Control
  Write_command(0xFF);
  
  Write_command(0xD9); //Set Pre-Charge period
  Write_command(0x22);
  
  Write_command(0xDB); //Set Deselect Vcomh level
  Write_command(0x30);
  
  Write_command(0xA4); //Entire Display ON
  
  Write_command(0xA6); //Set Normal Display
  Write_command(0xAF); // Display ON
}
