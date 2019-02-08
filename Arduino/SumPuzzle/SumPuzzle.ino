#include "SevSeg.h"
#include "Keypad.h"

const byte ROWS = 4; // number of rows
const byte COLS = 4; // number of columns
char keys[ROWS][COLS] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {29, 28, 27, 26};
byte colPins[COLS] = {25, 24, 23, 22};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
SevSeg sevseg;

void setup()
{
  byte numDigits = 1;
  byte digitPins[] = {};
  byte segmentPins[] = {6, 5, 2, 3, 4, 7, 8, 9};
  bool resistorsOnSegments = true;

  byte hardwareConfig = COMMON_CATHODE;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);

  Serial.begin(9600);
  Serial.setTimeout(100);

  reset();
}

const long NEWNUM_INTERVAL = 4000;
const long BLINK_INTERVAL = 500;
const long INPUT_TIMEOUT = 2000;
const long SERIAL_UPDATE_INTERVAL = 5000;

unsigned long lastNewNum;
unsigned long lastBlink;
unsigned long lastKeyPress;
unsigned long lastSerialUpdateSent;

byte input1;
byte input2;
byte targetNumber;
bool displayOff;

const byte NO_INPUT_NUM = 11;
const String MY_NAME = "SumPuzzle";
const int LF = 10;

String serialAction;
bool sendSerialUpdate;

bool success;
bool debug;

void reset()
{
  success = false;
  debug = false;
  targetNumber = -1;
  displayOff = true;
  input1 = NO_INPUT_NUM;
  input2 = NO_INPUT_NUM;
  sendSerialUpdate = false;
  serialAction = "";

  lastNewNum = 0;
  lastBlink = 0;
  lastKeyPress = 0;
  lastSerialUpdateSent = 0;
}

void loop()
{
  checkInputs();
  processSerialActions();
  processState();
  updateOutputs();
}

void checkInputs()
{
  checkKeyPress();
  checkSerial();
  checkTimers();
}

void checkKeyPress()
{
  char key = keypad.getKey();

  if (key != NO_KEY)
  {
    lastKeyPress = millis();
    if (key >= '0' && key < '9')
    {
      byte newNum = key - '0';
      input1 = input2;
      input2 = newNum;
    }
    else if (key == 'A')
      reset();
  }
}

void checkSerial()
{
  if (Serial.available())
  {
    String received = Serial.readStringUntil(LF);
    bool payAttention = received.startsWith(MY_NAME + ":") || received.startsWith("All:");

    if (payAttention)
    {
      int separator = received.indexOf(':');
      serialAction = received.substring(separator + 1);
    }
  }
}

void checkTimers()
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastSerialUpdateSent > SERIAL_UPDATE_INTERVAL)
    onSerialUpdateNeeded();

  if (!success)
  {
    if (currentMillis - lastKeyPress > INPUT_TIMEOUT)
      onInputTimeout();

    if (currentMillis - lastNewNum >= NEWNUM_INTERVAL)
      onNeedNewNumber();
  }
  else
  {
    if (currentMillis - lastBlink >= BLINK_INTERVAL)
      onBlink();
  }
}

void onInputTimeout()
{
  input1 = NO_INPUT_NUM;
  input2 = NO_INPUT_NUM;
}

void onNeedNewNumber()
{
  lastNewNum = millis();

  byte newNumber = random(9);
  while (targetNumber == newNumber)
    newNumber = random(9);

  targetNumber = newNumber;
}

void onBlink()
{
  lastBlink = millis();
  displayOff = !displayOff;
}

void onSerialUpdateNeeded()
{
  lastSerialUpdateSent = millis();
  sendSerialUpdate = true;
}

void processSerialActions()
{
  if (serialAction == F("RESET"))
  {
    serialAction;
    reset();
  }
  else if (serialAction == F("DEBUG-ON"))
  {
    debug = true;
  }
  else if (serialAction == F("DEBUG-OFF"))
  {
    debug = false;
  }

  serialAction = "";
}

void processState()
{
  if(!success && targetNumber != -1)
    displayOff = false;
    
  if (input1 + input2 == targetNumber)
    success = true;
}

void updateOutputs()
{
  updateDisplay();

  if (sendSerialUpdate)
  {
    sendSerialUpdate = false;

    if (debug)
      statusToSerial();

    successToSerial();
  }
}

void updateDisplay()
{
  if (!displayOff)
  {
    sevseg.setNumber(targetNumber);
    sevseg.refreshDisplay();
  }
  else
  {
    sevseg.blank();
  }
}

void successToSerial()
{
  Serial.print(MY_NAME + F("-success:"));
  Serial.println(success);
}

void statusToSerial()
{
  Serial.print(MY_NAME + F("-debug:"));
  
  Serial.print(F("targetNumber="));
  Serial.print(targetNumber);

  Serial.print(F(",input1="));
  if (input1 != NO_INPUT_NUM)
    Serial.print(input1);
  else
    Serial.print("-");

  Serial.print(F(",input2="));
  if (input2 != NO_INPUT_NUM)
    Serial.print(input2);
  else
    Serial.print("-");

  Serial.print(F(",success="));
  Serial.println(success);
}