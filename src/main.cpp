#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SimpleSerialProtocol.h>

/*** SERIAL COMMUNICATION DECLARATIONS ***/
// declare callbacks (this is boilerplate code but needed for proper compilation of the sketch)
void onError(uint8_t errorNum);
void onReceivedLedColor();

const long BAUDRATE = 9600;
const long CHARACTER_TIMEOUT = 500;

const byte COMMAND_ID_LED_COLOR = 'l';
const byte COMMAND_ID_CARD_SCAN = 'c';
const byte COMMAND_ID_BUTTON_PRESS = 'b';

// Create instance. Pass Serial instance. Define command-id-range within Simple Serial Protocol is listening (here: a - z)
SimpleSerialProtocol ssp(Serial, STREAM_TYPE_HARDWARESERIAL, BAUDRATE, CHARACTER_TIMEOUT, onError, 'a', 'z');

/*** BUTTON DECLARATIONS ***/
const int PIN_PLAY = 3;
// const int PIN_PAUSE = 2;
const int PIN_STOP = 7;

void onPlayButton();
void onPauseButton();
void onStopButton();

/*** RGB LED DECLARATIONS ***/
const int PIN_RED = 5;
const int PIN_GREEN = 6;
const int PIN_BLUE = 9;

/*** RFID READER DECLARATIONS ***/
#define RST_PIN 8
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
byte uids[3][7] = {
    {0x04, 0x48, 0x59, 0x12, 0xAA, 0x52, 0x81},
    {0x04, 0x9D, 0xF5, 0x12, 0xAA, 0x52, 0x80},
    {0x04, 0xA7, 0x30, 0x12, 0xAA, 0x52, 0x80},
};
byte matchedCard = 0xFF;

void setup()
{
  /*** SERIAL COMMUNICATION SETUP ***/
  ssp.init();
  ssp.registerCommand(COMMAND_ID_LED_COLOR, onReceivedLedColor);

  /*** BUTTON SETUP ***/
  attachInterrupt(digitalPinToInterrupt(PIN_PLAY), onPlayButton, RISING);
  // attachInterrupt(digitalPinToInterrupt(PIN_PAUSE), onPauseButton, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_STOP), onStopButton, RISING);

  /*** RGB LED SETUP ***/
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  /*** RFID READER SETUP ***/
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
}

void loop()
{
  ssp.loop();

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  for (int i = 0; i < 3; i++)
  {
    if (memcmp(mfrc522.uid.uidByte, uids[i], 4) == 0)
    {
      byte cardAsByte = (byte)(i + 1);
      if (matchedCard == cardAsByte)
        return;
      matchedCard = cardAsByte;
      ssp.writeCommand(COMMAND_ID_CARD_SCAN);
      ssp.writeByte(matchedCard);
      ssp.writeEot();
    }
  }
}

void onReceivedLedColor()
{
  byte redValue = ssp.readByte();
  byte greenValue = ssp.readByte();
  byte blueValue = ssp.readByte();
  ssp.readEot();

  analogWrite(PIN_RED, redValue);
  analogWrite(PIN_GREEN, greenValue);
  analogWrite(PIN_BLUE, blueValue);
}

void onError(uint8_t errorNum)
{
}

void onPlayButton()
{
  ssp.writeCommand(COMMAND_ID_BUTTON_PRESS);
  ssp.writeByte(0x00);
  ssp.writeEot();
}

// void onPauseButton()
// {
//   ssp.writeCommand(COMMAND_ID_BUTTON_PRESS);
//   ssp.writeByte(0x01);
//   ssp.writeEot();
// }

void onStopButton()
{
  ssp.writeCommand(COMMAND_ID_BUTTON_PRESS);
  ssp.writeByte(0x02);
  ssp.writeEot();
}