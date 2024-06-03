#include <Arduino.h>
#include <ESP32Encoder.h>
#include <Keypad.h>     // https://github.com/Chris--A/Keypad
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad

BleGamepad bleGamepad("BoutonBox", "Simon Bourgain", 100); // Shows how you can customise the device name, manufacturer name and initial battery level

#define ENCODER_PRESS_TIME 100
#define ENC_SEND_BUTTON 1
#define ENC_SEND_AXIS 1

ESP32Encoder encoder1;
ESP32Encoder encoder2;

int32_t encoder1Count = 0;
int32_t encoder2Count = 0;

#define numOfButtons 26

byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];
byte buttonPins[numOfButtons] = {33, 32};
byte physicalButtons[numOfButtons] = {22, 21};

#define ROWS 5
#define COLS 4
uint8_t rowPins[ROWS] = {23, 22, 1, 3, 21};
uint8_t colPins[COLS] = {19, 18, 5, 17};
uint8_t keymap[ROWS][COLS] = 
{
  {5, 4, 3, 2},
  {9, 8, 7, 6},
  {16, 14,12,10},
  {17,15,13,11},
  {20,19,18,1}
};

Keypad customKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

void setup()
{
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    for (byte currentPinIndex = 0; currentPinIndex < numOfButtons; currentPinIndex++)
    {
        pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
        previousButtonStates[currentPinIndex] = HIGH;
        currentButtonStates[currentPinIndex] = HIGH;
    }

    // Attach pins to encoders
    encoder1.attachHalfQuad(13, 12);
    encoder2.attachHalfQuad(14, 27);


    // Clear encoder counts
    encoder1.clearCount();
    encoder2.clearCount();


    // Initialise encoder counts
    encoder1Count = encoder1.getCount();
    encoder2Count = encoder2.getCount();
    
    log_i("loop...");
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    bleGamepadConfig.setButtonCount(numOfButtons);
    bleGamepadConfig.setHatSwitchCount(0);

    bleGamepadConfig.setWhichAxes(false, false, false, false, false, false, false, false);
    bleGamepadConfig.setAutoReport(false);

    bleGamepad.begin(&bleGamepadConfig);
}

void EncodersUpdate()
{
  int32_t tempEncoderCount1 = encoder1.getCount();
  int32_t tempEncoderCount2 = encoder2.getCount();

  
  bool sendReport = false;
  
  if(tempEncoderCount1 != encoder1Count) 
  {
    sendReport = true;
    
    if(tempEncoderCount1 > encoder1Count)
    {
      bleGamepad.press(BUTTON_26);
    }
    else
    {
      bleGamepad.press(BUTTON_25);
    } 
  }
  
  if(tempEncoderCount2 != encoder2Count) 
  {
    sendReport = true;
    
    if(tempEncoderCount2 > encoder2Count)
    {
      bleGamepad.press(BUTTON_24);
    }
    else
    {
      bleGamepad.press(BUTTON_23);
    }
  }
  
  
  if (sendReport)
  {
    bleGamepad.sendReport();
    delay(100);

    bleGamepad.release(BUTTON_24);
    bleGamepad.release(BUTTON_23);
    bleGamepad.release(BUTTON_26);
    bleGamepad.release(BUTTON_25);
    bleGamepad.sendReport();
    delay(100);

    encoder1Count = encoder1.getCount();
    encoder2Count = encoder2.getCount();
  } 
}

void KeypadUpdate()
{
    customKeypad.getKeys();

    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.      //LIST_MAX is provided by the Keypad library and gives the number of buttons of the Keypad instance
    {
        if (customKeypad.key[i].stateChanged) // Only find keys that have changed state.
        {
            uint8_t keystate = customKeypad.key[i].kstate;

            if (bleGamepad.isConnected())
            {
                if (keystate == PRESSED)
                {
                    bleGamepad.press(customKeypad.key[i].kchar);
                } // Press or release button based on the current state
                if (keystate == RELEASED)
                {
                    bleGamepad.release(customKeypad.key[i].kchar);
                }

                bleGamepad.sendReport(); // Send the HID report after values for all button states are updated, and at least one button state had changed
            }
        }
    }
}

void loop() 
{
   if(bleGamepad.isConnected()) 
    {
        for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++)
        {
            currentButtonStates[currentIndex] = digitalRead(buttonPins[currentIndex]);

            if (currentButtonStates[currentIndex] != previousButtonStates[currentIndex])
            {
                if (currentButtonStates[currentIndex] == LOW)
                {
                    bleGamepad.press(physicalButtons[currentIndex]);
                }
                else
                {
                    bleGamepad.release(physicalButtons[currentIndex]);
                }
            }
        }

        if (currentButtonStates != previousButtonStates)
        {
            for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++)
            {
                previousButtonStates[currentIndex] = currentButtonStates[currentIndex];
            }

            bleGamepad.sendReport();
        }
      KeypadUpdate();
      EncodersUpdate();
      delay(10);
    }
}
