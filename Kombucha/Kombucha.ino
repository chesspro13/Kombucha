
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
// OLED variables
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Timing
unsigned long lastInterrupt;
// Other
const String blank = " ";
// Pinouts
// Stepper driver
const int enablePin = 6;  // Pull LOW to enable
const int directionPin = 10;
const int stepPin = 9;
// Rotary Encoder
const int inputButton = 3;
const int inputCLK = 5;
const int inputDT = 4;
// Rotary encoder tracking
int lastCount = 0;
int cnt = 0;
long lastDebounce = 0;
int bounceThreshhold = 500;
bool updateCnt = true;
// Depricated??
bool isRunning = false;
// Input
boolean localButton;
boolean buttonPressed;
boolean curButtonState;
// UI
bool needsRefresh;
int menuArea = 0;
int curSelection = 0;
char output[128];
int menuSelection;
int menuSelectionOffset;
int minMenu;
int maxMenu;
// Menus
char menu[4][20] = { "Start", "Calibrate", "Settings", "Version" };
// String calibrateMenu[] = {"Calibrate btl", "Calibrate 2cups", "Zero bed", "Back"};
// String startMenu[] = {"Bottle", "2 cups", "Back"};
// String startStuff[] = {"Start", "Back"};
// int menuSizes[] = {0, sizeof(menu[0]), sizeof(calibrateMenu[0]), sizeof(startMenu[0]), 2,2,2,2,2};
void setup() {
  Serial.begin(9600);
  // OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
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
  // Producing interfearacne
  // attachInterrupt(digitalPinToInterrupt(inputButton), buttonInturupt, FALLIN
  // splash();
  display.setTextWrap(false);
  menuArea = 0;
  needsRefresh = true;
  drawToScreen();
  menuArea = 1;
  needsRefresh = true;
}
void splash() {
  char name[20] = "Brandon Mauldin's";
  char version[20] = " s";
  char kombucha[8] = "KOMBUKA";
  char wizard[7] = "WIZARD";
  // OLED splash
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.cp437(true);                  // Use full 256 char 'Code Page 437' font
  display.setTextSize(1);               // Normal 1:1 pixel scale
  // display.setTextColor(SSD1306_YELLOW); // Draw white text
  display.setCursor(14, 8);               // Start at top-left corner
  display.print(F("Brandon Mauldin's"));  //auldin");//@mauldin314.com");
  display.setCursor(66, 48);              // Start at top-left corner
  display.print(F("V1.7.0"));
  display.setTextSize(2);     // Normal 1:1 pixel scale
  display.setCursor(35, 32);  // Start at top-left corner
  display.print(F("WIZARD"));
  display.setCursor(25, 16);  // Start at top-left corner
  display.print(F("KOMBUKA"));
  display.display();
  delay(3000);
  // for(;;); // Don't proceed, loop forever
}
void loop() {
  // TODONE: Encoder check
  readEncoder();
  drawToScreen();

  // Switched to inturrupt
  // Button inturrupt has been triggered, need to do stuff about it
  // if( curButtonState == true )
  // {
  // buttonPress();
  // curButtonState = false;
  // }
  // Depricated
  // Rotary inturrupt has been triggered, need to do stuff about it
  // if( rotaryAction )
  // {
  // // This variable makes this method only fire when needed
  // rotaryAction= false;
  // if( lastInput == 1 && currentStateCLK == 0)
  // {
  // clockwise();
  // drawToScreen();
  // }
  // else if (lastInput == 1 && currentStateCLK == 1 )
  // {
  // counterClockwise();
  // drawToScreen();
  // }
  // }
  // Depricated
  // if (scale.is_ready()) {
  // curWeight = scale.read();
  // trueWeight = curWeight - scaleOffset;
  // }
  // Depricated
  // Debug output
  // Serial.println("Weight on scale: " + (String)curWeight );
  // Depricated
  // Draw everything to the screen
  // drawToScreen();
  // Don't need to wait
  // Loop fires only once every second
  // delay( 1000 );
}
// Definately ignore this (Depricated)
// Maybe ignore this
// int getGrams(long x)
// {
// prnt("Rise/run: " + (String)((float)(rise/calRun)+5), "Run/rise: " + (String)((float)(calRun/rise))+5);
// delay( 2000 );
// return (int)((float)(calRun/rise) * ( x - scaleOffset));
// }
// Depricated
// Get current weigt on the scale
// long takeMeasurement()
// {
// long reading = -1;
// delay(250);
// if (scale.is_ready())
// reading = scale.read();
// return reading;
// }

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
void readEncoder() {
  Serial.println(menuSelection);
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
// Depricated
// Interrupt for handeling rotary events
// void roraryInturupt()
// {
// if(millis() - lastInterrupt > 6) // we set a 10ms no-interrupts window
// {
// rotaryAction = true;
// lastInput = digitalRead( inputDT );
// currentStateCLK= digitalRead( inputCLK );
// lastInterrupt = millis();
// }
// }

// Resets navigation stuff
void resetNavigation(int newMenuArea) {
  menuArea = newMenuArea;
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

  switch (menuArea) {
    case 1:
      strcpy(menuText, "Main Menu");
      minMenu = 0;
      maxMenu = sizeof(menu) / sizeof(menu[0]) - 1;
      display.setCursor(centerDisplay(menuText), 0);  // Start at top-left corner
      display.print(menuText);                        //Main Menu12345"));
      for (int i = menuSelectionOffset; i < sizeof(menu) - 1; i++) {
        // if( i - menuSelection > 2)
        // continue;
        display.setCursor(5, (i - menuSelectionOffset) * 16 + 16);
        if (menuSelection == i) {
          strcpy(output, "[");
          strcat(output, menu[i]);
          strcat(output, "]");
        } else {
          strcpy(output, menu[i]);
        }
        Serial.println(output);
        display.println(output);
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

// // Run motor sensed by weight on the scale
void runMotorAuto() {
}

// Need to refactor with current pins
void runMotorHand() {
}

// Need to refactor with OLED
// Updates navigation then print current navigation menu to the OLED
void drawToScreen() {
  if (!needsRefresh)
    return;
  needsRefresh = false;
  switch (menuArea) {
    // Splash screen
    case 0:
      splash();
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
