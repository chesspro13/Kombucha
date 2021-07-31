#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Orange inits
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;

// HX711 Scale Stuff
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 11;

// Timing
unsigned long lastInterrupt;

// Other

const String blank = "                ";

// Pinouts
const int toggleSwitch = 6;
const int motorTerminal1 = 8;
const int motorTerminal2 = 7;
const int enablePin = 9;
const int runpin = 5;
const int inputCLK = 4;
const int inputDT = 3;

bool isRunning = false;

// Input
  // Button
boolean localButton;
boolean buttonPressed;
boolean curButtonState;
  
  // Rotary stuff
volatile int currentStateCLK;
volatile int previousStateCLK;
volatile byte lastInput;
volatile byte curClock;
volatile boolean rotaryAction;

// UI
int menuArea = 0;
int curSelection = 0;
String curLineA;
String curLineB;

// Calibration checks
boolean calibratedCups;
boolean calibratedBottle;
boolean calibratedZero;

// Menus
String menu[]={"Start", "Calibrations", "Back"};
String calibrateMenu[] = {"Calibrate btl", "Calibrate 2cups", "Zero bed", "Back"};
String startMenu[] = {"Bottle", "2 cups", "Back"};
String startStuff[] = {"Start", "Back"};
int menuSizes[] = {0, sizeof(menu[0]), sizeof(calibrateMenu[0]), sizeof(startMenu[0]), 2,2,2,2,2};

// Scale stuff
long scaleOffset;
long reading;
long curWeight;
const long maxWeight = (long)(1000000 * 0.75);
long bottleWeight;
long trueWeight;

// Tried to figure shit out...
long containerWeight = 208780;
int gramsContainer=466;
long bottle = 104418;
long grams5 = 109063;
long grams10 = 111506;
long grams20 = 116363;
long grams40 = 125269;
long grams80 = 143929;
long grams242 = 217536;
long grams242Real = 113118;
long cupsWeight = 423872 - containerWeight;

int grams5Real = 4645;
int grams10Real = 7088;
int grams20Real = 11945;
int grams40Real = 20851;
int grams80Real = 39511;
//(5,4645),(10,7088),(20,11945),(40,20851),(80,39511)
//1&2: 1.53
//2&3: 1.69
//3&4: 1.74
//4&5: 1.89

int gramsFull = 470; //Not including container
int rise = 11622;//21436;
int calRun = 25;//47;

//Add idle animation
int idle;

//OTHER
int max = 32695; 
int zero = 25108;


void setup() {
  
  // For the scale
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  // Setup
  pinMode(toggleSwitch, INPUT);
  pinMode(motorTerminal1, OUTPUT);
  pinMode(motorTerminal2, OUTPUT);
  pinMode(enablePin, OUTPUT);

  // Enable H bridge
  digitalWrite(enablePin, HIGH);

  // Input
  pinMode( inputCLK, INPUT );
  pinMode( inputDT, INPUT );
  //pinMode(runpin, INPUT_PULLUP);
  //isButtonPressed = false;  

  // LCD init
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  prnt( "Kombucha  Wizard","     V0.0.1     ");
  delay( 1000 );

  // Calibrate the bed
  if( calibratedZero == false )
  {
    prnt( "Self calibration", "   initiating   ");
    delay( 1000 );
    zeroScale();
    delay( 1000 );
  }

  // Add interrupts for the button and rotary encoder
  attachInterrupt(digitalPinToInterrupt(2), buttonInturupt, LOW);
  attachInterrupt(digitalPinToInterrupt(3), roraryInturupt, CHANGE);

  //zeroCups();
  calibratedCups = true;
  calibratedBottle = true;
  calibratedZero = true;

  // Additional setup
  previousStateCLK = digitalRead( inputCLK );
  //lastButtonState = false;

  // Display the splash screen
  drawToScreen();
}


void loop() {
  // Button inturrupt has been triggered, need to do stuff about it
  if( curButtonState == true )
  {
    buttonPress();
    curButtonState = false;
  }

  // Rotary inturrupt has been triggered, need to do stuff about it
  if( rotaryAction )
  {
    // This variable makes this method only fire when needed
    rotaryAction= false;
    if( lastInput == 1 && currentStateCLK == 0)
    {
      clockwise();
      drawToScreen();
    }
    else if (lastInput == 1 && currentStateCLK == 1 )
    {
      counterClockwise();
      drawToScreen();
    }
  }

  if (scale.is_ready()) {
    curWeight = scale.read();
    trueWeight = curWeight - scaleOffset;
  }

  // Debug output
  Serial.println("Weight on scale: " + (String)curWeight );

  // Draw everything to the screen
  drawToScreen();

  // Loop fires only once every second
  delay( 1000 );
}


// Maybe ignore this
int getGrams(long x)
{
  prnt("Rise/run: " + (String)((float)(rise/calRun)+5), "Run/rise: " + (String)((float)(calRun/rise))+5);
  delay( 2000 );
  return (int)((float)(calRun/rise) * ( x - scaleOffset));  
}


