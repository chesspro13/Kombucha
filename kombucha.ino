
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
const unsigned int debounceTime = 200;
unsigned long lastInterrupt;

// Pinouts
  // Stepper driver
const unsigned int enablePin = 6;  // Pull LOW to enable
const unsigned int directionPin = 10;
const unsigned int stepPin = 9;

bool invertMotor;
int steps = 500;
int microstepping = 1;

  // Rotary Encoder
const unsigned int inputButton = 3;
const unsigned int inputCLK = 5;
const unsigned int inputDT = 4;

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

#define backText "Back"
#define trueText "True"
#define falseText "False"
#define openBracket "["
#define closeBracket "]"

const int characters = 10;

  // Menus
const char menu[][characters] = { "Menu", "Run", "Calibrate", "Settings", "Version" };
const char runMenu[][characters] = { "Run", "Bottle 1", "Bottle 2", "Bottle 3", "2 Cups", "Manual", "Auto", backText};
const char settingsMenu[][characters] = {"Settings", "Motor Dir", "Step rate", "Micro Step", backText};
const char calibrateMenu[][characters] = { "Calibrate", &runMenu[1], &runMenu[2], &runMenu[3], &runMenu[4], backText};
const char handMenu[][characters] = { &runMenu[5], "Push", "Pull", backText};
const char motorInvert[][characters] = {"Invert mtr", "Yes", "No", backText};
const char microstepOptions[][characters] = {"Micro Step", "Full", "Half", "1/4", "1/8", "1/16", "1/32", backText};

// const char name[] = "Brandon Mauldin's";
// const char kombucha[] = "KOMBUCHA";
// const char wiz[] = "WIZARD";

char output[16];

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
  // drawToScreen();
  splash();
  menuState = 1;

    // Button Inturrupt
  attachInterrupt(digitalPinToInterrupt(inputButton), buttonInturupt, FALLING );
}


  // OLED splash
void splash() {
  
  display.clearDisplay();

  // printToOled("Brandon Mauldin's", 14, 8, 1);
  // printToOled("KOMBUCHA", 25, 16, 2);
  // printToOled("WIZARD", 35, 32, 2);
  // printToOled(name, 14, 8, 1);
  // printToOled(kombucha, 25, 16, 2);
  // printToOled(wiz, 35, 32, 2);
  // printToOled(VERSION, 66, 48, 1);

  display.setTextSize( 1 );
  display.setCursor( 14, 8 );
  display.print(F("Brandon Mauldin's"));
  display.setCursor( 66, 48 );
  display.print(F("V1.7.P"));

  display.setTextSize( 2 );
  display.setCursor( 25, 16 );
  display.print(F("KOMBUKA"));
  display.setCursor( 35, 32 );
  display.print(F("WIZARD"));

  display.display();
  delay(3000);
  needsRefresh = true;
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
  if (millis() - lastInterrupt > debounceTime)  // we set a 10ms no-interrupts window
  {
    curButtonState = true;
    lastInterrupt = millis();
  }
}


  // Resets navigation stuff
void resetNavigation(int newMenuState) {
  menuState = newMenuState;
  menuSelection = 0;
  menuSelectionOffset = 0;
  needsRefresh = true;


}

  // Be able to change the steps
void microsteppingMenu()
{
  minMenu = 0;
  maxMenu = 1;
  printToOled(settingsMenu[3], 0, 0, 2);

  if(menuSelection == 0)
  {
    strcpy(output, openBracket);
    // strcat( output, "poop" );
    strcat(output, microstepOptions[microstepping]);
    strcat(output, closeBracket);
  }
  else
    strcpy(output, microstepOptions[microstepping]);

  printToOled(output, 5, 16, 2);
  if( menuSelection == 1 )
  {
    strcpy(output, openBracket);
    strcat(output, backText);
    strcat(output, closeBracket);
  }
  else
    strcpy(output, backText);
  printToOled(output, 5, 32, 2);
}

  // Be able to change the steps
void changeSteps()
{
  minMenu = 0;
  maxMenu = 100;
  printToOled(settingsMenu[2], centerDisplay(settingsMenu[2]), 0, 2);
  // char output[12];
  char stepCount[10];
  itoa( 500 + (25 * menuSelection), stepCount, 10);
  
  strcpy(output, stepCount);
  printToOled(output, 5, 24, 4);
}

  // Be able to change the steps
void stepMenu()
{
  minMenu = 0;
  maxMenu = 1;
  printToOled(settingsMenu[2], centerDisplay(settingsMenu[2]), 0, 2);
  // char output[12];
  char stepCount[10];
  itoa( steps, stepCount, 10);

  if(menuSelection == 0)
  {
    strcpy(output, openBracket);
    strcat(output, stepCount);
    strcat(output, closeBracket);
  }
  else
    strcpy(output, stepCount);

  printToOled(output, 5, 16, 2);
  if( menuSelection == 1 )
  {
    strcpy(output, openBracket);
    strcat(output, backText);
    strcat(output, closeBracket);
    printToOled(output, 5, 32, 2);
  }
  else
  {
    display.setTextSize( 2 );
    display.setCursor( 5, 32 );
    display.print(F("Back"));
  }
}

