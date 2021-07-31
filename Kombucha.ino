#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// HX711 Scale Stuff
HX711 scale;
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 11;


unsigned long lastInterrupt;

// Other

const String blank = "                ";
const int toggleSwitch = 6;
const int motorTerminal1 = 8;
const int motorTerminal2 = 7;
const int enablePin = 9;
bool isRunning = false;

// Input
const int runpin = 5;
const int inputCLK = 4;
const int inputDT = 3;
String encdir = "";
volatile boolean isButtonPressed;
boolean isTurning;

bool lastButtonState;
bool curButtonState;
boolean buttonPressed;

int counter = 0;
volatile int currentStateCLK;
volatile int previousStateCLK;

long scaleOffset;
int scaleReading;
// Screen Stuff
LiquidCrystal_I2C lcd(0x27, 16, 2);
long reading;

// UI
int menuArea = 0;
int curSelection = 0;

boolean calibratedCups;
boolean calibratedBottle;
boolean calibratedZero;

String menu[]={"Start", "Calibrations", "Back"};
String calibrateMenu[] = {"Calibrate btl", "Calibrate 2cups", "Zero bed", "Back"};
String startMenu[] = {"Bottle", "2 cups", "Back"};
String startStuff[] = {"Start", "Back"};
int menuSizes[] = {0, sizeof(menu[0]), sizeof(calibrateMenu[0]), sizeof(startMenu[0]), 2,2,2,2,2};

// Scale stuff
int count = 0;

long curWeight;
const long maxWeight = (long)(1000000 * 0.75);
long bottleWeight;

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

long trueWeight;
int gramsFull = 470; //Not including container
int rise = 11622;//21436;
int calRun = 25;//47;

boolean turning;

boolean localButton;

//Add idle animation
int idle;

// Maybe ignore this
int getGrams(long x)
{
  prnt("Rise/run: " + (String)((float)(rise/calRun)+5), "Run/rise: " + (String)((float)(calRun/rise))+5);
  delay( 2000 );
  return (int)((float)(calRun/rise) * ( x - scaleOffset));  
}

void setup() {

  // Setup
  pinMode(toggleSwitch, INPUT);
  pinMode(motorTerminal1, OUTPUT);
  pinMode(motorTerminal2, OUTPUT);
  pinMode(enablePin, OUTPUT);

  // Enable H bridge
  digitalWrite(enablePin, HIGH);
  
  int buttonState;
  

  // For the scale
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Input
  pinMode( inputCLK, INPUT );
  pinMode( inputDT, INPUT );
  //pinMode(runpin, INPUT_PULLUP);
  isButtonPressed = false;
  

  previousStateCLK = digitalRead( inputCLK );
  

  // LCD Stuff
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Kombucha  Wizard");
  lcd.setCursor(0,1);
  lcd.print("     V0.0.1     ");
  delay( 1000 );

  if( calibratedZero == false )
  {
    lcd.setCursor(0,0);
    lcd.print("Self calibration");
    lcd.setCursor(0,1);
    lcd.print("   initiating   ");
    delay( 1000 );
    zeroScale();
    delay( 1000 );
  }
  turning = false;

  lastButtonState = false;
  attachInterrupt(digitalPinToInterrupt(2), tst, LOW);
  attachInterrupt(digitalPinToInterrupt(3), bloop, CHANGE);
  lcd.setCursor(0,0);

  //zeroCups();
  calibratedCups = true;
  calibratedBottle = true;
  calibratedZero = true;

  drawToScreen();

  //zeroGram();
}

volatile boolean needPrint;

void tst()
{
  //curButtonState = true;
  if(millis() - lastInterrupt > 10) // we set a 10ms no-interrupts window
    {    
      curButtonState = true;
      lastInterrupt = millis();

    }
}
volatile byte lastInput;
volatile byte curClock;
volatile boolean wait;
void bloop()
{
  if(millis() - lastInterrupt > 6) // we set a 10ms no-interrupts window
    {    
     
  needPrint = true;
  
  lastInput = digitalRead( inputDT );
  //previousStateCLK = currentStateCLK;
  currentStateCLK= digitalRead( inputCLK );
  //wait = true;
  
      lastInterrupt = millis();

    }
}

