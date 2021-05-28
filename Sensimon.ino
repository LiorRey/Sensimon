#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>

typedef struct
{
    char *type;
    int soundFreq;
    int lightColor;
} Command;

// CONSTANTS
#define numOfPossibleCommands 5
#define commandShowDuration 1
#define delayBetweenCommands 0.25

#define lightSensorCorrectRange 50
#define flashLightThreshold 700

#define blowSensorCorrectRange 20
#define blowSensorThreshold 100

#define temperatureSensorCorrectRange 4
#define temperatureThresholdOffset 8

// For Simon Animation state :)
#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_C5  523

// ENUM
enum StateEnum
{
    SIMON_ANIMATION,
    INIT_BOARD,
    BOARD_COMMANDS,
    PLAYER_TURN,
    SENSOR_RESET
};

// GLOBALS
int state = SIMON_ANIMATION;
Command gameCommands[84] = {};
Command possibleCommands[numOfPossibleCommands] =
{
        {"BLUE", 1000, 0x0000FF},        // Cold
        {"GREEN", 400, 0x00FF00},        // Blow
        {"YELLOW", 600, 0xFFFF00},       // Light
        {"PURPLE_RIGHT", 800, 0xFF8080}, // Right button
        {"PURPLE_LEFT", 1200, 0xFF8080}, // Left button
};

int currNumOfCommands = 0;
int currPlayerSequenceIdx = 0;
int LightSensorValueOnInit, blowSensorValueOnInit, temperatureSensorValueOnInit;
int temperatureThreshold;

// For Simon Animation state :)
int NOTES[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5 };
int led_indices[][2] {
  {0, 1},
  {3, 4},
  {5, 6},
  {8, 9}
};
int gameColors[] = {0, 50, 100, 150};

void setup()
{
    Serial.begin(9600);
    CircuitPlayground.begin();
    randomSeed(CircuitPlayground.lightSensor());
    LightSensorValueOnInit = CircuitPlayground.lightSensor();
    blowSensorValueOnInit = CircuitPlayground.mic.soundPressureLevel(10);
    temperatureSensorValueOnInit = CircuitPlayground.temperatureF();
    temperatureThreshold = temperatureSensorValueOnInit - temperatureThresholdOffset;
}

void loop()
{
    Serial.println(CircuitPlayground.temperatureF());
    switch (state)
    {
    case INIT_BOARD:
        state = initBoardState();
        break;

    case BOARD_COMMANDS:
        state = boardCommandsState();
        break;

    case PLAYER_TURN:
        state = playerTurnState();
        break;

    case SENSOR_RESET:
        state = sensorResetState();
        break;

    case SIMON_ANIMATION:
        state = simonAnimationState();
        break;
    }
}

int initBoardState()
{
    Serial.print("INIT_BOARD: adding next command");
    gameCommands[currNumOfCommands] = possibleCommands[random(0, numOfPossibleCommands)];
    currNumOfCommands++;

    return BOARD_COMMANDS;
}

int boardCommandsState()
{
    Serial.println("BOARD_COMMANDS: showing sequence");
    for (int i = 0; i < currNumOfCommands; i++)
    {
        showCommand(gameCommands[i]);
        delay(delayBetweenCommands * 1000);
    }

    return PLAYER_TURN;
}

void showCommand(Command command)
{
    bool showRightLights = true;
    bool showLeftLights = true;
    if (command.type == "PURPLE_LEFT")
    {
        showRightLights = false;
    }
    else if (command.type == "PURPLE_RIGHT")
    {
        showLeftLights = false;
    }

    // show lights
    for (int i = 0; i < 10; i++)
    {
        if (i <= 4 && showLeftLights || i > 4 && showRightLights)
        {
            CircuitPlayground.setPixelColor(i, command.lightColor);
        }
    }

    // make sound
    CircuitPlayground.playTone(command.soundFreq, commandShowDuration * 1000);

    CircuitPlayground.clearPixels();
}

int playerTurnState()
{
    delay(50);

    Command expectedCommand;
    if (currPlayerSequenceIdx == currNumOfCommands)
    {
        currPlayerSequenceIdx = 0;

        return INIT_BOARD;
    }

    int nextState = PLAYER_TURN;
    expectedCommand = gameCommands[currPlayerSequenceIdx];
    if (CircuitPlayground.leftButton())
    {
        nextState = validateDetectedCommandType("PURPLE_LEFT", expectedCommand);
    }

    else if (CircuitPlayground.rightButton())
    {
        nextState = validateDetectedCommandType("PURPLE_RIGHT", expectedCommand);
    }

    else if (CircuitPlayground.lightSensor() > flashLightThreshold)
    {
        nextState = validateDetectedCommandType("YELLOW", expectedCommand);
    }

    else if (CircuitPlayground.mic.soundPressureLevel(10) > blowSensorThreshold)
    {
        nextState = validateDetectedCommandType("GREEN", expectedCommand);
    }

    else if (CircuitPlayground.temperatureF() < temperatureThreshold)
    {
        nextState = validateDetectedCommandType("BLUE", expectedCommand);
    }

    return nextState;
}

int validateDetectedCommandType(char *typeOfCommandDetected, Command expectedCommand)
{
    if (strcmp(expectedCommand.type, typeOfCommandDetected) == 0)
    {
        currPlayerSequenceIdx++;
        showCommand(expectedCommand);
        return SENSOR_RESET;
    }
    else
    {
        return SIMON_ANIMATION;
    }
}

int sensorResetState()
{
    delay(100);

    bool buttonsReleased = !CircuitPlayground.leftButton() && !CircuitPlayground.rightButton();

    int lightSensorValue = CircuitPlayground.lightSensor();
    bool lightIsInCorrectRange = lightSensorValue <= LightSensorValueOnInit + lightSensorCorrectRange &&
                                 lightSensorValue >= LightSensorValueOnInit - lightSensorCorrectRange;

    int blowSensorValue = CircuitPlayground.mic.soundPressureLevel(10);
    bool blowIsInCorrectRange = blowSensorValue <= blowSensorValueOnInit + blowSensorCorrectRange &&
                                blowSensorValue >= blowSensorValueOnInit - blowSensorCorrectRange;

    int temperatureSensorValue = CircuitPlayground.temperatureF();
    bool temperatureIsInCorrectRange = temperatureSensorValue <= temperatureSensorValueOnInit + temperatureSensorCorrectRange &&
                                 temperatureSensorValue >= temperatureSensorValueOnInit - temperatureSensorCorrectRange;

    if (buttonsReleased && lightIsInCorrectRange && blowIsInCorrectRange && temperatureIsInCorrectRange)
    {
        return PLAYER_TURN;
    }
    else
    {
        return SENSOR_RESET;
    }
}

int simonAnimationState()
{
    CircuitPlayground.clearPixels();
    Serial.println("Condolences");
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 3; j >= 0; --j)
        {
            for (int l = 0; l < 2; ++l)
            {
                CircuitPlayground.setPixelColor(led_indices[j][l], CircuitPlayground.colorWheel(gameColors[j]));
            }
            
            CircuitPlayground.playTone(NOTES[j], (10 - i) * 10, false);
            for (int l = 0; l < 2; ++l)
            {
                CircuitPlayground.setPixelColor(led_indices[j][l], 0);
            }
        }
    }

    delay(1000);
    resetGameParameters();

    return INIT_BOARD;
}

void resetGameParameters()
{
    currNumOfCommands = 0;
    currPlayerSequenceIdx = 0;
}