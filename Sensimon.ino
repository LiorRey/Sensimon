/*
 * Lights Game Challenge: Sensimon!
 * 
 * We created Sensimon - a sensored version of one of the most iconic electronic games of the 70's and 80's, Simon!
 * 
 * To beat the Sensimon game, the player needs to remember the sounds and LED lights commands (which are described below),
 * and repeat them by triggering the different sensors of the circuit in the correct order.
 * It starts off at a nice steady pace, but the more you play, the more complicated the sequence of commands become,
 * building suspense with each turn.
 * 
 * The Sensimon commands:
 * - 10 Blue LEDs and 1000Hz tone : The player should trigger the temprature sensor (A9) with a cold object (like cold finger after touching ice).
 * - 10 Green LEDs and 400Hz tone : The player should trigger the sound sensor (microphone) with a blowing.
 * - 10 Yellow LEDs and 600Hz tone : The player should trigger the light sensor (A8) with a light-emitting object (like flashlight).
 * - 5 Left Purple LEDs and 800Hz tone : The player should press the left button A (D4).
 * - 5 Right Purple LEDs and 1200Hz tone : The player should press the right button B (D5).
 *
 * Video links (turn on the CAPTIONS of the videos!) :
 * - IoT: Lights Game Challenge - Sensimon! | Part 1
 *   https://www.youtube.com/watch?v=UU1b3NPOIrk
 * - IoT: Lights Game Challenge - Sensimon! | Part 2
 *   https://www.youtube.com/watch?v=WQ1iN_uXcUw
 * 
 * Created by :
 * Tomer Ben-Gigi, 206198772
 * Lior Reytan, 204326607
 */

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

// For Sensimon Animation state :)
#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_C5  523

// ENUM
enum StateEnum
{
    SENSIMON_ANIMATION,
    INIT_BOARD,
    BOARD_COMMANDS,
    PLAYER_TURN,
    SENSOR_RESET
};

// GLOBALS
int state = SENSIMON_ANIMATION;
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

// For Sensimon Animation state :)
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

    case SENSIMON_ANIMATION:
        state = sensimonAnimationState();
        break;
    }
}

// Handles the addition of a new randomly-chosen command to the current game seqeunce
int initBoardState()
{
    Serial.print("INIT_BOARD: adding next command");
    gameCommands[currNumOfCommands] = possibleCommands[random(0, numOfPossibleCommands)];
    currNumOfCommands++;

    return BOARD_COMMANDS;
}

// Displays the current expected sequence of commands for the player to repeat
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

// Displays a single expected command for the player to repeat, with lights and sounds
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

    // Display lights
    for (int i = 0; i < 10; i++)
    {
        if (i <= 4 && showLeftLights || i > 4 && showRightLights)
        {
            CircuitPlayground.setPixelColor(i, command.lightColor);
        }
    }

    // Play sound
    CircuitPlayground.playTone(command.soundFreq, commandShowDuration * 1000);

    CircuitPlayground.clearPixels();
}

// Handles the turn of the player and checks what sensor he/she has triggered with his/her current move
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
        nextState = checkPlayerMove("PURPLE_LEFT", expectedCommand);
    }

    else if (CircuitPlayground.rightButton())
    {
        nextState = checkPlayerMove("PURPLE_RIGHT", expectedCommand);
    }

    else if (CircuitPlayground.lightSensor() > flashLightThreshold)
    {
        nextState = checkPlayerMove("YELLOW", expectedCommand);
    }

    else if (CircuitPlayground.mic.soundPressureLevel(10) > blowSensorThreshold)
    {
        nextState = checkPlayerMove("GREEN", expectedCommand);
    }

    else if (CircuitPlayground.temperatureF() < temperatureThreshold)
    {
        nextState = checkPlayerMove("BLUE", expectedCommand);
    }

    return nextState;
}

// Checks if the player made the correct move (detected command) relative to the expected command.
// If so - the game sequence continues. Else - Sensimon's Game Over animation is displayed and a new game starts.
int checkPlayerMove(char *typeOfCommandDetected, Command expectedCommand)
{
    if (strcmp(expectedCommand.type, typeOfCommandDetected) == 0)
    {
        currPlayerSequenceIdx++;
        showCommand(expectedCommand);

        return SENSOR_RESET;
    }
    else
    {
        return SENSIMON_ANIMATION;
    }
}

// Makes Sensimon wait for all the sensors to get back to a steady state/range
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

// Displays that spectacular Sensimon animation and starts a new game
int sensimonAnimationState()
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

// Restarts the current session for a new game
void resetGameParameters()
{
    currNumOfCommands = 0;
    currPlayerSequenceIdx = 0;
}