// Get current weigt on the scale
long takeMeasurement()
{
  long reading = -1;
  delay(250);
  if (scale.is_ready())
    reading = scale.read();
  return reading;
}


// Inturrutp for handeling button presses
void buttonInturupt()
{
  //curButtonState = true;
  if(millis() - lastInterrupt > 10) // we set a 10ms no-interrupts window
    {    
      curButtonState = true;
      lastInterrupt = millis();
    }
}


// Interrupt for handeling rotary events
void roraryInturupt()
{
  if(millis() - lastInterrupt > 6) // we set a 10ms no-interrupts window
    {    
      rotaryAction = true;
      lastInput = digitalRead( inputDT );
      currentStateCLK= digitalRead( inputCLK );
      lastInterrupt = millis();
    }
}


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
    if ( calibratedBottle )
    {
      runMotorAuto();
    }
  }
}


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

  // Check to see if the line changes, if not don't waste the time and energy re-wrighting to the screen
  if( outputA != curLineA)
  {
    lcd.setCursor(0,0);
    lcd.print(blank);
    curLineA = outputA;
    lcd.setCursor(0,0);
    lcd.print(outputA );
  }

  if( outputB != curLineB)
  {
    lcd.setCursor(0,1);
    lcd.print(blank);
    curLineB = outputB;
    lcd.setCursor(0,1);
    lcd.print(outputB );
  }
}


// Zero the scale
void zeroScale()
{
  int delayTime = 250;
  long readings[3];

  prnt(" Reading  Scale ", "====");
  delay( delayTime );
  for( int i = 0; i < 3; i++ )
  {
    prnt(" Reading  Scale ", "====" + repeatX("====",i+1));
    readings[i] = takeMeasurement();
    delay( delayTime );
  }

  scaleOffset = (readings[0] + readings[1] + readings[2])/3;

  prnt( "Calibration done", blank);
  delay( delayTime/2);
  calibratedZero = true;
}


// Get the weight of 2 cups of kombucha
void zeroCups()
{
  localButton = true;
  
  prnt("Place empty cup", "then press btn");
  while( !curButtonState )
    delay( 500 );
  buttonPressed = false;
  containerWeight = takeMeasurement() - scaleOffset;

  curButtonState = false;
  delay( 500 );
  

  prnt("Fill with 2 cups", "then press btn" );
  while( !curButtonState )
    delay( 500 );
  buttonPressed = false;
  
  cupsWeight = takeMeasurement() - scaleOffset - containerWeight;

  curButtonState = false;

  prnt("Cont: " + (String)containerWeight, "2 cups: " + (String) (cupsWeight+containerWeight));
  delay(2000);
  localButton = false;
}


// Just for testing
void zeroGram()
{
  localButton = true;
  
  int delayTime = 250;
  long readings[3];


  prnt("Place empty cup", "then press btn");
  while( !curButtonState )
    delay( 500 );

  prnt(" Reading  Scale ", "====");
  delay( delayTime );
  for( int i = 0; i < 3; i++ )
  {
    prnt(" Reading  Scale ", "====" + repeatX("====",i+1));
    readings[i] = takeMeasurement();
    delay( delayTime );
  }

  scaleOffset = (readings[0] + readings[1] + readings[2])/3;

  prnt( "5 grams ", (String)scaleOffset);
  delay( 999999999999);
  
  localButton = false;
}


// Run motor sensed by weight on the scale
void runMotorAuto()
{
  reading = 0;
  if (scale.is_ready()) {
    reading = scale.read();

  while( reading -scaleOffset -containerWeight < 20000)
  //if( reading < 200000 )
  {
    if (digitalRead(toggleSwitch) == HIGH) {
      digitalWrite(motorTerminal1, LOW); //these logic levels create forward direction
      digitalWrite(motorTerminal2, HIGH);
    }
    else {
      digitalWrite(motorTerminal1, HIGH); // these logic levels create reverse direction
      digitalWrite(motorTerminal2, LOW);
    }
    prnt("Fill: " + (String)((float)((trueWeight-containerWeight)/20000)*100), blank);
  }//else
  {
      digitalWrite(motorTerminal1, LOW); // these logic levels create reverse direction
      digitalWrite(motorTerminal2, LOW);
  }
  
  }   
}


void runMotorHand()
{
  reading = 0;
  if (scale.is_ready()) {
    reading = scale.read();
  if( isRunning )
  {
    if (digitalRead(toggleSwitch) == HIGH) {
      digitalWrite(motorTerminal1, LOW); //these logic levels create forward direction
      digitalWrite(motorTerminal2, HIGH);
    }
    else {
      digitalWrite(motorTerminal1, HIGH); // these logic levels create reverse direction
      digitalWrite(motorTerminal2, LOW);
    }
  }else
  {
      digitalWrite(motorTerminal1, LOW); // these logic levels create reverse direction
      digitalWrite(motorTerminal2, LOW);
  }
  
  }   
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
            zeroScale();
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


// Updates navigation then print current navigation menu to the LCD screen
void drawToScreen()
{

  Serial.print("menu: " + (String)menuArea + "\t selection: " + (String)curSelection );
  if( curWeight > maxWeight )
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
