#include <Adafruit_CircuitPlayground.h>
#include <AsyncDelay.h>

// Threshold the mood ring should beep at you
const int LIGHT_THRESHOLD = 100;
const int ACCEL_THRESHOLD = 20;

// Variable for score
volatile int score = 0;
/*
0: Startup - Displays lights and starting tune
1: Game Start/Reset - Resets all variables
2: Round Start - picks random game and says instructions
3: Detect - Waits for user input or timeout
4: Fail (nonhighscore) - Says earned score and current highscore
5: Fail (highscore) - Variation where you beat the highscore
6: Game End - Waits for user input to transition to Reset state


*/

// Variable for correct trigger
volatile bool trigger = false;

// Flags for debounce
volatile bool switchFlag = false;
volatile bool leftFlag = false;
volatile bool rightFlag = false;

// Game state variable
int state = 0;

// Game variation variable
int game = 0;
/*
0: Button press
1: Switch input
2: Accel threshold
3: Light input
*/
int light = 0;
int accel = 0;
// Timer
AsyncDelay inputTimeout;

// Highscore
int highscore = 0;

void setup() {
  //Timer and Board setup
  CircuitPlayground.begin();

  // Randomize seed
  randomSeed(analogRead(A0));

  // interrupt for switch
  attachInterrupt(digitalPinToInterrupt(7), switchInterrupt, CHANGE);  //switch

  // interrupt for left button
  attachInterrupt(digitalPinToInterrupt(4), leftInterrupt, FALLING);  //left button

  // interrupt for right button
  attachInterrupt(digitalPinToInterrupt(5), rightInterrupt, FALLING);  //right button

  inputTimeout.start(5000, AsyncDelay::MILLIS);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case 0:
      // Intro sequence and sound
      Serial.println("case 0");
      //Play lights and sound

      state = 1;
      break;
    case 1:
      // Game reset/setup

      Serial.println("case 1");
      score = 0;
      game = 0;
      trigger = false;

      // Flags
      switchFlag = true;
      rightFlag = true;
      leftFlag = true;

      CircuitPlayground.clearPixels();
      state = 2;
      break;
    case 2:
      // Round setup
      Serial.println(score);
      // Flags
      switchFlag = true;
      rightFlag = true;
      leftFlag = true;

      delay(1000);
      
      // Pick random game
      game = random(0, 4);

      switch (game) {
        case 0:
          Serial.println("Press It!");
          rightFlag = false;
          leftFlag = false;
          break;
        case 1:
          Serial.println("Switch It!");
          switchFlag = false;
          break;
        case 2:
          Serial.println("Cover It!");
          break;
        case 3:
          Serial.println("Shake It!");
          break;
      }
      state = 3;

      inputTimeout.restart();
      break;
    case 3:
      // Detect input/timeout
      if(rightFlag){
        delay(25);
        rightFlag = false;
      }
      if(leftFlag){
        delay(25);
        leftFlag = false;
      }
      if(switchFlag){
        delay(25);
        switchFlag = false;
      }

      // Check the timer to see if it expired
      if (inputTimeout.isExpired()) {
        Serial.println("timeout");
        if (score > highscore) {
          state = 5;
        } else {
          state = 4;
        }
      }
      // If timer is not expired, then check for inputs
      switch (game) {
        case 0:
          // Button input, waits for trigger to switch to true
          if (trigger) {
            score++;
            trigger = false;
            state = 2;
          }
          break;
        case 1:
          // Switch input, waits for trigger to switch to true
          if (trigger) {
            score++;
            trigger = false;
            state = 2;
          }
          break;
        case 2:
          // Light input, waits for recorded light to drop BELOW threshold
          light = CircuitPlayground.lightSensor();
          light = constrain(light, 0, 1000);

          if (light < LIGHT_THRESHOLD) {
            state = 2;
            score++;
          }
          break;
        case 3:
          // Accelerometer input, waits for acceleration to rise ABOVE threshold
          accel = 0;
          accel += CircuitPlayground.motionX();
          accel += CircuitPlayground.motionY();
          accel += CircuitPlayground.motionZ();

          if (accel > ACCEL_THRESHOLD) {
            state = 2;
            score++;
          }
          break;
      }

      break;
    case 4:
      // Fail state
      Serial.println("case 4");

      Serial.print("Score: ");
      Serial.println(score);

      delay(1000);
      state = 6;
      Serial.println("case 6");
      rightFlag = false;
      leftFlag = false;
      break;

    case 5:

      Serial.println("case 5");
      highscore = score;

      Serial.print("New Highscore! ");
      Serial.println(score);

      delay(1000);
      state = 6;
      Serial.println("case 6");
      rightFlag = false;
      leftFlag = false;
      break;

    case 6:


      // Wait for user input for next game
      if (rightFlag || leftFlag) {
        state = 1;
      }
      rainbowCycle(50, 10);
      break;
  }
}

void switchInterrupt() {
  if (!switchFlag) {
    switchFlag = true;
    if (game == 1) {
      trigger = true;
    }
  }
}
void rightInterrupt() {
  if (!rightFlag) {
    rightFlag = true;
    if (game == 0) {
      trigger = true;
    }
  }
}

void leftInterrupt() {
  if (!leftFlag) {
    leftFlag = true;
    if (game == 0) {
      trigger = true;
    }
  }
}


void rainbowCycle(int currentSpeed, int stripLen) {
  // Rainbow cycle from Kaleidoscope project

  // Make an offset based on the current millisecond count scaled by the current speed.

  uint32_t offset = millis() / currentSpeed;

  // Loop through each pixel and set it to an incremental color wheel value.

  for (int i = 0; i < 10; ++i) {

    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / stripLen) + offset) & 255));
  }
}