int max = 32695; 
int zero = 25108;

void loop() {
  //lastInput = digitalRead( inputDT );
  if( curButtonState == true )
  {
    buttonPress();
    curButtonState = false;
  }
  
  if( needPrint )
  {
    //       prnt("Counter: " + (String) ((int)count), encdir);
    needPrint= false;
    //prnt((String) lastInput, (String) currentStateCLK );

    if( lastInput == 1 && currentStateCLK == 0)
    {
      //prnt("Clockwise",blank);
      clockwise();
      drawToScreen();
    }
    else if (lastInput == 1 && currentStateCLK == 1 )
    {
      //prnt("CounterClockwise",blank);
      counterClockwise();
      drawToScreen();
    }
    /*
    if (lastInput != currentStateCLK) { 
      encdir ="CW";
      count+=1;
      //clockwise();
      prnt("Counter: " + (String) ((int)count/2), encdir);
      turning = true;
      //drawToScreen();
    } else {
      encdir ="CCW";
      //counterClockwise();
      count-=1;
      //drawToScreen();
      prnt("Counter: " + (String) ((int)count/2), encdir);
      turning = true;
    }
  }
   //previousStateCLK = currentStateCLK; */
  }
 // Serial.print( (String)scaleReading + "\n");


  if (scale.is_ready()) {
    //scaleReading = scale.read();
    curWeight = scale.read();
    trueWeight = curWeight - scaleOffset;
  }

  Serial.println("Weight on scale: " + (String)curWeight );
  
  drawToScreen();
  delay( 1000 );
}
    
  // Read the current state of inputCLK
  // currentStateCLK = digitalRead(inputCLK);
  // curButtonState = digitalRead(runpin);

   /*
/*
   if( lastButtonState == HIGH && curButtonState == LOW )
   {
    if( isButtonPressed == false)
    {
      isButtonPressed = true;
      buttonPress();
    } 
   }else
      isButtonPressed = false;
* /

      //prnt(blank,blank);
      //prnt("menu# " + (String) menuArea ,blank);
    //drawToScreen();
   
    
   // If the previous and the current state of the inputCLK are different then a pulse has occured
   if (currentStateCLK != previousStateCLK){ 
       
     // If the inputDT state is different than the inputCLK state then 
     // the encoder is rotating counterclockwise
   if( turning )
    turning = false;
   else{
     if (digitalRead(inputDT) != currentStateCLK) { 
       encdir ="CW";
       count+=1;
       clockwise();
       //prnt("Counter: " + (String) ((int)count), encdir);
       turning = true;
       drawToScreen();
     } else {
       encdir ="CCW";
       counterClockwise();
       count-=1;
       drawToScreen();
       //prnt("Counter: " + (String) ((int)count), encdir);
       turning = true;
     }}
   } 
   // Update previousStateCLK with the current state
   previousStateCLK = currentStateCLK; 
   lastButtonState = curButtonState;
   //delay(0500);
   * /
   previousStateCLK = currentStateCLK; 
}*/

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

void clockwise()
{
  if (menuArea != 0)
    curSelection++;
  if (curSelection > menuSizes[menuArea] -1)
    curSelection = 0;
}

void counterClockwise()
{
  if (menuArea != 0)
    curSelection--;
  if (curSelection < 0)
    curSelection = menuSizes[menuArea] -1;
}

void setStuff(int newMenuArea)
{
  menuArea = newMenuArea;
  curSelection = 0;
}

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
  
  lcd.setCursor(0,0);
  lcd.print(blank);
  lcd.setCursor(0,1);
  lcd.print(blank);

  if( true )
  {
    lcd.setCursor(0,0);
    lcd.print(outputA );//                                   ");
    lcd.setCursor(0,1);
    lcd.print(outputB );//+ "                                         ");
  }else{
    lcd.setCursor(0,0);
    lcd.print("Menu: " + (String) menuArea + "            ");
    lcd.setCursor(0,1);
    lcd.print("Selection: " + (String) curSelection + "                    ");
  }
}

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

