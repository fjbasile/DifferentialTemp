#include <Wire.h>
#include <math.h>

//**************************************************************************************************
//THESE ARE YOUR PUMP TEMPERATURE SETTINGS. PUMP CURRENTLY COMES ON AT 220 AND STAYS ON UNTIL 190 **
//                                                                                                **
//THE OVERHEAT TEMP WILL MOST LIKELY NEVER CHANGE BUT I PUT IT THERE FOR EASY REFERENCE           **
//**************************************************************************************************
int pumpCutoffTemp = 190; //threshold temperature to cut pump back off
int pumpTurnOnTemp = 220; //threshold temperature to turn pump on
int overheatTemp = 305; //special case in which temp exceeds abilities of calculations
int pumpTurnOnTime; //initial variable for determining whether pump should bump on

//**************************************************************************************************
//                             THESE ARE THE BLINK ON/OFF TIMES                                   **
//                                 AND OUTPUT PIN SETTINGS                                        **
//**************************************************************************************************
int ledPin = 13; //LED pin for LED output
int blinkOnTime = 500;
int blinkOffTime = 5000;
bool ledFlag = false;
bool pumpBumpDone = false;
unsigned long previousTime = millis();
unsigned long currentTime = millis();

int pumpPin = 3;
int sensorPin = A0; //analog input is A0
int sensorValue = 0;
float voltage;
float V1 = 5.0; //rail voltage
float clusterRes = 2073.0; //cluster resistance
float temp;
String tempWord;
unsigned long sysTime;
unsigned long tempTime;
unsigned long blinkTime;


//creates mirrored hex digits representative of 7 segment display numbers 
const byte zero = 0x3F;
const byte one = 0x30;
const byte two = 0x6D;
const byte three = 0x79;
const byte four = 0x72;
const byte five = 0x5B;
const byte six = 0x5E;
const byte seven = 0x31;
const byte eight = 0x7F;
const byte nine = 0x73;
const byte F = 0x47;

//address of the 7 segment led display
const byte s7sAddress = 0x71;

void setup() 
{
  //begin communication with 7-segment display
  Wire.begin();
  
  //clear the display and set cursor to the left
  clearDisplayI2C();

  //********************************************************
  //set display brightness - default is 255 (full high)   **
  //********************************************************
  setBrightnessI2C(127);
  
  delay(1000);

  //clear the display one final time and set the cursor to the left
  clearDisplayI2C();
  //turn off decimal point
  setDecimalsI2C(0b000000);
  //pin 3 is digital output
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  sysTime = millis();  

  //determines whether pump should run for 10000 ms (10 seconds) based on initial temps
  if (getMeasurements() <= 60)
  {
    pumpTurnOnTime = 0;
  }
  else
  {
    pumpTurnOnTime = 10000;
  }

  //pumpTurnOnTime = 0; //uncomment this and comment out above if-else statement to get rid of initial "turn on"
}

