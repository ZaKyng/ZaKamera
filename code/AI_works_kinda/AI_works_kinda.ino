#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

U8G2_SH1107_PIMORONI_128X128_1_HW_I2C u8g2(U8G2_R0);

uint8_t delay_time = 50;

// Buttons
bool buttons[5] = {false}; 
bool last_btns[4] = {false};
const uint8_t btn_pins[4] = {9, 8, 7, 6};

// UI State & Errors
enum UIState { HOME, SHOOT, CALIB };
enum ErrorCode { ERR_NONE, ERR_NO_SD, ERR_DIR, ERR_PATH, ERR_FILE, STATUS_OK };

UIState currentState = HOME;
ErrorCode lastError = ERR_NONE;
int8_t selected = 0;

// Hardware
const uint8_t sensor_pin = A7;
const uint8_t servoPins[2] = {4, 3};
Servo servos[2];
bool movingRight = true;

// SD Config
const uint8_t chipSelect = 10;
bool connectedSD = false;
const char dirName[] = "ZAKAMER1"; 
char nextFileName[32] = ""; 
uint16_t currentFileIndex = 0;

// Simple clean path builder: "ZAKAMER1/IMG000.TXT"
void formatFileName(char* buffer, uint16_t index) {
  snprintf(buffer, 32, "%s/IMG%03d.TXT", dirName, index);
}

void getNextFileName() {
  while (currentFileIndex < 999) {
    formatFileName(nextFileName, currentFileIndex);
    if (!SD.exists(nextFileName)) {
      Serial.print(F("[SD] Target file found: "));
      Serial.println(nextFileName);
      return;
    }
    currentFileIndex++;
  }
}

void setUpSD() {
  Serial.println(F("[SD] Mounting SD..."));
  
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);

  if (!SD.begin(SPI_QUARTER_SPEED, chipSelect)) {
    connectedSD = false;
    lastError = ERR_NO_SD;
    Serial.println(F("[SD] FAIL: Card mount error."));
    return;
  }
  connectedSD = true;

  if (!SD.exists(dirName)) {
    Serial.println(F("[SD] Creating directory..."));
    if (!SD.mkdir(dirName)) {
      connectedSD = false;
      lastError = ERR_DIR;
      Serial.println(F("[SD] FAIL: Mkdir failed."));
      return;
    }
  }

  getNextFileName();
}

// Noise-filtering analog sampling (median/trimmed average to remove white noise dots)
int readCleanSensor() {
  const uint8_t SAMPLES = 8;
  int reads[SAMPLES];
  
  for (uint8_t i = 0; i < SAMPLES; i++) {
    reads[i] = analogRead(sensor_pin);
    delay(35);
  }

  // Simple bubble sort to find robust median & drop outliers
  for (uint8_t i = 0; i < SAMPLES - 1; i++) {
    for (uint8_t j = i + 1; j < SAMPLES; j++) {
      if (reads[i] > reads[j]) {
        int temp = reads[i];
        reads[i] = reads[j];
        reads[j] = temp;
      }
    }
  }

  // Take average of the middle 4 values (drops highest and lowest noise spikes)
  int sum = 0;
  for (uint8_t i = 2; i < 6; i++) {
    sum += reads[i];
  }
  return sum / 4;
}

