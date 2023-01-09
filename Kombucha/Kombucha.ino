
/**************************************************************************
* Kombucha Machine
**************************************************************************/
// OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// ??
#include <stdio.h>
#include <stdlib.h>

#define VERSION "V1.7.P"

// OLED variables
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  // Timing
unsigned long lastInterrupt;

// Pinouts
  // Stepper driver
const int enablePin = 6;  // Pull LOW to enable
const int directionPin = 10;
const int stepPin = 9;

  // Rotary Encoder
const int inputButton = 3;
const int inputCLK = 5;
const int inputDT = 4;

  // Input
bool curButtonState;

  // UI
bool needsRefresh;
int menuState = 0;
int curSelection = 0;
int menuSelection;
int menuSelectionOffset;
int minMenu;
int maxMenu;
char output[128];

  // Menus
char menu[4][20] = { "Start", "Calibrate", "Settings", "Version" };
char calibrateMenu[5][20] = {"Bottle 1", "Bottle 2", "Bottle 3", "2 Cups", "Back"};

void setup() {
  Serial.begin(9600);

    // OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Don't proceed, loop forever
  }

    // Stepper motor driver
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  pinMode(directionPin, OUTPUT);
  pinMode(stepPin, OUTPUT);

    // Input
  pinMode(inputCLK, INPUT);
  pinMode(inputDT, INPUT);
  pinMode(inputButton, INPUT_PULLUP);
  
    // OLED Setup
  display.setTextWrap(false);
  display.cp437(true);
  display.setTextColor(SSD1306_WHITE);  // Draw white text

    // Set menu state 
  menuState = 0;
  needsRefresh = true;
  drawToScreen();

    // Button Inturrupt
  attachInterrupt(digitalPinToInterrupt(inputButton), buttonInturupt, FALLING );
}


  // OLED splash
void splash() {
  
  display.clearDisplay();

  printToOled("Brandon Mauldin's", 14, 8, 1);
  printToOled("KOMBUCHA", 25, 16, 2);
  printToOled("WIZARD", 35, 32, 2);
  printToOled(VERSION, 66, 48, 1);

  display.display();
  delay(3000);
}

void printToOled(char* output, int x, int y, int textSize) {
  display.setTextSize( textSize );
  display.setCursor( x, y );
  display.print( output );
}

void loop() {
  // TODONE: Encoder check
  readEncoder();
  drawToScreen();

  // Switched to inturrupt
  // Button inturrupt has been triggered, need to do stuff about it
  if( curButtonState == true )
    buttonPressed();
}


// Inturrutp for handeling button presses
void buttonInturupt() {
  //curButtonState = true;
  // Debouncing
  if (millis() - lastInterrupt > 10)  // we set a 10ms no-interrupts window
  {
    curButtonState = true;
    lastInterrupt = millis();
  }
}

void buttonPressed() {
  curButtonState = false;
  Serial.println("Button has been pressed");
}


void readEncoder() {
  // Serial.println(menuSelection);
  static uint8_t state = 0;
  bool clkState = digitalRead(inputCLK);
  bool dtState = digitalRead(inputDT);
  switch (state) {
    case 0:
      if (!clkState) {
        state = 1;
      } else if (!dtState) {
        state = 4;
      }
      break;
    case 1:
      if (!dtState) {
        state = 2;
      }
      break;
    case 2:
      if (clkState) {
        state = 3;
      }
      break;
    case 3:
      if (clkState && dtState) {
        state = 0;
        menuSelection++;
        if (menuSelection >= maxMenu)
          menuSelection = maxMenu;
        //else
        {
          if (menuSelection > 2 + menuSelectionOffset) {
            menuSelectionOffset += 1;
          }
        }
        Serial.println("Going UP!");
        needsRefresh = true;
      }
      break;
    case 4:
      if (!clkState) {
        state = 5;
      }
      break;
    case 5:
      if (dtState) {
        state = 6;
      }
      break;
    case 6:
      if (clkState && dtState) {
        state = 0;
        menuSelection--;
        if (menuSelection < 1) {
          if (menuSelection <= minMenu)
            menuSelection = minMenu;
          menuSelectionOffset -= 1;
          if (menuSelectionOffset < 0)
            menuSelectionOffset = 0;
          if (menuSelection == 0)
            menuSelectionOffset = 0;
        }
        Serial.println("Going DOWN!");
        needsRefresh = true;
      }
      break;
  }
}

  // Resets navigation stuff
void resetNavigation(int newMenuState) {
  menuState = newMenuState;
  curSelection = 0;
}


  // Repeat a string pattern X times
String repeatX(String str, int X) {
  String ret = "";
  for (int i = 0; i < X; i++)
    ret += str;
  return ret;
}


  // Starts the botteling process
void start(String item) {
  if (item == "bottle") {
    // if ( calibratedBottle )
    // {
    // // runMotorAuto();
    // }
  }
}


void printMenu() {
  display.clearDisplay();
  display.setTextSize(2);  // Normal 1:1 pixel scale
  char menuText[12] = "Menu here";

  switch (menuState) {
    case 1:
      strcpy(menuText, "Main Menu");
      minMenu = 0;
      maxMenu = sizeof(menu) / sizeof(menu[0]) - 1;

      printToOled(menuText, centerDisplay( menuText ), 0, 2);

      for (int i = menuSelectionOffset; i < sizeof(menu) - 1; i++) {
      
        if (menuSelection == i) {
            // Put brackets around menu item
          {
            strcpy(output, "[");
            strcat(output, menu[i]);
            strcat(output, "]");
          }
        } else {
          strcpy(output, menu[i]);
        }

        Serial.println(output);
        printToOled(output, 5, (i - menuSelectionOffset) * 16 + 16, 2);
      }
      break;
  }

  display.display();
}


  // Calculate padding needed for center spacing
int centerDisplay(char* item) {
  if (strlen(item) > 10)
    return 0;
  return (127 - strlen(item) * 11 - 1) / 2;
}

  // Run motor sensed by weight on the scale
void runMotorAuto() {
}

  // Need to refactor with current pins
void runMotorHand() {
}

  // Updates navigation then print current navigation menu to the OLED
void drawToScreen() {
  if (!needsRefresh)
    return;
  needsRefresh = false;

  switch (menuState) {
    // Splash screen
    case 0:
      splash();
      menuState = 1;
      needsRefresh = true;
      break;
    // Main Menu
    case 1:
    case 2:
    case 3:
    case 4:
      printMenu();
      break;
  }
}