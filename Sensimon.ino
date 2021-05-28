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
#define delayBetweenCommands 0.5
#define lightSensorCorrectRange 50
#define flashLightThreshold 900

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
        {"GREEN", 0, 0},                 // Blow
        {"BLUE", 0, 0},                  // Cold
        {"YELLOW", 600, 0xFFFF00},       // Light
        {"PURPLE_RIGHT", 800, 0xFF8080}, // Right button
        {"PURPLE_LEFT", 1200, 0xFF8080}, // Left button
        {"PINK", 0, 0}                   // High note
};

int currNumOfCommands = 0;
int currPlayerSequenceIdx = 0;
int LightSensorValueOnInit;

void setup()
{
    Serial.begin(9600);
    CircuitPlayground.begin();
    randomSeed(analogRead(0));

    LightSensorValueOnInit = CircuitPlayground.lightSensor();
}

void loop()
{
    // delay(500);
    // Serial.println(CircuitPlayground.lightSensor());
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
        break;
    }
}

int initBoardState()
{
    Serial.print("INIT_BOARD");
    gameCommands[currNumOfCommands] = possibleCommands[random(3, 6)];
    currNumOfCommands++;
    // gameCommands[currNumOfCommands] = possibleCommands[random(0, numOfPossibleCommands)];
    return BOARD_COMMANDS;
}

int boardCommandsState()
{

    Serial.println("commands: ");

    for (int i = 0; i < currNumOfCommands; i++)
    {
        Serial.println(gameCommands[i].type);
        showCommand(gameCommands[i]);
    }

    Serial.println();

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
    delay(delayBetweenCommands * 1000);
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
        nextState = checkNextPlayerState(expectedCommand, "PURPLE_LEFT");
    }

    else if (CircuitPlayground.rightButton())
    {
        nextState = checkNextPlayerState(expectedCommand, "PURPLE_RIGHT");
    }

    else if (CircuitPlayground.lightSensor() > flashLightThreshold)
    {
        nextState = checkNextPlayerState(expectedCommand, "YELLOW");
    }

    return nextState;
}

int checkNextPlayerState(Command expectedCommand, char *name)
{
    if (strcmp(expectedCommand.type, name) == 0)
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
    delay(500);

    bool buttonsReleased = !CircuitPlayground.leftButton() && !CircuitPlayground.rightButton();

    int lightSensorValue = CircuitPlayground.lightSensor();
    bool lightIsInCorrectRange = lightSensorValue <= LightSensorValueOnInit + lightSensorCorrectRange &&
                                 lightSensorValue >= LightSensorValueOnInit - lightSensorCorrectRange;

    if (buttonsReleased && lightIsInCorrectRange)
    {
        return PLAYER_TURN;
    }
    else
    {
        return SENSOR_RESET;
    }
}