void shootAll(const char* photoPath) {
  if (!connectedSD) { lastError = ERR_NO_SD; return; }
  if (photoPath[0] == '\0') { lastError = ERR_PATH; return; }

  File dataFile = SD.open(photoPath, FILE_WRITE);
  if (!dataFile) { 
    lastError = ERR_FILE; 
    Serial.println(F("[SHOOT] FAIL: Cannot open file."));
    return; 
  }

  // Attach servos prior to scanning
  servos[0].attach(servoPins[0]);
  servos[1].attach(servoPins[1]);

  // Move smoothly to home position prior to starting
  servos[0].write(0);
  servos[1].write(0);
  delay(300);

  // Draw visual feedback on display during scan
  u8g2.firstPage();
  do {
    u8g2.setCursor(20, 60);
    u8g2.print(F("Scanning..."));
  } while (u8g2.nextPage());


  dataFile.print('[');
  movingRight = true;

  for (uint8_t y = 0; y < 30; y++) {
    dataFile.print('[');
    servos[1].write(y * 6);
    delay(50); // Settling time after pan movement

    for (uint8_t x = 0; x < 31; x++) {
      uint8_t targetX = movingRight ? (x * 6) : ((30 - x) * 6);
      servos[0].write(targetX);
      delay(140); // Settling time after tilt movement

      int cleanValue = readCleanSensor();
      
      dataFile.print(cleanValue);
      if (x != 30) dataFile.print(F(", "));
    }
    movingRight = !movingRight;
    dataFile.print(']');
    if (y != 29) dataFile.print(F(", "));
    dataFile.flush();
  }
  dataFile.print(']');
  dataFile.close();

  // Silence servos to end mechanical whine & electronic noise
  servos[0].write(90);
  servos[1].write(90);
  delay(150);
  servos[0].detach();
  servos[1].detach();

  lastError = STATUS_OK;
  currentFileIndex++;
  getNextFileName();
}

void loopBTNS() {
  buttons[4] = false;
  for (uint8_t i = 0; i < 4; i++) {
    bool state = !digitalRead(btn_pins[i]);
    buttons[i] = state && !last_btns[i];
    last_btns[i] = state;
    if (buttons[i]) buttons[4] = true;
  }
}

void setup() {
  Serial.begin(9600);
  delay(400);

  pinMode(sensor_pin, INPUT);
  for (uint8_t i = 0; i < 4; i++) pinMode(btn_pins[i], INPUT_PULLUP);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_helvR08_tr);

  setUpSD();

  // Attach servos prior to scanning
  servos[0].attach(servoPins[0]);
  servos[1].attach(servoPins[1]);

  // Move smoothly to home position prior to starting
  servos[0].write(90);
  servos[1].write(90);

  delay(150);
  servos[0].detach();
  servos[1].detach(); 
}

void displayError() {
  u8g2.setCursor(10, 115);
  switch (lastError) {
    case ERR_NO_SD:   u8g2.print(F("ERR: NO SD")); break;
    case ERR_DIR:     u8g2.print(F("ERR: DIR FAIL")); break;
    case ERR_PATH:    u8g2.print(F("ERR: PATH FAIL")); break;
    case ERR_FILE:    u8g2.print(F("ERR: FILE OPEN")); break;
    case STATUS_OK:   u8g2.print(F("OK: DONE")); break;
    default: break;
  }
}

void loop() {
  loopBTNS();

  if (currentState == HOME) {
    if (buttons[1]) {
      if (selected == 0) currentState = SHOOT;
      else if (selected == 1) {
        currentState = CALIB;
        delay_time = 150;
      }
    }
    else if (buttons[0]) selected = (selected <= 0) ? 1 : selected - 1;
    else if (buttons[2]) selected = (selected >= 1) ? 0 : selected + 1;

    u8g2.firstPage();
    do {
      u8g2.setCursor(20, 15);
      u8g2.print(F("Photo"));
      u8g2.setCursor(20, 45);
      u8g2.print(F("Calib"));
      
      displayError();
      u8g2.drawFrame(4, selected * 30 + 8, 10, 4);
    } while (u8g2.nextPage());

  } else if (currentState == SHOOT) {
    if (connectedSD) {
      shootAll(nextFileName);
    } else {
      lastError = ERR_NO_SD;
    }
    currentState = HOME;

  } else if (currentState == CALIB) {
    if (buttons[4]) {
      selected = 0;
      currentState = HOME;
      delay_time = 50;
    }

    u8g2.firstPage();
    do {
      u8g2.setCursor(55, 32);
      u8g2.print(analogRead(sensor_pin));
      u8g2.setCursor(48, 50);
      u8g2.print(F("Back"));
    } while (u8g2.nextPage());
  }

  delay(delay_time);
}