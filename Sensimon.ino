#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>

// ENUM
enum StateEnum {INIT_BOARD, BOARD_COMMANDS, PLAYER_TURN, SENSOR_RESET, GAME_OVER};
uint8_t state = INIT_BOARD;
enum ColorEnum {RED = 1, GREEN, BLUE, YELLOW, ORANGE_RIGHT, ORANGE_LEFT, PINK};

// GLOBALS
bool isPlayerTurnState = false;
int commands[84] = {}; // 0 value - end of commands
int currNumOfCommands = 0;
int commandColors[] = {RED, GREEN, BLUE, YELLOW, ORANGE_RIGHT, ORANGE_LEFT, PINK};
long randCommand;

void setup()
{
  Serial.begin(9600);
  CircuitPlayground.begin();
}

void loop()
{
    Serial.println("*");
    Serial.println(random(1,7));

    /*
    switch (state)
    {
        case INIT_BOARD:
            initBoard();
        break;
        
        case BOARD_COMMANDS:
        break;
        
        case PLAYER_TURN:
        break;
        
        case SENSOR_RESET:
        break;

        case GAME_OVER:
        break;
    }
    */
}

void initBoard()
{
    currNumOfCommands++;    
}

void boardTurnState()
{

}

void playerTurnState()
{

}