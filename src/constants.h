#pragma once

#include <map>
#include <vector>
#include <WString.h>
#include <string>
#include <cmath>

// Button
constexpr unsigned int menuButtonPin = 0; // D3 on ESP
constexpr unsigned int upButtonPin = 2; // D4 on ESP
constexpr unsigned int downButtonPin = 14; // D5 on ESP
constexpr unsigned int confirmButtonPin = 12; // D6 on ESP

// Sensor
constexpr unsigned int trigPin = 13; // D7 on ESP
constexpr unsigned int echoPin = 15; // D8 on ESP
constexpr unsigned int refreshIntervalInSeconds = 5;

// LCD
constexpr unsigned int lcdCols=16;
constexpr unsigned int lcdRows=2;

// Menu
inline std::vector<String> mainMenuItems = {
    "Configura altezza",
    "Configura forma",
    "Configura lato lungo",
    "Configura lato corto",
    "Test sensore"
};

inline std::vector<std::pair<String, float>> availableShapesItems = {
    {"Rettangolo", 1},
    {"Cilindro", M_PI}
};
