#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include  <machine_state.h>
#include <constants.h>
#include "data.h"

#include <EEPROM.h>

// Prototypes
void normalMachineState();

void menuMachineState();

void confirmMachineState();

void saveDataMachineState();

int retrieveDistance();

void incrementMenu();

void incrementUpDown();

void decrementMenu();

void decrementUpDown();

void configureHeight();

void configureShape();

void configureShorterSide();

void configureLongerSide();

void testSensor();

// ButtonState
bool menuLastButtonState = HIGH;
bool upLastButtonState = HIGH;
bool downLastButtonState = HIGH;
bool confirmLastButtonState = HIGH;

// Data
auto data = Data();
auto machineState = MachineState::NORMAL_STATE;
unsigned long lastMeasurementMillis;
int menuItemSelected = 0;
int upDownValue = 0;
bool refreshRequired = true;

std::vector<std::function<void()> > menuActions = {
    []() {
        configureHeight();
    },
    []() {
        configureShape();
    },
    []() {
        configureLongerSide();
    },
    []() {
        configureShorterSide();
    },
    []() {
        testSensor();
    }
};

// Display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    // init pin
    pinMode(menuButtonPin, INPUT_PULLUP);
    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);
    pinMode(confirmButtonPin, INPUT_PULLUP);

    //init lcd
    lcd.init();
    lcd.backlight();

    // init serial
    Serial.begin(115200);

    // init eeprom
    EEPROM.begin(sizeof(Data));
    EEPROM.get(0, data);
    Serial.println("Data Contains:\n\t"
                   "maxHeight: " + String(data.maxHeight) +
                   "\n\tlongerSide: " + String(data.longerSide) +
                   "\n\tshorterSide: " + String(data.shorterSide) +
                   "\n\tmultiplicationFactor: " + String(data.multiplicationFactor));

    // init
    lastMeasurementMillis = millis();
}

void loop() {
    const bool currentMenuButtonState = digitalRead(menuButtonPin);
    const bool currentUpButtonState = digitalRead(upButtonPin);
    const bool currentDownButtonState = digitalRead(downButtonPin);
    const bool currentConfirmButtonState = digitalRead(confirmButtonPin);

    if (menuLastButtonState == HIGH && currentMenuButtonState == LOW) {
        Serial.println("Menu pressed");
        if (machineState == MachineState::NORMAL_STATE) {
            machineState = MachineState::MENU_SELECTED_STATE;
        } else if (machineState == MachineState::MENU_SELECTED_STATE) {
            machineState = MachineState::NORMAL_STATE;
        } else if (machineState == MachineState::CONFIRM_SELECTED_STATE) {
            machineState = MachineState::MENU_SELECTED_STATE;
        } else {
            machineState = MachineState::NORMAL_STATE;
        }
        upDownValue = 0;
        refreshRequired = true;
    }
    if (upLastButtonState == HIGH && currentUpButtonState == LOW) {
        Serial.println("Up pressed");
        if (machineState == MachineState::MENU_SELECTED_STATE) {
            decrementMenu();
        } else if (machineState == MachineState::CONFIRM_SELECTED_STATE) {
            incrementUpDown();
        }
    }
    if (downLastButtonState == HIGH && currentDownButtonState == LOW) {
        Serial.println("Down pressed");
        if (machineState == MachineState::MENU_SELECTED_STATE) {
            incrementMenu();
        } else if (machineState == MachineState::CONFIRM_SELECTED_STATE) {
            decrementUpDown();
        }
    }
    if (confirmLastButtonState == HIGH && currentConfirmButtonState == LOW) {
        Serial.println("Confirm pressed");
        if (machineState == MachineState::MENU_SELECTED_STATE) {
            machineState = MachineState::CONFIRM_SELECTED_STATE;
        } else if (machineState == MachineState::CONFIRM_SELECTED_STATE) {
            machineState = MachineState::SAVE_DATA_STATE;
        }
        upDownValue = 0;
        refreshRequired = true;
    }

    menuLastButtonState = currentMenuButtonState;
    upLastButtonState = currentUpButtonState;
    downLastButtonState = currentDownButtonState;
    confirmLastButtonState = currentConfirmButtonState;
    delay(20); // small delay for stability

    switch (machineState) {
        case MachineState::NORMAL_STATE:
            normalMachineState();
            break;
        case MachineState::MENU_SELECTED_STATE:
            menuMachineState();
            break;
        case MachineState::CONFIRM_SELECTED_STATE:
            confirmMachineState();
            break;
        case MachineState::SAVE_DATA_STATE:
            saveDataMachineState();
            break;
    }
}

// Machine States
void normalMachineState() {
    if (const unsigned long now = millis(); now - lastMeasurementMillis > 5 * 1000) {
        const int distance = retrieveDistance();
        const float waterLevel = static_cast<float>((data.maxHeight - distance) * data.longerSide * data.shorterSide) *
                                 data.multiplicationFactor / 1000;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Misurazione");
        lcd.setCursor(0, 1);
        lcd.print("> " + String(static_cast<int>(waterLevel)) + " L");
        lastMeasurementMillis = now;
        Serial.println("Measurement\n> " + String(static_cast<int>(waterLevel)) + " L");
    }
}

void menuMachineState() {
    if (!refreshRequired) return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu");
    lcd.setCursor(0, 1);
    lcd.print(mainMenuItems[menuItemSelected]);
    Serial.println("Menu\n" + mainMenuItems[menuItemSelected]);
    refreshRequired = false;
}

void confirmMachineState() {
    menuActions[menuItemSelected]();
}

void saveDataMachineState() {
    Serial.println("Saving state to eprom");
    // read menu item index to match corresponding data
    // save data to eprom
    machineState = MachineState::MENU_SELECTED_STATE;
}

void incrementMenu() {
    refreshRequired = true;
    if (menuItemSelected + 1 > static_cast<int>(mainMenuItems.size()) - 1) {
        menuItemSelected = 0;
    } else {
        menuItemSelected++;
    }
}

void incrementUpDown() {
    upDownValue++;
    refreshRequired = true;
}

void decrementMenu() {
    refreshRequired = true;
    if (menuItemSelected - 1 < 0) {
        menuItemSelected = static_cast<int>(mainMenuItems.size()) - 1;
    } else {
        menuItemSelected--;
    }
}

void decrementUpDown() {
    upDownValue--;
    refreshRequired = true;
}

void configureHeight() {
    if (!refreshRequired) return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configura Altezza");
    lcd.setCursor(0, 1);
    lcd.print(String(upDownValue) + " cm");
    Serial.println("Configure Height\n" + String(upDownValue) + " cm");
    refreshRequired = false;
}

void configureShape() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configura Forma");
    lcd.setCursor(0, 1);
    lcd.print(String(upDownValue) + " cm");
    Serial.println("Configure Height\n" + String(upDownValue) + " cm");
}

void configureLongerSide() {
}

void configureShorterSide() {
}

void testSensor() {
    if (const unsigned long now = millis(); now - lastMeasurementMillis > 3 * 1000) {
        const int distance = retrieveDistance();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Misurazione");
        lcd.setCursor(0, 1);
        lcd.print("> " + String(distance) + " cm");
        lastMeasurementMillis = now;
        Serial.println("Measurement\n> " + String(distance) + " cm");
    }
}

int retrieveDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    const unsigned long duration = pulseIn(echoPin, HIGH);
    const double distance = duration * .0343 / 2;
    return static_cast<int>(distance);
}
