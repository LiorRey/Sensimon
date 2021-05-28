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
#define numOfPossibleCommands 7
#define commandShowDuration 1
#define delayBetweenCommands 0.25

#define lightSensorCorrectRange 50
#define flashLightThreshold 800

#define blowSensorCorrectRange 20
#define blowSensorThreshold 100

// ENUM
enum StateEnum
{
    INIT_BOARD,
    BOARD_COMMANDS,
    PLAYER_TURN,
    SENSOR_RESET,
    GAME_OVER
};

// GLOBALS
int state = INIT_BOARD;
Command gameCommands[84] = {};
Command possibleCommands[numOfPossibleCommands] =
    {
        {"RED", 0, 0},                   // Hot
        {"BLUE", 0, 0},                  // Cold
        {"GREEN", 400, 0x00FF00},        // Blow
        {"YELLOW", 600, 0xFFFF00},       // Light
        {"PURPLE_RIGHT", 800, 0xFF8080}, // Right button
        {"PURPLE_LEFT", 1200, 0xFF8080}, // Left button
        {"PINK", 0, 0}                   // High note
};

int currNumOfCommands = 0;
int currPlayerSequenceIdx = 0;
int LightSensorValueOnInit, blowSensorValueOnInit;

void setup()
{
    Serial.begin(9600);
    CircuitPlayground.begin();
    randomSeed(CircuitPlayground.lightSensor());
    LightSensorValueOnInit = CircuitPlayground.lightSensor();
    blowSensorValueOnInit = CircuitPlayground.mic.soundPressureLevel(10);
}

void loop()
{

    // delay(500);
    // Serial.println(CircuitPlayground.mic.soundPressureLevel(10));
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

    case GAME_OVER:
        state = gameOverState();
        break;
    }
}

int initBoardState()
{
    Serial.print("INIT_BOARD: adding next command");
    gameCommands[currNumOfCommands] = possibleCommands[random(2, 6)];
    currNumOfCommands++;
    // gameCommands[currNumOfCommands] = possibleCommands[random(0, numOfPossibleCommands)];
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
        return GAME_OVER;
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

    if (buttonsReleased && lightIsInCorrectRange && blowIsInCorrectRange)
    {
        return PLAYER_TURN;
    }
    else
    {
        return SENSOR_RESET;
    }
}

int gameOverState()
{
    CircuitPlayground.clearPixels();
    for (int i = 0; i < 10; i++)
    {
        CircuitPlayground.setPixelColor(i, 0xFF0000);
    }

    return GAME_OVER;
}