long takeMeasurement()
{
  long reading = -1;
  delay(250);
  if (scale.is_ready()) {
    reading = scale.read();
  }

  return reading;
}

// Repeat a string pattern X times
String repeatX(String str, int X)
{
  String ret = "";

  for( int i=0; i< X; i++)
    ret += str;

  return ret;
}

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
        setStuff( 1 );
        break;
  
      // Main menu
      case 1:
        // Start menu
        switch( curSelection )
        {
          case 0:
            setStuff( 3);
            break;
          case 1:
            setStuff( 2);
            break;
          case 3:
            setStuff( 0 );
            break;
        }
        break;
  
      // Calibration selection
      case 2:
        switch( curSelection )
        {
          case 0:
            setStuff(6);
            break;
          case 1:
            setStuff(7);
            break;
          case 2:
            setStuff(0);
            break;
          case 3:
            setStuff(1);
            break;
        }
        break;
  
      // Start Menu
      case 3:
        switch( curSelection )
        {
          case 0:
            setStuff(4);
            break;
          case 1:
            setStuff(5);
            break;
          case 2:
            setStuff(1);
            break;
        }
        break;
  
      // Starts
      case 4:
        switch( curSelection )
        {
          case 0:
            setStuff( 0 );
            start("bottle"); 
            break;
          case 1:
            setStuff( 3 );
            break;
        }
        break;
      case 5:
        switch( curSelection )
        {
          case 0:
            setStuff( 0 );
            start("bottle"); 
            break;
          case 1:
            setStuff( 3 );
            break;
        }
        break;
      // Calibrations
      case 6:
        switch( curSelection )
        {
          case 0:
            // Do calibratinos
            //setStuff(0);
            zeroScale();
            break;
          case 1:
            setStuff(2);
            break;
        }
        break;
      case 7:
        switch( curSelection )
        {
          case 0:
            // Do calibratinos
            setStuff(0);
            break;
          case 1:
            setStuff(2);
            break;
        }
        break;
      case 8:
        switch( curSelection )
        {
          case 0:
            // Do calibratinos
            setStuff(0);
            break;
          case 1:
            setStuff(2);
            break;
        }
        break;
    }
  }
  drawToScreen();
}

String getFill()
{
  //
}

void drawToScreen()
{

  Serial.print("menu: " + (String)menuArea + "\t selection: " + (String)curSelection );
  if( curWeight > maxWeight )
  {
    prnt("Item too heavy!!", "Please remove it");
    //return;
  }else{
    
    String calibrationText = "Put itm on scl";
    String start = ">Start      Back";
    String back = "Start      >Back";
    
    Serial.print("\tIn the loop");
    switch( menuArea )
    {
      // Splash screen
      case 0:
        Serial.println("\tAt splash screen");
        /*long f = takeMeasurement();/*
        String fillPercent;
        if( f - scaleOffset < containerWeight - containerWeight * 0.1  )
          fillPercent = "ERR";
        else
          fillPercent = (String)(int)(((float)(f - scaleOffset - containerWeight)/(float)(cupsWeight))*100)+"%";
        prnt( (String)( f - scaleOffset - containerWeight) + "::" + (String) cupsWeight, (String)(f-scaleOffset) + "::" + d);
        //prnt( (String)( curWeight - scaleOffset), (String)((curWeight-scaleOffset)/cupsWeight) + "%" );*/
        //prnt( "Weight: " + (String)getGrams(f) + "g ", "Target: " + (String)gramsFull + "g");
        prnt( blank, blank );
        break;
        
      // Main Menu
      case 1:
        Serial.println("\tAt main menu");
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
