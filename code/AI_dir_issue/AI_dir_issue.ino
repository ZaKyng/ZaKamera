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
uint8_t angles[2] = {90, 90};
uint8_t targets[2] = {90, 90};
Servo servos[2];
bool movingRight = true;

// SD Card Configuration
const uint8_t chipSelect = 10;
bool connectedSD = false;
char nextFileName[13] = "IMG000.TXT"; 
uint16_t fileIndex = 0;

// Generates lightweight 8.3 FAT root filenames: IMG000.TXT, IMG001.TXT...
void getNextFileName(char* outBuffer) {
  while (fileIndex < 999) {
    outBuffer[0] = 'I';
    outBuffer[1] = 'M';
    outBuffer[2] = 'G';
    outBuffer[3] = '0' + (fileIndex / 100) % 10;
    outBuffer[4] = '0' + (fileIndex / 10) % 10;
    outBuffer[5] = '0' + (fileIndex % 10);
    outBuffer[6] = '.';
    outBuffer[7] = 'T';
    outBuffer[8] = 'X';
    outBuffer[9] = 'T';
    outBuffer[10] = '\0';

    if (!SD.exists(outBuffer)) {
      Serial.print(F("[SD] Target file found: "));
      Serial.println(outBuffer);
      return;
    }
    fileIndex++;
  }
}

void setUpSD() {
  Serial.println(F("[SD] Mounting Card..."));
  
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);

  // Use quarter speed to maintain SPI stability on breadboards/jumper wires
  if (!SD.begin(SPI_QUARTER_SPEED, chipSelect)) {
    connectedSD = false;
    lastError = ERR_NO_SD;
    Serial.println(F("[SD] FAIL: SD Card Mount Error"));
    return;
  }
  
  connectedSD = true;
  Serial.println(F("[SD] OK: Card Mounted"));
  getNextFileName(nextFileName);
}

void moveServo(uint8_t angle, bool xaxis) {
  targets[xaxis ? 0 : 1] = constrain(angle, 0, 180);
}

void shootAll(const char* photoDir) {
  if (!connectedSD) { lastError = ERR_NO_SD; return; }
  if (photoDir[0] == '\0') { lastError = ERR_PATH; return; }

  File dataFile = SD.open(photoDir, FILE_WRITE);
  if (!dataFile) { 
    lastError = ERR_FILE; 
    Serial.println(F("[SHOOT] FAIL: Cannot open file."));
    return; 
  }

  dataFile.print('[');
  movingRight = true;

  for (uint8_t x = 0; x < 30; x++) {
    dataFile.print('[');
    servos[1].write(x * 6);
    delay(40);

    for (uint8_t y = 0; y < 30; y++) {
      servos[0].write(movingRight ? (y * 6) : ((29 - y) * 6));
      delay(100);

      int temp = 0;
      for (uint8_t s = 0; s < 4; s++) {
        temp += analogRead(sensor_pin);
        delay(60);
      }
      dataFile.print(temp / 4);
      if (y != 29) dataFile.print(F(", "));
    }
    movingRight = !movingRight;
    dataFile.print(']');
    if (x != 29) dataFile.print(F(", "));
    dataFile.flush();
  }
  dataFile.print(']');
  dataFile.close();

  lastError = STATUS_OK;
  fileIndex++; // Increment for next scan
  moveServo(90, true);
  delay(30);
  moveServo(90, false);
}

void setUpServos() {
  for (uint8_t i = 0; i < 2; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
  }
}

void loopServos() {
  for (uint8_t i = 0; i < 2; i++) {
    if (targets[i] != angles[i]) {
      angles[i] += (targets[i] > angles[i]) ? min((uint8_t)(targets[i] - angles[i]), (uint8_t)3)
                                            : -min((uint8_t)(angles[i] - targets[i]), (uint8_t)3);
      servos[i].write(angles[i]);
    }
  }
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
  Serial.println(F("\n--- SYSTEM BOOT ---"));

  pinMode(sensor_pin, INPUT);
  for (uint8_t i = 0; i < 4; i++) pinMode(btn_pins[i], INPUT_PULLUP);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_helvR08_tr);

  setUpServos();
  setUpSD();

  Serial.println(F("--- SYSTEM READY ---"));
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
  loopServos();

  if (currentState == HOME) {
    if (buttons[1]) {
      if (selected == 0) currentState = SHOOT;
      else if (selected == 1) {
        currentState = CALIB;
        moveServo(90, true);
        delay(30);
        moveServo(90, false);
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
      for (uint8_t i = 0; i < 60; i++) { loopServos(); delay(20); }
      shootAll(nextFileName);
      if (connectedSD) getNextFileName(nextFileName);
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