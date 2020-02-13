int sensorPin = A0;
int sensorValue = 0;
float voltage;
float V1 = 5.0;
float clusterRes = 2073.0;
int ledPin = 13;


void setup() 
{
  pinMode(3, OUTPUT);
  pinMode(ledPin, OUTPUT);
  for (int i = 0; i < 20; i++)
  {
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin,LOW);
  }

}

void loop() 
{
  sensorValue = analogRead(sensorPin); //Read A0
  voltage = sensorValue * (5.0 / 1023.0); //Correlate sensor value to voltage
  float temp = getTemp(getSensorResistance(voltage)); //calls getTemp function by getting sensor resistance based on voltage
  if (temp >= 220.0)
  {
    while (getTemp(getSensorResistance(voltage)) >= 190.0)
    {
      digitalWrite(3, HIGH);
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
  if (resistance > 48.1 && resistance <= 69.1)
  {
    temp = .0131047*pow(resistance,2) -2.81891*resistance + 407.237;
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
  }
  else if (resistance > 69.1 && resistance <= 134.7)
  {
    temp = -.0000299607*pow(resistance,3)+ .0131945*pow(resistance, 2) -2.40934*resistance + 388.352;
    for (int i = 0; i < 2; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
  }

  else if (resistance > 134.7 && resistance <= 287)
  {
    temp = -.00000298751*pow(resistance,3)+ .00272654*pow(resistance,2) -1.02937*resistance + 326.463;
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
  }

  else if (resistance > 287 && resistance <= 819)
  {
    temp = ((-1.5323 * pow(10, -7))*pow(resistance,3)) + .000365177 * pow(resistance, 2) - .353805*resistance + 259.963;
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
  }

  else if (resistance > 819 && resistance <= 2268)
  {
    temp = ((-6.222 * pow(10, -9))*pow(resistance, 3)) + .00004081*pow(resistance,2) - .109254*resistance + 196.447;
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
  }

  else if (resistance > 2268 && resistance <= 9516)
  {
    temp = ((-1.1136 * pow(10, -10) * pow(resistance, 3))) + .00000273966 * pow(resistance, 2) - .0266401*resistance + 133.322;
    for (int i = 0; i < 6; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
  }

  else if (resistance > 9516 && resistance <= 39064)
  {
    temp = ((-1.3344 * pow(10, -12) * pow(resistance, 3))) + (1.3488 * pow(10, -7) * pow(resistance, 2)) - .0054169*resistance + 72.3059;
    for (int i = 0; i < 7; i++)
    {
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
    }
  }
  return temp;
}
