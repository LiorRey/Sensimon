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
#define commandShowDuration 2

// ENUM
enum StateEnum
{
    INIT_BOARD,
    BOARD_COMMANDS,
    PLAYER_TURN,
    SENSOR_RESET,
    GAME_OVER
};
int state = INIT_BOARD;

// GLOBALS
Command gameCommands[84] = {};
Command possibleCommands[numOfPossibleCommands] =
    {
        {"RED", 0, 0},
        {"GREEN", 0, 0},
        {"BLUE", 0, 0},
        {"YELLOW", 0, 0},
        {"ORANGE_RIGHT", 0, 0},
        {"ORANGE_LEFT", 0, 0},
        {"PINK", 0, 0}
    };

bool isPlayerTurnState = false;
int currNumOfCommands = -1;
long randCommand;

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
}

void showCommand(Command command)
{
    // make sound
    CircuitPlayground.playTone(command.soundFreq, commandShowDuration * 1000);
    
    // show lights
    for (int i=0; i<10; i++)
    {
        CircuitPlayground.setPixelColor(i, command.lightColor);
    }
    delay(commandShowDuration * 1000);
    CircuitPlayground.clearPixels();
}

int playerTurnState()
{
}