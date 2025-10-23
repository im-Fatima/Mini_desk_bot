#include <Arduino.h>
#include <U8g2lib.h>

// Change this to match your OLED model and connection.
// This example uses a common 128x64 SSD1306 OLED with hardware I2C.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Set some constants for face parts to make the code easier to read and modify
const int face_center_x = 64;
const int face_center_y = 32;
const int eye_spacing = 20;
const int eye_y_offset = -8;
const int mouth_y_offset = 12;

// Pin Definitions for the new components
const int IR_SENSOR_PIN = 0; // GPIO 0 for IR sensor
const int LED_PIN = 1;       // GPIO 1 for LED
const int BUZZER_PIN = 2;    // GPIO 2 for buzzer

// PWM settings for the buzzer
const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 8; // 8-bit resolution (0-255)

// State variables for sensor logic
unsigned long lastTouchTime = 0;
unsigned long touchStartTime = 0;
unsigned int touchCount = 0;
bool isTouched = false;
bool wasTouched = false;
bool soundPlaying = false;
unsigned long soundStartTime = 0;

// Enum to define the emotional states
enum EmotionState {
  NORMAL,
  SMILE,
  ANGRY,
  SAD,
  LAUGH,
  LOVE
};

EmotionState currentState = NORMAL;

// --- Function Prototypes for each emotion state and sounds ---
void drawNormal();
void drawSmile();
void drawAngry();
void drawSad();
void drawLaugh();
void drawLove();

void playSmileSound();
void playAngrySound();
void playSadSound();
void playLaughSound();
void playLoveSound();
void stopBuzzer();

