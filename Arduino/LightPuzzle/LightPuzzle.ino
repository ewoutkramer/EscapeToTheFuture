const int sensorPin = A0; // select the input pin for the potentiometer
const int buttonPin = 2;
const int greenLedPin = 3;
const int LF = 10;

int sensorValue = 0; // variable to store the value coming from the sensor
int buttonState = 0;
bool success = false;

void setup()
{
  // declare the ledPin as an OUTPUT:
  pinMode(buttonPin, INPUT);
  pinMode(greenLedPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);

  Serial.begin(9600);
  Serial.setTimeout(100);
}

void loop()
{
  // read the values from the sensors
  sensorValue = analogRead(sensorPin);
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && sensorValue > 900)
    success = true;

  String cmd = "";

  if (Serial.available())
    cmd = Serial.readStringUntil(LF);

  // report on serial
  if (cmd == F("LightPuzzleReset") || cmd == F("ResetAll"))
  {
    success = false;
  }

  if (success == true)
    digitalWrite(greenLedPin, HIGH);
  else
    digitalWrite(greenLedPin, LOW);

  Serial.print(F("LightPuzzleSensor: "));
  Serial.println(sensorValue);
  Serial.print(F("LightPuzzleButton: "));
  Serial.println(buttonState);
  Serial.print(F("LightPuzzleSuccess: "));
  Serial.println(success);

  delay(500);
}
