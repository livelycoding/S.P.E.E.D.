#define Light1 6
#define Light2 5
#define Button1 7
#define Button2 8
#define VictoryLedRed 9
#define VictoryLedBlue 10
#define VictoryLedGreen 11

//Debouncing: Keep track of last time a button was toggled so button inputs aren't sent repeatedly
unsigned long lastDebounceTimePlayerOne = 0;
unsigned long lastDebounceTimePlayerTwo = 0;

//How long before we read a new button state after the last one
const unsigned long debounceDelay = 40; 

//Keep track of current game state
int pressesToWin;
bool gameInProgress = false;
bool gameOver = false;
int winner;

//There is a startup and ending light sequence...
bool blinkSequenceActive = false;
int blinkState = HIGH;
unsigned long timeSinceLastBlink = 0;

//...And you can alter values here to change its duration.
int blinkRepetitions = 20;
int blinkRate = 50;
const int blinkSequenceSustain = 1500;

//Keep track of the state of each player's button and of each player's LED
int buttonStatePlayerOne;
int buttonStatePlayerTwo;
int lastButtonStatePlayerOne = LOW;
int lastButtonStatePlayerTwo = LOW;
int buttonPressesPlayerOne = 0;
int buttonPressesPlayerTwo = 0;

void setup() {
  // put your setup code here, to run once:

  pinMode(Light1, OUTPUT); //Player one's light
  pinMode(Button1, INPUT); //Player one's button

  pinMode(Light2, OUTPUT); //Player two's light
  pinMode(Button1, INPUT); //Player two's button

  pinMode(VictoryLedRed, OUTPUT); //Light for when the game is won.
  pinMode(VictoryLedBlue, OUTPUT);
  pinMode(VictoryLedGreen, OUTPUT);

  //Initialize light states
  analogWrite(Light1, LOW);
  analogWrite(Light2, LOW);
  analogWrite(VictoryLedRed, LOW);
  analogWrite(VictoryLedGreen, LOW);
  analogWrite(VictoryLedBlue, LOW);

  //Generate a random number of times
  //between 15 and 30 that both players
  //need to press the button to win the game.

  randomSeed(analogRead(0));
  pressesToWin = 15 + random(16);
  Serial.begin(9600);

  //Trigger start up light sequence
  startup();
  
  gameInProgress = true;
  gameOver = false;
}


void startup() {
  //flashes a 3, 2, 1, go!
  setVictoryColor(255, 0, 0); //red
  delay(2000);
  setVictoryColor(255, 255, 102); //yellow
  delay(2000);
  blinkSequenceActive = true;
}

void blinkSequenceStateChange(int red, int green, int blue)
{
    //Flip the light state if enough time since last blink has passed
    if (millis() - timeSinceLastBlink >= blinkRate)
    {
      blinkRepetitions--;
      timeSinceLastBlink = millis();
      if (blinkState == HIGH) {
        setVictoryColor(0,0,0);
        blinkState = LOW;
      }
      else {
        setVictoryColor(red,green,blue);
        blinkState = HIGH;
      }
    }
}

//Ends the startup phase by holding the light for several seconds.

void concludeBlinkLightSustain(int red, int green, int blue) {
    //turn off the light if the sustain time ran out
    if (millis() - timeSinceLastBlink >= blinkSequenceSustain)
    {
      setVictoryColor(0,0,0);
      blinkSequenceActive = false;
    }
    else {
      setVictoryColor(red,green,blue);
    }
  }

void setVictoryColor(int red, int green, int blue) {
  analogWrite(VictoryLedRed, red);
  analogWrite(VictoryLedGreen, green);
  analogWrite(VictoryLedBlue, blue);
}

