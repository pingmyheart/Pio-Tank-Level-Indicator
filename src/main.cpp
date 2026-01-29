#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <EEPROM.h>

// struct and data
struct Data {
    int maxHeight = 0;
    int longerSide = 0;
    int shorterSide = 0;
    float multiplicationFactor = 0;
};

enum MachineState {
    NORMAL_STATE,
    MENU_SELECTED_STATE,
    CONFIRM_SELECTED_STATE,
    SAVE_DATA_STATE
};

// Prototypes
void normalMachineState();

void menuMachineState();

void confirmMachineState();

void saveMachineState();

int retrieveDistance();

void incrementMenu();

void decrementMenu();

void configureHeight();

void configureShape();

void configureShorterSide();

void configureLongerSide();

// Button Pin
constexpr int menuButtonPin = 0; // D3 on ESP
constexpr int upButtonPin = 2; // D4 on ESP
constexpr int downButtonPin = 14; // D5 on ESP
constexpr int confirmButtonPin = 12; // D6 on ESP

// Sensor Pin
constexpr int trigPin = 13; // D7 on ESP
constexpr int echoPin = 15; // D8 on ESP

// ButtonState
bool menuLastButtonState = HIGH;
bool upLastButtonState = HIGH;
bool downLastButtonState = HIGH;
bool confirmLastButtonState = HIGH;

// Data
auto data = Data();
auto machineState = NORMAL_STATE;
unsigned long lastMeasurement;
unsigned int menuItemSelected = 0;
int upDownValue = 0;
std::vector<String> menuItems = {
    "Configura altezza",
    "Configura forma",
    "Configura lato lungo",
    "Configura lato corto"
};

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
    // EEPROM.begin(512);

    // init
    lastMeasurement = millis();
}

void loop() {
    const bool currentMenuButtonState = digitalRead(menuButtonPin);
    const bool currentUpButtonState = digitalRead(upButtonPin);
    const bool currentDownButtonState = digitalRead(downButtonPin);
    const bool currentConfirmButtonState = digitalRead(confirmButtonPin);

    if (menuLastButtonState == HIGH && currentMenuButtonState == LOW) {
        Serial.println("Menu pressed");
        if (machineState == NORMAL_STATE) {
            machineState = MENU_SELECTED_STATE;
        } else if (machineState == MENU_SELECTED_STATE) {
            machineState = NORMAL_STATE;
        } else if (machineState == CONFIRM_SELECTED_STATE) {
            machineState = MENU_SELECTED_STATE;
        } else {
            machineState = NORMAL_STATE;
        }
        upDownValue = 0;
    }
    if (upLastButtonState == HIGH && currentUpButtonState == LOW) {
        Serial.println("Up pressed");
        if (machineState == MENU_SELECTED_STATE) {
            incrementMenu();
        } else if (machineState == CONFIRM_SELECTED_STATE) {
            upDownValue++;
        }
    }
    if (downLastButtonState == HIGH && currentDownButtonState == LOW) {
        Serial.println("Down pressed");
        if (machineState == MENU_SELECTED_STATE) {
            decrementMenu();
        } else if (machineState == CONFIRM_SELECTED_STATE) {
            upDownValue--;
        }
    }
    if (confirmLastButtonState == HIGH && currentConfirmButtonState == LOW) {
        Serial.println("Confirm pressed");
        if (machineState == MENU_SELECTED_STATE) {
            machineState = CONFIRM_SELECTED_STATE;
        } else if (machineState == CONFIRM_SELECTED_STATE) {
            machineState = SAVE_DATA_STATE;
        }
        upDownValue = 0;
    }

    menuLastButtonState = currentMenuButtonState;
    upLastButtonState = currentUpButtonState;
    downLastButtonState = currentDownButtonState;
    confirmLastButtonState = currentConfirmButtonState;
    delay(20); // small delay for stability

    switch (machineState) {
        case NORMAL_STATE:
            normalMachineState();
            break;
        case MENU_SELECTED_STATE:
            menuMachineState();
            break;
        case CONFIRM_SELECTED_STATE:
            confirmMachineState();
            break;
        case SAVE_DATA_STATE:
            saveMachineState();
            break;
    }
}

// Machine States
void normalMachineState() {
    if (const unsigned long now = millis(); now - lastMeasurement > 10 * 1000) {
        const int distance = retrieveDistance();
        const float waterLevel = static_cast<float>((data.maxHeight - distance) * data.longerSide * data.shorterSide) *
                                 data.multiplicationFactor / 1000;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Misurazione");
        lcd.setCursor(0, 1);
        lcd.print("> " + String(static_cast<int>(waterLevel)) + " L");
        lastMeasurement = now;
        Serial.println("Measurement\n\t> " + String(static_cast<int>(waterLevel)) + " L");
    }
}

void menuMachineState() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu");
    lcd.setCursor(0, 1);
    lcd.print(menuItems[menuItemSelected]);
    Serial.println(menuItems[menuItemSelected]);
}

void confirmMachineState() {
    menuActions[menuItemSelected]();
}

void saveMachineState() {
    Serial.println("Saving state to eprom");
    machineState = MENU_SELECTED_STATE;
}

void incrementMenu() {
    if (menuItemSelected + 1 > menuItems.size() - 1) {
        menuItemSelected = 0;
    } else {
        menuItemSelected++;
    }
}

void decrementMenu() {
    if (menuItemSelected - 1 < 0) {
        menuItemSelected = menuItems.size() - 1;
    } else {
        menuItemSelected--;
    }
}

void configureHeight() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Altezza");
    lcd.setCursor(0, 1);
    lcd.print(String(upDownValue) + " cm");
    Serial.println("Configure Height\n\t" + String(upDownValue) + " cm");
}

void configureShape() {
}

void configureLongerSide() {
}

void configureShorterSide() {
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