void loop() 
{ 
  tempTime = millis();

  //this is basically a time delay for the pump turn on/off 
  if (tempTime - sysTime >= pumpTurnOnTime && pumpBumpDone == false)
  {
    pumpBumpDone = true;
    digitalWrite(ledPin, LOW);
    ledFlag = false;
    digitalWrite(pumpPin, HIGH); //pump turns off with high output
  }

  else if (tempTime - sysTime < pumpTurnOnTime && pumpBumpDone == false)
  {
    digitalWrite(ledPin, HIGH); //turns or keeps the LED on to indicate pump running
    ledFlag = true;
    digitalWrite(pumpPin,LOW); //pump turns on with low output
  }

  delay(500); //ARBITRARY DELAY FOR FLICKERING*********************
  

  //MOST COMPLICATED BLINKING OF AN LED KNOWN TO MAN************************************************************
  currentTime = millis(); //get the current time

  //if we are not in the initial pump bump AND if the led is off AND exceeded the maximum blink off time 
  if(pumpBumpDone == true && ledFlag == false && (currentTime - previousTime > blinkOffTime))
  {    
    previousTime = millis(); // save the last system time of when the LED was off
    digitalWrite(ledPin, HIGH); //turn the LED on
    ledFlag = true; //change the led state boolean to true
  }

  //if we are not in the initial pump bump AND if the led is on AND exceeded the maximum blink on time 
  else if (pumpBumpDone == true && ledFlag == true && (currentTime - previousTime > blinkOnTime))
  {
    previousTime = currentTime; //save the last system time of when the LED was on
    digitalWrite(ledPin, LOW); //turn the LED off
    ledFlag = false; //change the led state boolean to false
  }
  //************************************************************************************************************
  
  temp = getMeasurements();
  buildDisplayOutput(int(temp)); //comment out for normal display
  //s7sSendStringI2C(buildTempWord(temp)); //uncomment for normal display

  if (temp >= pumpTurnOnTemp)
  {
    digitalWrite(ledPin, HIGH); //turns the LED on to indicate pump is running
    digitalWrite(pumpPin, LOW); //turns the pump on
    ledFlag = true;
    while (temp == overheatTemp)
    {
        digitalWrite(ledPin, HIGH); //ensures LED stays on
        ledFlag = true;
        temp = getMeasurements();
        //s7sSendStringI2C(String("5hit")); //uncomment for normal display
        buildDisplayOutput(int(temp)); //comment out for normal display
        delay(500);
        s7sSendStringI2C("    ");
        delay(500);
    }
    while ((temp >= pumpCutoffTemp && temp <= pumpTurnOnTemp ) || (temp > pumpTurnOnTemp && temp != overheatTemp))
    {
      digitalWrite(ledPin, HIGH); //ensures LED stays on
      ledFlag = true;
      temp = getMeasurements();
      buildDisplayOutput(int(temp)); //comment out for normal display
      //s7sSendStringI2C(buildTempWord(temp)); //uncomment for normal display
      delay(500); //ARBITRARY DELAY FOR FLICKERING****************
    }
  }
  
  //OR SCENARIO FOR ONE MINUTE PUMP RUN
  // if temp>= 220.0
  //{
  //    digitalWrite(3,HIGH); //HIGH OUTPUT
  //    delay(60000); //Delay 1 Minute
  //}
}



/*
 * This function reads an analog value between 0 and 1023 from the temp sensor, and then converts that to a voltage
 * on a 5 volt scale. It then calls getTemp() by passing the voltage to getSensorResistance() first
*/
float getMeasurements()
{
  sensorValue = analogRead(sensorPin); //Read A0
  voltage = sensorValue * (V1 / 1023.0); //Correlate sensor value to voltage
  return getTemp(getSensorResistance(voltage)); //calls getTemp function by getting sensor resistance based on voltageget
}

/*
 * This calculates the sensor resistance based on a re-configured voltage divider formula
 */
float getSensorResistance(float V2)
{
  return (clusterRes * V2)/(V1-V2);
}

/*
 * Calculates the actual temperature based on the resistance calculated in getSensorResistance(). 
 * There are 8 individual formulas calculating the temperature based on specific resistances. Since
 * the sensor is not linear, it had to be done this way in order to account for the entire temperature
 * band. Temps are in Fahrenheit.
 */
float getTemp(float resistance)
{
  float temp;
  if (resistance <= 48.1)
  {
    temp = 305.0;
  }
  else if (resistance > 48.1 && resistance <= 69.1)
  {
    temp = .0131047*pow(resistance,2) -2.81891*resistance + 407.237;
  }
  else if (resistance > 69.1 && resistance <= 134.7)
  {
    temp = -.0000299607*pow(resistance,3)+ .0131945*pow(resistance, 2) -2.40934*resistance + 388.352;
  }

  else if (resistance > 134.7 && resistance <= 287)
  {
    temp = -.00000298751*pow(resistance,3) + .00272654*pow(resistance,2) - 1.02937*resistance + 326.463;
  }

  else if (resistance > 287 && resistance <= 819)
  {
    temp = ((-1.5323 * pow(10, -7))*pow(resistance,3)) + .000365177 * pow(resistance, 2) - .353805*resistance + 259.963;
  }

  else if (resistance > 819 && resistance <= 2268)
  {
    temp = ((-6.222 * pow(10, -9))*pow(resistance, 3)) + .00004081*pow(resistance,2) - .109254*resistance + 196.447;
  }

  else if (resistance > 2268 && resistance <= 9516)
  {
    temp = ((-1.1136 * pow(10, -10) * pow(resistance, 3))) + .00000273966 * pow(resistance, 2) - .0266401*resistance + 133.322;
  }

  else if (resistance > 9516 && resistance <= 39064)
  {
    temp = ((-1.3344 * pow(10, -12) * pow(resistance, 3))) + (1.3488 * pow(10, -7) * pow(resistance, 2)) - .0054169*resistance + 72.3059;
  }
  return temp;
}