void loop() {
  //game is currently being played and hasn't ended yet.
  if (gameInProgress == true && gameOver == false) {
    
    if (blinkSequenceActive) {
      if (blinkRepetitions > 0) {
        blinkSequenceStateChange(0,255,0);
      }
      else {
        concludeBlinkLightSustain(0,255,0);  
      }
    }

    //Handle input
    int readingP1 = digitalRead(Button1);
    int readingP2 = digitalRead(Button2);

    //update debouncing timers if the button's state has been altered
    if (readingP1 != lastButtonStatePlayerOne)
    {
      lastDebounceTimePlayerOne = millis();
    }
    if (readingP2 != lastButtonStatePlayerTwo)
    {
      lastDebounceTimePlayerTwo= millis();  
    }

    //If the below is true, the button state has been sustained and is an actual state
    if ((millis() - lastDebounceTimePlayerOne) > debounceDelay) {
      if (readingP1 != buttonStatePlayerOne) {
         buttonStatePlayerOne = readingP1;
      
        if (buttonStatePlayerOne == HIGH) {
          if (buttonPressesPlayerOne < pressesToWin)
            buttonPressesPlayerOne++;
          else
            buttonPressesPlayerOne=0;
        }
      }
    }

    //Repeat for player two
    if ((millis() - lastDebounceTimePlayerTwo) > debounceDelay) {
      if (readingP2 != buttonStatePlayerTwo) {
         buttonStatePlayerTwo = readingP2;
      
        if (buttonStatePlayerTwo == HIGH) {
          if (buttonPressesPlayerTwo < pressesToWin)
            buttonPressesPlayerTwo++;
          else
            buttonPressesPlayerTwo=0;
        }
      }
    }

    //check for a winner by player 1: if you don't press for one second on the win press, you won.
    if ((millis() - lastDebounceTimePlayerOne) > 1000) {
      if (buttonStatePlayerOne == LOW && buttonPressesPlayerOne == pressesToWin) {
        winner = 1;
        gameOver = true;
      }
    }
    //Repeat win check for player two
    if ((millis() - lastDebounceTimePlayerTwo) > 1000) {
      if (buttonStatePlayerTwo == LOW && buttonPressesPlayerTwo == pressesToWin) {
        winner = 2;
        gameOver = true;
      }
    }

    //set each player's LED to match the their press count
    analogWrite(Light1, floor(255*(1.0*buttonPressesPlayerOne/pressesToWin)));
    analogWrite(Light2, floor(255*(1.0*buttonPressesPlayerTwo/pressesToWin)));
    
    //Save previous button presses for next loop.
    lastButtonStatePlayerOne = readingP1;
    lastButtonStatePlayerTwo = readingP2;
  }
  //game is over for the first time
  else if (gameOver == true && gameInProgress == true) {
    gameInProgress = false;

    //We're going to begin another blink sequence
    timeSinceLastBlink = 0;
    blinkState = LOW;
    blinkRepetitions = 50;
    
    if (winner == 1) {
        analogWrite(Light1, 255);
        analogWrite(Light2, 0);
    }
    if (winner == 2) {
        analogWrite(Light1, 0);
        analogWrite(Light2, 255);
    }
  }
  //Game is over continuing state. Flash the winner's color, then sustain it.
  else if (gameOver == true) {
    if (blinkRepetitions > 0) {
        if (winner == 1) {
          blinkSequenceStateChange(0,0,255);
        }
        else {
          blinkSequenceStateChange(255,0,0);
        }
      }
      else {
        if (winner == 1)
          setVictoryColor(0,0,255);
        else
          setVictoryColor(255,0,0);  
      }
      
    if (winner == 1) {
        analogWrite(Light1, 255);
        analogWrite(Light2, 0);
    }
    else if (winner == 2) {
        analogWrite(Light1, 0);
        analogWrite(Light2, 255);
    }
  }
  //game is glitched because this state should not be achieved- make all lights be on.
  else {
    analogWrite(Light1, 255);
    analogWrite(Light2, 255);
    setVictoryColor(255,255,255);
  }
}
