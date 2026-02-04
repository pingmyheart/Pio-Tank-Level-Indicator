#pragma once

struct Data {
    int maxHeight = 0;
    int longerSide = 0;
    int shorterSide = 0;
    float multiplicationFactor = 1;
};

class DataSetter {
public:
    static void setIntValue(int& field, const int value);
    static void setFloatValue(float& field, const float value);
};
