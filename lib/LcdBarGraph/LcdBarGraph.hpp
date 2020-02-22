#pragma once

#include <Arduino.h>
#include <LiquidCrystal.h>

class LcdBarGraph
{
public:
    LcdBarGraph(LCD* lcd, uint8_t startSlot, uint8_t middleSlot, uint8_t endSlot);

    auto init() -> void;
    auto draw(uint8_t slot, int16_t posCol, int16_t posRow, int16_t barLength, double percent) -> void;

private:
    LCD *lcd;
    uint8_t startSlot, middleSlot, endSlot;
};