// --- Setup Function ---
void setup() {
  u8g2.begin();
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Configure PWM for the buzzer using the new API
  ledcAttach(BUZZER_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
}

// --- Face Drawing Functions ---

// Draws a standard face with simple circular eyes
void drawNormal() {
  // Eyes: two large rounded rectangles
  u8g2.drawRBox(face_center_x - eye_spacing - 5, face_center_y + eye_y_offset, 10, 20, 3); // Left Eye
  u8g2.drawRBox(face_center_x + eye_spacing - 5, face_center_y + eye_y_offset, 10, 20, 3); // Right Eye
  // Mouth: a slightly curved line
  u8g2.drawDisc(face_center_x, face_center_y + mouth_y_offset, 4, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT); // Mouth
}

// Draws a smiling face with a curved mouth
void drawSmile() {
  // Eyes: two large rounded rectangles
  u8g2.drawRBox(face_center_x - eye_spacing - 5, face_center_y + eye_y_offset, 10, 20, 3); // Left Eye
  u8g2.drawRBox(face_center_x + eye_spacing - 5, face_center_y + eye_y_offset, 10, 20, 3); // Right Eye
  // Mouth: A half-circle using lower quadrants
  u8g2.drawDisc(face_center_x, face_center_y + mouth_y_offset, 10, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT); // Mouth
}

// Draws an angry face with slanted eyebrows
void drawAngry() {
  // Eyes: smaller rounded rectangles
  u8g2.drawRBox(face_center_x - eye_spacing - 4, face_center_y + eye_y_offset, 8, 16, 2); // Left Eye
  u8g2.drawRBox(face_center_x + eye_spacing - 4, face_center_y + eye_y_offset, 8, 16, 2); // Right Eye
  // Eyebrows: slanted lines
  u8g2.drawLine(face_center_x - eye_spacing - 5, face_center_y + eye_y_offset - 5, face_center_x - eye_spacing + 5, face_center_y + eye_y_offset); // Left Brow
  u8g2.drawLine(face_center_x + eye_spacing + 5, face_center_y + eye_y_offset - 5, face_center_x + eye_spacing - 5, face_center_y + eye_y_offset); // Right Brow
  // Mouth: a straight line
  u8g2.drawHLine(face_center_x - 10, face_center_y + mouth_y_offset, 20); // Mouth
}

// Draws a sad face with a downward-curved mouth and a teardrop
void drawSad() {
  // Eyes: large rounded rectangles
  u8g2.drawRBox(face_center_x - eye_spacing - 5, face_center_y + eye_y_offset, 10, 20, 3); // Left Eye
  u8g2.drawRBox(face_center_x + eye_spacing - 5, face_center_y + eye_y_offset, 10, 20, 3); // Right Eye
  // Mouth: a half-circle using upper quadrants
  u8g2.drawDisc(face_center_x, face_center_y + mouth_y_offset + 5, 10, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT); // Mouth
}

// Draws a laughing face with wide-open eyes and mouth
void drawLaugh() {
  // Left eye: The > shape
  u8g2.drawLine(face_center_x - eye_spacing - 2, face_center_y + eye_y_offset, face_center_x - eye_spacing + 8, face_center_y + eye_y_offset + 10);
  u8g2.drawLine(face_center_x - eye_spacing - 2, face_center_y + eye_y_offset + 20, face_center_x - eye_spacing + 8, face_center_y + eye_y_offset + 10);
  
  // Right eye: The < shape
  u8g2.drawLine(face_center_x + eye_spacing + 2, face_center_y + eye_y_offset, face_center_x + eye_spacing - 8, face_center_y + eye_y_offset + 10);
  u8g2.drawLine(face_center_x + eye_spacing + 2, face_center_y + eye_y_offset + 20, face_center_x + eye_spacing - 8, face_center_y + eye_y_offset + 10);
  
  
  // Mouth: a large half-circle for the wide-open mouth
  u8g2.drawDisc(face_center_x, face_center_y + mouth_y_offset, 12, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT); // Mouth
  // Tongue
  u8g2.drawDisc(face_center_x, face_center_y + mouth_y_offset + 5, 5, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
}

// Draws a face with heart-shaped eyes
void drawLove() {
   // Left Heart
  u8g2.drawDisc(face_center_x - eye_spacing - 4, face_center_y + eye_y_offset - 3, 4, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawDisc(face_center_x - eye_spacing + 4, face_center_y + eye_y_offset - 3, 4, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawLine(face_center_x - eye_spacing - 8, face_center_y + eye_y_offset - 3, face_center_x - eye_spacing, face_center_y + eye_y_offset + 7);
  u8g2.drawLine(face_center_x - eye_spacing, face_center_y + eye_y_offset + 7, face_center_x - eye_spacing + 8, face_center_y + eye_y_offset - 3);

  // Right Heart
  u8g2.drawDisc(face_center_x + eye_spacing - 4, face_center_y + eye_y_offset - 3, 4, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawDisc(face_center_x + eye_spacing + 4, face_center_y + eye_y_offset - 3, 4, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  u8g2.drawLine(face_center_x + eye_spacing - 8, face_center_y + eye_y_offset - 3, face_center_x + eye_spacing, face_center_y + eye_y_offset + 7);
  u8g2.drawLine(face_center_x + eye_spacing, face_center_y + eye_y_offset + 7, face_center_x + eye_spacing + 8, face_center_y + eye_y_offset - 3);

  // Mouth: A simple smiling arc
  u8g2.drawDisc(face_center_x, face_center_y + mouth_y_offset, 10, U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
}

// --- Sound Functions ---

void playSmileSound() {
  // A playful, rising arpeggio
  ledcWriteTone(BUZZER_PIN, 880); // A5
  delay(50);
  ledcWriteTone(BUZZER_PIN, 1047); // C6
  delay(50);
  ledcWriteTone(BUZZER_PIN, 1319); // E6
  delay(50);
  stopBuzzer();
}

void playAngrySound() {
  // A rapid, squeaky "frustrated" sound
  ledcWriteTone(BUZZER_PIN, 1500);
  delay(50);
  ledcWriteTone(BUZZER_PIN, 1400);
  delay(50);
  ledcWriteTone(BUZZER_PIN, 1500);
  delay(50);
  stopBuzzer();
}

void playSadSound() {
  // A "whining" or whimpering sound
  ledcWriteTone(BUZZER_PIN, 600);
  delay(150);
  stopBuzzer();
  delay(50);
  ledcWriteTone(BUZZER_PIN, 500);
  delay(200);
  stopBuzzer();
}

void playLaughSound() {
  // A series of joyful, varied chirps
  ledcWriteTone(BUZZER_PIN, 1000);
  delay(75);
  stopBuzzer();
  delay(25);
  ledcWriteTone(BUZZER_PIN, 1200);
  delay(75);
  stopBuzzer();
  delay(25);
  ledcWriteTone(BUZZER_PIN, 1400);
  delay(75);
  stopBuzzer();
}

void playLoveSound() {
  // A gentle, sweet melody
  ledcWriteTone(BUZZER_PIN, 988); // B5
  delay(100);
  ledcWriteTone(BUZZER_PIN, 1175); // D6
  delay(150);
  stopBuzzer();
  delay(50);
  ledcWriteTone(BUZZER_PIN, 988);
  delay(100);
  stopBuzzer();
}

void stopBuzzer() {
  ledcWrite(BUZZER_PIN, 0);
}

// --- Main Loop Function ---
void loop() {
  u8g2.clearBuffer();
  EmotionState previousState = currentState;
  isTouched = !digitalRead(IR_SENSOR_PIN);

  // Check for a new touch
  if (isTouched && !wasTouched) {
    // A new touch has just started
    digitalWrite(LED_PIN, HIGH);
    touchStartTime = millis();
    lastTouchTime = millis();
    touchCount++;
    // Reset count if it's been too long since the last touch
    if (millis() - lastTouchTime > 500) {
      touchCount = 1;
    }
    wasTouched = true;
  }
  // Check if touch has ended
  if (!isTouched && wasTouched) {
    digitalWrite(LED_PIN, LOW);
    stopBuzzer();
    soundPlaying = false;
    wasTouched = false;
  }

  // Logic to determine the expression
  if (isTouched) {
    unsigned long touchDuration = millis() - touchStartTime;
    if (touchDuration >= 10000) {
      currentState = ANGRY;
    } else if (touchDuration >= 7000) {
      currentState = SAD;
    } else if (touchDuration >= 3000) {
      currentState = LOVE;
    } else if (touchCount == 2) {
      currentState = SMILE;
    } else if (touchCount >= 3) {
      currentState = LAUGH;
    }
  } else {
    // No touch, reset and go to normal
    currentState = NORMAL;
    if (millis() - lastTouchTime > 500) {
      touchCount = 0;
    }
  }

  // Play sound effect if the state has changed
  if (currentState != previousState && currentState != NORMAL) {
    switch (currentState) {
      case SMILE:
        playSmileSound();
        break;
      case ANGRY:
        playAngrySound();
        break;
      case SAD:
        playSadSound();
        break;
      case LAUGH:
        playLaughSound();
        break;
      case LOVE:
        playLoveSound();
        break;
      default:
        break;
    }
  }
  
  // Draw the face based on the current state
  switch (currentState) {
    case NORMAL:
      drawNormal();
      break;
    case SMILE:
      drawSmile();
      break;
    case ANGRY:
      drawAngry();
      break;
    case SAD:
      drawSad();
      break;
    case LAUGH:
      drawLaugh();
      break;
    case LOVE:
      drawLove();
      break;
  }
  
  u8g2.sendBuffer();
  // Small delay to prevent display flickering
  delay(10);
}
