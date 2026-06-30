#include <LiquidCrystal.h>
#include <Servo.h>
Servo y_axis;
Servo x_axis;

LiquidCrystal lcd(3, 2, 7, 6, 5, 4);


//taking photos
int pos = 0;

uint8_t samples = 3; //keep it odd
uint8_t step = 3; 

uint8_t fov[2] = {10, 10};
uint8_t start[2] = {(180 - fov[0] * step) / 2, (180 + fov[1] * step) / 2};

bool x_dir_right = true;

int8_t x_coord = 0;


//system

uint8_t delay_time = 30;
bool buttons[4] = {false}; // left-up-right-down
bool last_btns[4] = {false}; // left-up-right-down
uint8_t btn_pins[4] = {11, 10, 9, 8};


class Mode {
  public:
    String name;
    void (*onEntry)();
    void (*loop)();
};

Mode* mode_list[4] = {};

uint8_t active = 0;

  // MENU
byte icons[6][8] = {
  {0, 0, 12, 10, 9, 10, 12, 0}, //empty right
  {0, 0, 12, 14, 15, 14, 12, 0}, //full right
  {0, 4, 10, 17, 0, 17, 10, 4}, //empty up-down
  {0, 4, 14, 31, 0, 31, 14, 4}, //full up-down
  {0, 0, 6, 10, 18, 10, 6, 0},
  {0, 0, 6, 14, 30, 14, 6, 0} //full left
};




int8_t selected = 0;
uint8_t display_offset = 0;

  // Settings
uint8_t picture_atrib[3] = {10, 10, 3};


int compare(const void *a, const void *b) {
  return (*(int*)a - *(int*)b);
}

// manage button states (pressed)
void manageButtons() {
  for(int i = 0; i < 4; i++) {
    if (!digitalRead(btn_pins[i])) {
      if (!last_btns[i]) {
        buttons[i] = true;
        last_btns[i] = true;
      } else {
        buttons[i] = false;
      }
    } else {
      last_btns[i] = false;
      buttons[i] = false;
    }
  }
}


// raeding the sensor
void capturePoint() {
  y_axis.write(start[0] + x_coord * step);
  delay(300);
  int buf[samples];
  for (int i = 0; i < samples; i++) {
      buf[i] = analogRead(A7);
      delay(66);
  }

  qsort(buf, samples, sizeof(int), compare);

  Serial.println(buf[samples / 2]);// median
}

void takePhoto() {
  x_dir_right = true;
  fov[0] = picture_atrib[0];
  fov[1] = picture_atrib[1];
  step = min(180 / fov[0], min(180 / fov[1], picture_atrib[2]));

  start[0] = (180 - fov[0] * step) / 2;
  start[1] = (180 + fov[1] * step) / 2;

  y_axis.write(start[0]);
  delay(300);
  x_axis.write(start[1]);
  delay(300);

  Serial.println("start");
  delay(120);


  for (int y_coord = 0; y_coord < fov[1]; y_coord++) {
    x_axis.write(start[1] - y_coord * step);
    delay(200);
    
    if (x_dir_right) {
      for (x_coord = 0; x_coord < fov[0]; x_coord++) {
        capturePoint();
      }

    } else {
      for (x_coord = fov[0] - 1; x_coord >= 0; x_coord--) {
        capturePoint();
      }
    }
    x_dir_right = x_dir_right == false;
    
    Serial.println("nl");
  }
  Serial.println("end");
}


// modes
void switchMode(int index) {
  active = index;
  lcd.clear();
  mode_list[active]->onEntry();
}
void enterHome() {
  lcd.createChar(0, icons[0]);
  lcd.createChar(1, icons[1]);

  selected = 0;
}

