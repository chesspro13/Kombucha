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
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Timing

unsigned long lastInterrupt;

// Other

const String blank = "                ";

// Pinouts

  // Stepper driver
const int enablePin = 6;      // Pull LOW to enable
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
int menuArea = 0;
int curSelection = 0;
String lines[] = {};

// Menus
String menu[]={"Start", "Calibrations", "Back"};
String calibrateMenu[] = {"Calibrate btl", "Calibrate 2cups", "Zero bed", "Back"};
String startMenu[] = {"Bottle", "2 cups", "Back"};
String startStuff[] = {"Start", "Back"};
int menuSizes[] = {0, sizeof(menu[0]), sizeof(calibrateMenu[0]), sizeof(startMenu[0]), 2,2,2,2,2};

void setup() {
  Serial.begin(9600);

     // OLED Setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Stepper motor driver
  
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  pinMode( directionPin, OUTPUT );
  pinMode( stepPin, OUTPUT );

  // Input

  pinMode( inputCLK, INPUT );
  pinMode( inputDT, INPUT );
  pinMode( inputButton, INPUT_PULLUP );
    // Producing interfearacne 
  // attachInterrupt(digitalPinToInterrupt(inputButton), buttonInturupt, FALLING);

  // OLED splash  
  display.clearDisplay();
  
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.setTextSize(1);      // Normal 1:1 pixel scale

  // display.setTextColor(SSD1306_YELLOW); // Draw white text
  // display.setCursor(0, 1);     // Start at top-left corner
  // display.println("brandonmauldin");//@mauldin314.com");
  // display.setCursor(0, 48);     // Start at top-left corner
  // display.println(F("          V1.70"));

  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setCursor(0, 16);     // Start at top-left corner
  display.println("  KOMBUKA");
  display.setCursor(0, 32);     // Start at top-left corner
  display.println("   WIZARD");
  
  display.display();
  delay(1000);
}


