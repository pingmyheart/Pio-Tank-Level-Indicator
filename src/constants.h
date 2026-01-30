#pragma once

#include <map>
#include <vector>
#include <WString.h>
#include <string>
#include <cmath>

// Button Pin
constexpr int menuButtonPin = 0; // D3 on ESP
constexpr int upButtonPin = 2; // D4 on ESP
constexpr int downButtonPin = 14; // D5 on ESP
constexpr int confirmButtonPin = 12; // D6 on ESP

// Sensor Pin
constexpr int trigPin = 13; // D7 on ESP
constexpr int echoPin = 15; // D8 on ESP

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
