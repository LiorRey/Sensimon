#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>

typedef struct
{
    char* type;
    int soundFreq;
    int lightColor;
} Command;

// CONSTANTS
#define numOfPossibleCommands 7
#define commandShowDuration 1
#define delayBetweenCommands 0.5

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
        {"RED", 0, 0},          // Hot
        {"GREEN", 0, 0},        // Blow
        {"BLUE", 0, 0},         // Cold
        {"YELLOW", 0, 0},       // Light
        {"ORANGE_RIGHT", 0, 0}, // Right button
        {"ORANGE_LEFT", 0, 0},  // Left button
        {"PINK", 0, 0}          // High note
    };

int currNumOfCommands = -1;
int currPlayerSequenceIdx = 0;

void setup()
{
    Serial.begin(9600);
    CircuitPlayground.begin();
}

void loop()
{
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
        break;

    case GAME_OVER:
        break;
    }
}

int initBoardState()
{
    Serial.println("");
    currNumOfCommands++;
    gameCommands[currNumOfCommands] = possibleCommands[random(0, numOfPossibleCommands)];
    return BOARD_COMMANDS;
}

int boardCommandsState()
{
    Command testCommands[] = 
    {
        {"RED", 700, 0xFF0000},
        {"GREEN", 1000, 0x00FF00},
        {"YELLOW", 1200, 0xFFFF00},
    };

    for (int i = 0; i < 3; i++)
    {
        showCommand(testCommands[i]);
        // showCommand(gameCommands[i]);
    }

    return PLAYER_TURN;
}

void showCommand(Command command)
{
    // show lights
    for (int i=0; i<10; i++)
    {
        CircuitPlayground.setPixelColor(i, command.lightColor);
    }

    // make sound
    CircuitPlayground.playTone(command.soundFreq, commandShowDuration * 1000);
    delay(delayBetweenCommands * 1000);
    CircuitPlayground.clearPixels();
}

int playerTurnState()
{
    Command expectedCommand;

    if (currPlayerSequenceIdx == currNumOfCommands)
    {
        currPlayerSequenceIdx = 0;

        return INIT_BOARD;
    }

    expectedCommand = gameCommands[currPlayerSequenceIdx];

    if (CircuitPlayground.leftButton())
    {
        if (expectedCommand.type == "ORANGE_LEFT")
        {
            currPlayerSequenceIdx++;
            
            return PLAYER_TURN;
        }
        else
        {
            return GAME_OVER;
        }
    }
}
