#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include  <machine_state.h>
#include <constants.h>
#include <data.h>
#include <index.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <EEPROM.h>

#include "secrets.h"

// Prototypes
void normalMachineState();

void menuMachineState();

void confirmMachineState();

void saveDataMachineState();

int retrieveDistance();

void incrementMenu();

void incrementShapeSelectionMenu();

void incrementUpDown();

void decrementMenu();

void decrementShapeSelectionMenu();

void decrementUpDown();

void configureHeight();

void configureShape();

void configureShorterSide();

void configureLongerSide();

void testSensor();

void handleIndex();

void configureNetwork();

// ButtonState
bool menuLastButtonState = HIGH;
bool upLastButtonState = HIGH;
bool downLastButtonState = HIGH;
bool confirmLastButtonState = HIGH;

// Data
auto data = Data();
DataSetter dataSetter;
auto machineState = MachineState::NORMAL_STATE;
unsigned long lastMeasurementMillis;
int menuItemSelected = 0;
int shapeMenuItemSelected = 0;
int upDownValue = 0;
bool refreshRequired = true;

std::vector<std::function<void()> > menuIndexToDataReference = {
    []() {
        dataSetter.setIntValue(data.maxHeight, upDownValue);
    },
    []() {
        dataSetter.setFloatValue(data.multiplicationFactor, availableShapesItems[shapeMenuItemSelected].second);
    },
    []() {
        dataSetter.setIntValue(data.longerSide, upDownValue);
    },
    []() {
        dataSetter.setIntValue(data.shorterSide, upDownValue);
    },
    []() {
    }
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
    []() {
        testSensor();
    }
};

// Server
const char *apSsid = "ESP8266_AP";
const char *apPassword = "12345678";

IPAddress apLocalIp(192, 168, 1, 100);
IPAddress apGateway(192, 168, 1, 100);
IPAddress apSubnet(255, 255, 255, 0);

IPAddress localIp(192, 168, 1, 100);
IPAddress localGateway(192, 168, 1, 1);
IPAddress localSubnet(255, 255, 255, 0);

ESP8266WebServer server(80);

// Display
LiquidCrystal_I2C lcd(0x27, lcdCols, lcdRows);

void setup() {
    Wire.begin(D2, D1); // SDA, SCL — adjust to your wiring
    // Wire.setClock(100000); // slow I2C = more stable

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

    // Start Access Point
    configureNetwork();

    // Setup web server
    server.on("/", handleIndex);
    server.begin();
    Serial.println("Web server started");

    // hello world
    lcd.setCursor(0, 0);
    lcd.print("    LOADING");
    Serial.println("    LOADING");
    lcd.setCursor(0, 1);
    lcd.print("AR CORP SOLUTIONS");
    Serial.println("AR CORP SOLUTIONS");
    delay(5000);
}

void loop() {
    server.handleClient();
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
            incrementShapeSelectionMenu();
        }
    }
    if (downLastButtonState == HIGH && currentDownButtonState == LOW) {
        Serial.println("Down pressed");
        if (machineState == MachineState::MENU_SELECTED_STATE) {
            incrementMenu();
        } else if (machineState == MachineState::CONFIRM_SELECTED_STATE) {
            decrementUpDown();
            decrementShapeSelectionMenu();
        }
    }
    if (confirmLastButtonState == HIGH && currentConfirmButtonState == LOW) {
        Serial.println("Confirm pressed");
        if (machineState == MachineState::MENU_SELECTED_STATE) {
            machineState = MachineState::CONFIRM_SELECTED_STATE;
            upDownValue = 0;
        } else if (machineState == MachineState::CONFIRM_SELECTED_STATE) {
            machineState = MachineState::SAVE_DATA_STATE;
        }
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
    if (const unsigned long now = millis(); now - lastMeasurementMillis > refreshIntervalInSeconds * 1000) {
        const int distance = retrieveDistance();
        const float waterLevel = static_cast<float>((data.maxHeight - distance) * data.longerSide * data.shorterSide) *
                                 data.multiplicationFactor / 1000;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MISURAZIONE");
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
    lcd.print("MENU'");
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
    menuIndexToDataReference[menuItemSelected]();
    EEPROM.put(0, data);
    EEPROM.commit();
    machineState = MachineState::MENU_SELECTED_STATE;
    upDownValue = 0;
}

void incrementMenu() {
    refreshRequired = true;
    if (menuItemSelected + 1 > static_cast<int>(mainMenuItems.size()) - 1) {
        menuItemSelected = 0;
    } else {
        menuItemSelected++;
    }
}

void incrementShapeSelectionMenu() {
    refreshRequired = true;
    if (shapeMenuItemSelected + 1 > static_cast<int>(availableShapesItems.size()) - 1) {
        shapeMenuItemSelected = 0;
    } else {
        shapeMenuItemSelected++;
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

void decrementShapeSelectionMenu() {
    refreshRequired = true;
    if (shapeMenuItemSelected - 1 < 0) {
        shapeMenuItemSelected = static_cast<int>(availableShapesItems.size()) - 1;
    } else {
        shapeMenuItemSelected--;
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
    lcd.print("> " + String(upDownValue) + " cm");
    Serial.println("Configure Height\n" + String(upDownValue) + " cm");
    refreshRequired = false;
}

void configureShape() {
    if (!refreshRequired) return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configura Forma");
    lcd.setCursor(0, 1);
    lcd.print("> " + availableShapesItems[shapeMenuItemSelected].first);
    Serial.println("Configure Shape\n" + availableShapesItems[shapeMenuItemSelected].first);
    refreshRequired = false;
}

void configureLongerSide() {
    if (!refreshRequired) return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configura lato lungo");
    lcd.setCursor(0, 1);
    lcd.print("> " + String(upDownValue) + " cm");
    Serial.println("Configure Longer Side\n" + String(upDownValue) + " cm");
    refreshRequired = false;
}

void configureShorterSide() {
    if (!refreshRequired) return;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Configura lato corto");
    lcd.setCursor(0, 1);
    lcd.print("> " + String(upDownValue) + " cm");
    Serial.println("Configure Shorter Side\n" + String(upDownValue) + " cm");
    refreshRequired = false;
}

void testSensor() {
    if (const unsigned long now = millis(); now - lastMeasurementMillis > refreshIntervalInSeconds * 1000) {
        const int distance = retrieveDistance();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MISURAZIONE");
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

void handleIndex() {
    auto html = String(indexHtml); // Copy header string to mutable String
    const int distance = retrieveDistance();
    const float waterLevel = static_cast<float>((data.maxHeight - distance) * data.longerSide * data.shorterSide) *
                             data.multiplicationFactor / 1000;
    html.replace("%water%", String(static_cast<int>(waterLevel)) + "L"); // Replace placeholder
    server.send(200, "text/html", html);
}

void configureNetwork() {
    constexpr int maxAttempts = 20;
    // Try connecting to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.config(localIp, localGateway, localSubnet); // Assign static IP
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to Wi-Fi!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        // Fallback to AP mode
        Serial.println("\nFailed to connect. Starting AP mode...");
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(apLocalIp, apGateway, apSubnet);
        WiFi.softAP(apSsid, apPassword);
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    }
}
