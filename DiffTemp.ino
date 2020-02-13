#include <Wire.h>
#include <math.h>

int sensorPin = A0;
int sensorValue = 0;
float voltage;
float V1 = 5.0;
float clusterRes = 2073.0;
int highFlag = 0;

//address of the 7 segment led display
const byte s7sAddress = 0x71;

void setup() 
{
  //begin communication with 7-segment display
  Wire.begin();
  
  //clear the display and set cursor to the left
  clearDisplayI2C();
  
  //set display brightness - default is 255 (full high)
  setBrightnessI2C(255);
  delay(1000);

   //clear the display one final time and set the cursor to the left
  clearDisplayI2C();
  setDecimalsI2C(0b000100);
  pinMode(3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 20; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN,LOW);
    delay(150);
  }
  Serial.begin(9600);
}

void loop() 
{
  delay(250);
  digitalWrite(3, LOW);
  sensorValue = analogRead(sensorPin); //Read A0
  voltage = sensorValue * (5.0 / 1023.0); //Correlate sensor value to voltage
  float temp = getTemp(getSensorResistance(voltage)); //calls getTemp function by getting sensor resistance based on voltage
  Serial.print(temp);
  Serial.print("    " + String(highFlag));
  Serial.println();
  
  s7sSendStringI2C(String(int(temp)));
  delay(20);

  if (temp >= 220.0)
  {
    digitalWrite(3, HIGH);
    while (temp == 305.0)
  {
    sensorValue = analogRead(sensorPin); //Read A0
    voltage = sensorValue * (5.0 / 1023.0); //Correlate sensor value to voltage
    temp = getTemp(getSensorResistance(voltage));
    s7sSendStringI2C(String("5hit"));
    delay(500);
    s7sSendStringI2C("    ");
    delay(500);
  }
    while ((temp >= 190.0 && temp <= 220.0) || (temp > 220 && temp != 305.0))
    {
      sensorValue = analogRead(sensorPin); //Read A0
      voltage = sensorValue * (5.0 / 1023.0); //Correlate sensor value to voltage
      temp = getTemp(getSensorResistance(voltage)); //calls getTemp function by getting sensor resistance based on voltage
      Serial.print(String(int(temp)) + "    " + String(highFlag));
      Serial.println();
      s7sSendStringI2C(String(int(temp)));
      delay(20);
    }
  }


  //OR SCENARIO FOR ONE MINUTE
  // if temp>= 220.0
  //{
  //    digitalWrite(3,HIGH); //HIGH OUTPUT
  //    delay(60000); //Delay 1 Minute
  //}
}

float getSensorResistance(float V2)
{
  return (clusterRes * V2)/(V1-V2);
}

float getTemp(float resistance)
{
  float temp;
  if (resistance <= 48.1)
  {
    temp = 305.0;
  }
  else if (resistance > 48.1 && resistance <= 69.1)
  {
    temp = ceil((.0131047*pow(resistance,2) -2.81891*resistance + 407.237)*100) / 100;
  }
  else if (resistance > 69.1 && resistance <= 134.7)
  {
    temp = ceil(( -.0000299607*pow(resistance,3)+ .0131945*pow(resistance, 2) -2.40934*resistance + 388.352 ))*100/100;
  }

  else if (resistance > 134.7 && resistance <= 287)
  {
    temp = ceil((-.00000298751*pow(resistance,3)+ .00272654*pow(resistance,2) -1.02937*resistance + 326.463 ))*100/100;
  }

  else if (resistance > 287 && resistance <= 819)
  {
    temp = ceil((((-1.5323 * pow(10, -7))*pow(resistance,3)) + .000365177 * pow(resistance, 2) - .353805*resistance + 259.963 ))*100/100;
  }

  else if (resistance > 819 && resistance <= 2268)
  {
    temp = ceil((((-6.222 * pow(10, -9))*pow(resistance, 3)) + .00004081*pow(resistance,2) - .109254*resistance + 196.447 ))*100/100;
  }

  else if (resistance > 2268 && resistance <= 9516)
  {
    temp = ceil((((-1.1136 * pow(10, -10) * pow(resistance, 3))) + .00000273966 * pow(resistance, 2) - .0266401*resistance + 133.322 ))*100/100;
  }

  else if (resistance > 9516 && resistance <= 39064)
  {
    temp = ceil((((-1.3344 * pow(10, -12) * pow(resistance, 3))) + (1.3488 * pow(10, -7) * pow(resistance, 2)) - .0054169*resistance + 72.3059 ))*100/100;
  }
  return temp;
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