void loop() {

  // TODO: Encoder check

    // Switched to inturrupt
  // Button inturrupt has been triggered, need to do stuff about it
  // if( curButtonState == true )
  // {
  //   buttonPress();
  //   curButtonState = false;
  // }

    // Depricated
  // Rotary inturrupt has been triggered, need to do stuff about it
  // if( rotaryAction )
  // {
  //   // This variable makes this method only fire when needed
  //   rotaryAction= false;
  //   if( lastInput == 1 && currentStateCLK == 0)
  //   {
  //     clockwise();
  //     drawToScreen();
  //   }
  //   else if (lastInput == 1 && currentStateCLK == 1 )
  //   {
  //     counterClockwise();
  //     drawToScreen();
  //   }
  // }

    // Depricated
  // if (scale.is_ready()) {
  //   curWeight = scale.read();
  //   trueWeight = curWeight - scaleOffset;
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
//   prnt("Rise/run: " + (String)((float)(rise/calRun)+5), "Run/rise: " + (String)((float)(calRun/rise))+5);
//   delay( 2000 );
//   return (int)((float)(calRun/rise) * ( x - scaleOffset));  
// }

  // Depricated
// Get current weigt on the scale
// long takeMeasurement()
// {
//   long reading = -1;
//   delay(250);
//   if (scale.is_ready())
//     reading = scale.read();
//   return reading;
// }


// Inturrutp for handeling button presses
void buttonInturupt()
{
  //curButtonState = true;
    // Debouncing
  if(millis() - lastInterrupt > 10) // we set a 10ms no-interrupts window
    {    
      curButtonState = true;
      lastInterrupt = millis();
    }
}

  // Depricated
// Interrupt for handeling rotary events
// void roraryInturupt()
// {
//   if(millis() - lastInterrupt > 6) // we set a 10ms no-interrupts window
//     {    
//       rotaryAction = true;
//       lastInput = digitalRead( inputDT );
//       currentStateCLK= digitalRead( inputCLK );
//       lastInterrupt = millis();
//     }
// }


// Rotary dial has been turned clockwise
void clockwise()
{
  if (menuArea != 0)
    curSelection++;
  if (curSelection > menuSizes[menuArea] -1)
    curSelection = 0;
}


// Rotary dial has been turned counter clockwise
void counterClockwise()
{
  if (menuArea != 0)
    curSelection--;
  if (curSelection < 0)
    curSelection = menuSizes[menuArea] -1;
}


// Resets navigation stuff
void resetNavigation(int newMenuArea)
{
  menuArea = newMenuArea;
  curSelection = 0;
}


// Repeat a string pattern X times
String repeatX(String str, int X)
{
  String ret = "";
  for( int i=0; i< X; i++)
    ret += str;
  return ret;
}


// Starts the botteling process
void start(String item)
{
  if ( item == "bottle" )
  {
    // if ( calibratedBottle )
    // {
    //   // runMotorAuto();
    // }
  }
}

  // Needs to be refactored for OLEd
// Prints to LCD screen
void prnt( String lineA, String lineB)
{
  String outputA = "";
  String outputB = "";
  String errorMessage = "ERROR: STR2LONG!";

  if( lineA.length() > 16)
    outputA = errorMessage;
  else
    outputA = lineA;

  if( lineB.length() > 16)
    outputB = errorMessage;
  else
    outputB = lineB;
}

  // Depricated
// Zero the scale
// void zeroScale()
// {
//   int delayTime = 250;
//   long readings[3];

//   prnt(" Reading  Scale ", "====");
//   delay( delayTime );
//   for( int i = 0; i < 3; i++ )
//   {
//     prnt(" Reading  Scale ", "====" + repeatX("====",i+1));
//     readings[i] = takeMeasurement();
//     delay( delayTime );
//   }

//   scaleOffset = (readings[0] + readings[1] + readings[2])/3;

//   prnt( "Calibration done", blank);
//   delay( delayTime/2);
//   calibratedZero = true;
// }

  // Depricated
// Get the weight of 2 cups of kombucha
// void zeroCups()
// {
//   localButton = true;
  
//   prnt("Place empty cup", "then press btn");
//   while( !curButtonState )
//     delay( 500 );
//   buttonPressed = false;
//   containerWeight = takeMeasurement() - scaleOffset;

//   curButtonState = false;
//   delay( 500 );
  

//   prnt("Fill with 2 cups", "then press btn" );
//   while( !curButtonState )
//     delay( 500 );
//   buttonPressed = false;
  
//   cupsWeight = takeMeasurement() - scaleOffset - containerWeight;

//   curButtonState = false;

//   prnt("Cont: " + (String)containerWeight, "2 cups: " + (String) (cupsWeight+containerWeight));
//   delay(2000);
//   localButton = false;
// }

  // Depricated
// Just for testing
// void zeroGram()
// {
//   localButton = true;
  
//   int delayTime = 250;
//   long readings[3];


//   prnt("Place empty cup", "then press btn");
//   while( !curButtonState )
//     delay( 500 );

//   prnt(" Reading  Scale ", "====");
//   delay( delayTime );
//   for( int i = 0; i < 3; i++ )
//   {
//     prnt(" Reading  Scale ", "====" + repeatX("====",i+1));
//     readings[i] = takeMeasurement();
//     delay( delayTime );
//   }

//   scaleOffset = (readings[0] + readings[1] + readings[2])/3;

//   prnt( "5 grams ", (String)scaleOffset);
//   delay( 999999999999);
  
//   localButton = false;
// }


// // Run motor sensed by weight on the scale
// void runMotorAuto()
// {
//   reading = 0;
//   if (scale.is_ready()) {
//     reading = scale.read();

//   while( reading -scaleOffset -containerWeight < 20000)
//   //if( reading < 200000 )
//   {
//     if (digitalRead(toggleSwitch) == HIGH) {
//       digitalWrite(motorTerminal1, LOW); //these logic levels create forward direction
//       digitalWrite(motorTerminal2, HIGH);
//     }
//     else {
//       digitalWrite(motorTerminal1, HIGH); // these logic levels create reverse direction
//       digitalWrite(motorTerminal2, LOW);
//     }
//     prnt("Fill: " + (String)((float)((trueWeight-containerWeight)/20000)*100), blank);
//   }//else
//   {
//       digitalWrite(motorTerminal1, LOW); // these logic levels create reverse direction
//       digitalWrite(motorTerminal2, LOW);
//   }
  
//   }   
// }

  // Need to refactor with current pins
void runMotorHand()
{
  // // reading = 0;
  //   // Depricated
  // // if (scale.is_ready()) {
  // //   reading = scale.read();
  // if( isRunning )
  // {
  //     // Depricated
  //   // if (digitalRead(toggleSwitch) == HIGH) {
  //   //   digitalWrite(motorTerminal1, LOW); //these logic levels create forward direction
  //   //   digitalWrite(motorTerminal2, HIGH);
  //   }
  //   else {
  //       // Depricated
  //     // digitalWrite(motorTerminal1, HIGH); // these logic levels create reverse direction
  //     // digitalWrite(motorTerminal2, LOW);
  //   }
  // }else
  // {
  //       // Depricated
  //     // digitalWrite(motorTerminal1, LOW); // these logic levels create reverse direction
  //     // digitalWrite(motorTerminal2, LOW);
  // }
  
  // }   
}


void buttonPress()
{ 

  if( localButton )
  {
    buttonPressed = true;
    prnt("Button already", "pressed!");
  }else{
    
    switch( menuArea )
    {
      // Splash screen
      case 0:
        resetNavigation( 1 );
        break;
  
      // Main menu
      case 1:
        // Start menu
        switch( curSelection )
        {
          case 0:
            resetNavigation( 3);
            break;
          case 1:
            resetNavigation( 2);
            break;
          case 3:
            resetNavigation( 0 );
            break;
        }
        break;
  
      // Calibration selection
      case 2:
        switch( curSelection )
        {
          case 0:
            resetNavigation(6);
            break;
          case 1:
            resetNavigation(7);
            break;
          case 2:
            resetNavigation(0);
            break;
          case 3:
            resetNavigation(1);
            break;
        }
        break;
  
      // Start Menu
      case 3:
        switch( curSelection )
        {
          case 0:
            resetNavigation(4);
            break;
          case 1:
            resetNavigation(5);
            break;
          case 2:
            resetNavigation(1);
            break;
        }
        break;
  
      // Starts
      case 4:
        switch( curSelection )
        {
          case 0:
            resetNavigation( 0 );
            start("bottle"); 
            break;
          case 1:
            resetNavigation( 3 );
            break;
        }
        break;
      case 5:
        switch( curSelection )
        {
          case 0:
            resetNavigation( 0 );
            start("bottle"); 
            break;
          case 1:
            resetNavigation( 3 );
            break;
        }
        break;
      // Calibrations
      case 6:
        switch( curSelection )
        {
          case 0:
            // Do calibratinos
            //resetNavigation(0);
              // Depricated
            // zeroScale();
            break;
          case 1:
            resetNavigation(2);
            break;
        }
        break;
      case 7:
        switch( curSelection )
        {
          case 0:
            // Do calibratinos
            resetNavigation(0);
            break;
          case 1:
            resetNavigation(2);
            break;
        }
        break;
      case 8:
        switch( curSelection )
        {
          case 0:
            // Do calibratinos
            resetNavigation(0);
            break;
          case 1:
            resetNavigation(2);
            break;
        }
        break;
    }
  }
  drawToScreen();
}


  // Need to refactor with OLED
// Updates navigation then print current navigation menu to the LCD screen
void drawToScreen()
{

  Serial.print("menu: " + (String)menuArea + "\t selection: " + (String)curSelection );
    // Depricated
  // if( curWeight > maxWeight )
  if( false )
  {
    prnt("Item too heavy!!", "Please remove it");
  }else{
    
    String calibrationText = "Put itm on scl";
    String start = ">Start      Back";
    String back = "Start      >Back";
    
    Serial.print("\tIn the loop");
    switch( menuArea )
    {
      // Splash screen
      case 0:
        prnt( "    Kombucha    ", "      Town      ");
        break;
        
      // Main Menu
      case 1:
        switch( curSelection )
        {
          case 0:
            prnt( ">" + menu[0], " " + menu[1]);
            break;
          case 1:
            prnt( " " + menu[0], ">" + menu[1]);
            break;
          case 2:
            prnt( ">" + menu[2], blank);
            break;
        }
        break;
  
      // Calibrate Menu
      case 2:
        switch( curSelection )
        {
          case 0:
            prnt( ">" + calibrateMenu[0], " " + calibrateMenu[1]);
            break;
          case 1:
            prnt( " " + calibrateMenu[0], ">" + calibrateMenu[1]);
            break;
          case 2:
            prnt( ">" + calibrateMenu[2], " " + calibrateMenu[3]);
            break;
          case 3:
            prnt( " " + calibrateMenu[2], ">" + calibrateMenu[3]);
            break;
        }
        break;
  
      // Start Menu
      case 3:
        switch( curSelection )
        {
          case 0:
            prnt( ">" + startMenu[0], startMenu[1]);
            break;
          case 1:
            prnt( startMenu[0], ">" + startMenu[1]);
            break;
          case 2:
            prnt( ">" + startMenu[2], blank);
            break;
        }
        break;
  
      // Calibrations
      case 6:
      case 7:
      case 8:
        switch( curSelection )
        {
          case 0:
            prnt( calibrationText, start);
            break;
          case 1:
            prnt( calibrationText, back);
            break;
        }
        break;
  
      // Starts
      case 4:
      case 5:
        switch( curSelection )
        {
          case 0:
            prnt( calibrationText, start);
            break;
          case 1:
            prnt( calibrationText, back);
            break;
        }
        break;
    }
  }
  if( false )
    prnt( (String)menuArea,(String) curSelection);
}