void HomeLoop() {

  int num_modes = sizeof(mode_list)/sizeof(mode_list[0]) - 2;

  if (buttons[2]) {
    switchMode(1 + selected);
    return;
  }

  if (buttons[1]) {
    selected--;
    if (selected < 0) {
      selected = num_modes - 1;
    }
  }
  if (buttons[3]) {
    selected++;
  }
  
  
  selected = selected % num_modes;

  lcd.setCursor(0, 0);
  lcd.print("Menu");

  for (int mode_index = display_offset; mode_index < display_offset + num_modes; mode_index++) {
    lcd.setCursor(5, mode_index - display_offset);
    if (selected == mode_index) {
      lcd.write(byte(1));
    } else {
      lcd.write(byte(0));
    }
    lcd.print(mode_list[mode_index + 1]->name);
  }



}

Mode HomeNode = {
  "Home",
  enterHome,
  HomeLoop
};


void SettingsEnter() {
  lcd.createChar(0, icons[2]);
  lcd.createChar(1, icons[3]);
  lcd.createChar(2, icons[0]);
  lcd.createChar(3, icons[1]);
  selected = 0;
}

void SettingsLoop() {
  lcd.setCursor(0, 0);

  if (buttons[0]) {
    selected--;
    if (selected < 0) {
      switchMode(0);
      return;
    }
  }
  if (buttons[2]) {
    selected++;
    if (selected > 3) {
      switchMode(3);
      return;
    }
  }
  if (buttons[1]) {
    if (selected < 3) {
      picture_atrib[selected] = min(picture_atrib[selected]++, 180);
    }
  }
  if (buttons[3]) {
    if (selected < 3) {
      picture_atrib[selected] = max(picture_atrib[selected]--, 1);
    }
  }


  lcd.print("Sett");
  lcd.setCursor(0, 1);
  lcd.print("ings");

  for (int i = 0; i < 3; i++) {
    lcd.setCursor(6 + i * 3, 0);
    if (i == selected) {
      lcd.write(byte(1));
    } else {
      lcd.write(byte(0));
    }

    lcd.setCursor(5 + i * 3, 1);
    lcd.print(picture_atrib[i]);
    if (picture_atrib[i] < 100) lcd.print(" ");
    if (picture_atrib[i] < 10) lcd.print(" ");

  }

  lcd.setCursor(15, 0);

  if (selected > 2) {
    lcd.write(byte(3));
  } else {
    lcd.write(byte(2));
  }


}

Mode SettingsNode = {
  "Shoot",
  SettingsEnter,
  SettingsLoop
};

void PhotoEnter() {
  lcd.setCursor(3, 0);
  lcd.print("Scanning...");
  takePhoto();
}

void PhotoLoop() {
  switchMode(0);
  return;
}

Mode PhotoNode = {
  "Photo",
  PhotoEnter,
  PhotoLoop
};

void CalibEnter() {
  lcd.createChar(0, icons[4]);
  lcd.createChar(1, icons[5]);

  y_axis.write(90);
  x_axis.write(90);
  return;
}

void CalibLoop() {
  if (buttons[0]) {
    switchMode(0);
    return;
  }

  int reading = analogRead(A7);

  lcd.setCursor(1, 0);
  lcd.write(byte(1));

  lcd.setCursor(4, 0);
  lcd.print("Calibrate");

  Serial.println(reading);
  reading = reading * 256 / 1024;
  lcd.setCursor(4, 1);
  if (reading < 100) lcd.print(" ");
  if (reading < 10) lcd.print(" ");
  lcd.print(reading);
  lcd.print("/255");

  delay(60);
}

Mode CalibNode = {
  "Calibrate",
  CalibEnter,
  CalibLoop
};


void setup() {
  Serial.begin(115200);
  delay(1500);
  lcd.begin(16, 2);

  y_axis.attach(13);
  x_axis.attach(12);

  pinMode (11, INPUT_PULLUP);
  pinMode (10, INPUT_PULLUP);
  pinMode (9, INPUT_PULLUP);
  pinMode (8, INPUT_PULLUP);
  pinMode (A7, INPUT);

  mode_list[0] = &HomeNode;
  mode_list[1] = &SettingsNode;
  mode_list[2] = &CalibNode;
  mode_list[3] = &PhotoNode;

  active = 0;
  mode_list[active]->onEntry();
}

void loop() {
  manageButtons();

  mode_list[active]->loop();
}