void invertMenu(int option)
{
  // char output[characters + 3];
  minMenu = 0;
  maxMenu = 1;

  printToOled( "Invert", centerDisplay("Invert"), 0, 2);
  if(menuSelection == 0)
  {
    strcpy(output, openBracket);
    if( invertMotor )
      strcat(output, trueText);
    else
      strcat(output, falseText);
    strcat(output, closeBracket);
  }
  else
    if( invertMotor )
      strcpy(output, trueText);
    else
      strcpy(output, falseText);

  printToOled(output, 5, 16, 2);
  if( menuSelection == 1 )
  {
    strcpy(output, openBracket);
    strcat(output, backText);
    strcat(output, closeBracket);
    printToOled(output, 5, 32, 2);
  }
  else
  {
    display.setTextSize( 2 );
    display.setCursor( 5, 32 );
    display.print(F("Back"));
  }
  
}

void menuOutput(char options[][characters], int elements)
{
  // char output[characters + 3];
  minMenu = 0;
  maxMenu = elements-2;

  printToOled(options[0], centerDisplay( options[0] ), 0, 2);

  for (int i = menuSelectionOffset + 1; i <= maxMenu+1; i++) {
  
    if (menuSelection +1 == i) {
      strcpy(output, openBracket);
      strcat(output, options[i]);
      strcat(output, closeBracket);
    } else {
      strcpy(output, options[i]);
    }

    if( maxMenu <= 3 )
      menuSelectionOffset = 0;

    // Serial.println(output);
    printToOled(output, 5, (i - menuSelectionOffset) * 16, 2);
  }
  
}

void printMenu() {
  display.clearDisplay();
  display.setTextSize(2);  // Normal 1:1 pixel scale
  // char menuText[12] = "Menu here";

  switch (menuState) {
      // Main Menu
    case 1:
      menuOutput(menu, 5);
      break;
      // Run Menu
    case 2:
      menuOutput(runMenu, 8);
      break;
      // Calibration Menu
    case 3:
      menuOutput(calibrateMenu, 6);
      break;
      // Settings Menu
    case 4:
      menuOutput(settingsMenu, 5);
      break;
      // Change Motor Direction
    case 5:
      // menuOutput( motorInvert, 4);
      invertMenu(0);
      break;
      // Step Menu
    case 6:
      stepMenu();
      break;
      // Step Changes
    case 7:
      changeSteps();
      break;
      // Micro Stepping
    case 8:
      microsteppingMenu();
      break;
      // Micro Stepping Options
    case 9:
      menuOutput(microstepOptions, 8);
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

  // Serial.print("\nMenu State: ");
  // Serial.println(menuState);
  // Serial.print("Menu Selection: " );
  // Serial.println(menuSelection);
  // Serial.print("Menu Selection Offset: " );
  // Serial.println(menuSelectionOffset);

  // if( menuState == 0 )
  // {
  //   splash();
  //   menuState = 1;
  //   needsRefresh = true;
  //   return;
  // }
  // printMenu();

  switch (menuState) {
    // Splash screen
    case 0:
      splash();
      menuState = 1;
      needsRefresh = true;
      break;
    default:
    // Main Menu
    // case 1:
    // case 2:
    // case 3:
    // case 4:
    // case 5:
      printMenu();
      break;
  }
}

void buttonPressed() {
  curButtonState = false;
  // Serial.println("Button has been pressed");

  switch ( menuState )
  {
      // Main Menu
    case 1:
      switch ( menuSelection )
      {
        case 0:
          // Start
          resetNavigation(2);
          break;
        case 1:
          // Calibrate
          resetNavigation(3);
          break;
        case 2:
          // Settings
          resetNavigation(4);
          break;
        case 3:
          // Version
          break;
      }
      break;
      // Start
    case 2:
      switch( menuSelection )
      {
        case 6:
          resetNavigation(1);
          break;
      }
      break;
      // Calibration
    case 3:
      switch( menuSelection )
      {
        case 4:
          resetNavigation(1);
          break;
      }
      break;
      // Settings
    case 4:
      switch( menuSelection )
      {
          // Change Motor Direction
        case 0:
          resetNavigation(5);
          break;
        case 1:
          resetNavigation(6);
          break;
        case 2:
          resetNavigation(8);
          break;
        case 3:
          resetNavigation(1);
          break;
      }
      break;
      // Motor inverter
    case 5:
      switch( menuSelection )
      {
        case 0:
          // TODO: WRITE TO EEPROM!!
          invertMotor = !invertMotor;
          resetNavigation(5);
        case 1:
          resetNavigation(4);
          break;
      }
      break;
      // Step Menu
    case 6:
      switch( menuSelection )
      {
        case 0:
          resetNavigation(7);
          break;
        case 1:
          resetNavigation(4);
          break;
      }
      break;
      // Step Count
    case 7:
      steps = 500 + (25 * menuSelection);
      resetNavigation(6);
      break;
      // Micro Stepping
    case 8:
      switch( menuSelection )
      {
        case 0:
          resetNavigation(9);
          break;
        case 1:
          resetNavigation(4);
          break;
      }
      break;
      // Micro Stepping Options
    case 9:
        // Plus one because of menu name
      switch( menuSelection + 1 )
      {
        case 1:
        case 2:
        case 3:
        case 4: 
        case 5:
        case 6:
          microstepping = menuSelection + 1;
          resetNavigation(8);
          break;
        case 7:
          resetNavigation(8);
          break;
      }
      break;
  }
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

        if( menuSelectionOffset > maxMenu - 2 )
          menuSelectionOffset = maxMenu - 2;
        // Serial.println("Going UP!");
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
        // Serial.println("Going DOWN!");
        needsRefresh = true;
      }
      break;
  }
}