/*
 * This function builds the temperature to be displayed in a NON-MIRRORED fashion
 */
String buildTempWord(float temp)
{
  if (temp >= 100)
  {
      return String(int(temp)) + "F";
  }
  else if (temp <100 && temp >= 10)
  {
    return (" " + String(int(temp)) + "F");    
  }

  else if (temp < 10 && temp >= 0)
  {
    return ("  " + String(int(temp)) + "F");
  }

  else if (temp < 0 && temp >= -20)
  {
    return ("  " + String(int(temp)) + "F");
  }
}

// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array
void s7sSendStringI2C(String toSend)
{
  Wire.beginTransmission(s7sAddress);
  for (int i = 0; i < 4; i ++)
  {
    Wire.write(toSend[i]);
  }
  Wire.endTransmission();
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplayI2C()
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x76);
  Wire.endTransmission();
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightnessI2C(byte value)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x7A);
  Wire.write(value);
  Wire.endTransmission();
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimalsI2C(byte decimals)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x77);
  Wire.write(decimals);
  Wire.endTransmission(); 
}

/*
 * This function is a custom function that mirrors the seven-segment displays 
 * output to be a mirror image. This way the display can be viewed in a mirror.
 * Complicated hex stuff ahead 
 */
void buildDisplayOutput(int temp)
{
  const byte digits[4] = {0x7B, 0x7C, 0x7D, 0x7E};
  const byte nums[11] = {zero, one, two, three, four, five, six, seven, eight, nine, F};
  const byte tempNums[4] = {};
    
  if (temp == 305)
  {
    const byte tempNums[4] = {0x4E, 0x10, 0x56, five}; //spells 'shit' mirrored
    Wire.beginTransmission(s7sAddress);
    for (int i = 0; i < 4; i++)
    {
      Wire.write(digits[i]);
      Wire.write(tempNums[i]);
    }
    Wire.endTransmission();
  }
  
  else if (temp >= 100)
  {
    int digitOne = temp/100;
    int digitTwo = temp/10%10;
    int digitThree = temp%10;
    const byte tempNums[4] = {F, nums[digitThree], nums[digitTwo],nums[digitOne]}; 
    Wire.beginTransmission(s7sAddress);
    for (int i = 0; i < 4; i++)
    {
      Wire.write(digits[i]);
      Wire.write(tempNums[i]);
    }
    Wire.endTransmission();
  }

  else if (temp < 100 && temp >= 10)
  {
    int digitOne = temp/10%10;
    int digitTwo = temp%10;
    const byte tempNums[4] = {F, nums[digitTwo], nums[digitOne], 0x00};
    Wire.beginTransmission(s7sAddress);
    for (int i = 0; i < 4; i++)
    {
      Wire.write(digits[i]);
      Wire.write(tempNums[i]);
    }
    Wire.endTransmission();
  }

  else if (temp < 10 && temp >= 0)
  {
    int digitOne = temp%10;
    const byte tempNums[4] = {F, nums[digitOne], 0x00, 0x00};
    Wire.beginTransmission(s7sAddress);
    for (int i = 0; i < 4; i++)
    {
      Wire.write(digits[i]);
      Wire.write(tempNums[i]);
    }
    Wire.endTransmission();
  }

  else if (temp < 0 && temp >= -10)
  {
    int digitOne = abs(temp%10);
    const byte tempNums[4] = {F, nums[digitOne], 0x40, 0x00};
    Wire.beginTransmission(s7sAddress);
    for (int i = 0; i < 4; i++)
    {
      Wire.write(digits[i]);
      Wire.write(tempNums[i]);
    }
    Wire.endTransmission();      
  }

  else if (temp <= -10 && temp > -99)
  {
    int digitOne = abs(temp/10%10);
    int digitTwo = abs(temp%10);
    const byte tempNums[4] = {F, nums[digitTwo], nums[digitOne], 0x40};
    Wire.beginTransmission(s7sAddress);
    for (int i = 0; i < 4; i++)
    {
      Wire.write(digits[i]);
      Wire.write(tempNums[i]);
    }
    Wire.endTransmission();
  } 
}
