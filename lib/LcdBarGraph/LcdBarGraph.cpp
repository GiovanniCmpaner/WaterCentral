
#include <Arduino.h>

#include <LiquidCrystal.h>
#include <array>
#include <cstdint>

#include "LcdBarGraph.hpp"

namespace Characters
{
    PROGMEM static constexpr uint8_t single[8]{
        0b11111,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b11111};

    PROGMEM static constexpr uint8_t start[8]{
        0b11111,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b11111};

    PROGMEM static constexpr uint8_t end[8]{
        0b11111,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b11111};

    PROGMEM static constexpr uint8_t middle[8]{
        0b11111,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b11111};

    PROGMEM static constexpr uint8_t overflow[8]{
        0b11111,
        0b01011,
        0b10101,
        0b11010,
        0b11010,
        0b10101,
        0b01011,
        0b11111};

    PROGMEM static constexpr const uint8_t underflow[8]{
        0b00000,
        0b00101,
        0b01010,
        0b10100,
        0b10100,
        0b01010,
        0b00101,
        0b00000};
} // namespace Characters

LcdBarGraph::LcdBarGraph(LCD *lcd, uint8_t startSlot, uint8_t middleSlot, uint8_t endSlot)
{
    this->lcd = lcd;
    this->startSlot = startSlot;
    this->middleSlot = middleSlot;
    this->endSlot = endSlot;
}

auto LcdBarGraph::init() -> void
{
    uint8_t customChar[8];

    memcpy_P(customChar, Characters::start, sizeof(customChar));
    lcd->createChar(startSlot, customChar);

    memcpy_P(customChar, Characters::middle, sizeof(customChar));
    lcd->createChar(middleSlot, customChar);

    memcpy_P(customChar, Characters::end, sizeof(customChar));
    lcd->createChar(endSlot, customChar);
}

auto LcdBarGraph::draw(uint8_t slot, int16_t posCol, int16_t posRow, int16_t barLength, double percent) -> void
{
    if (barLength == 0)
    {
        return;
    }

    percent = constrain(percent,-100,+200);

    auto barColumns{static_cast<int16_t>((barLength * 5 - 2) * (percent / 100.0))};

    for (int16_t n{0}; n < barLength; ++n)
    {
        lcd->setCursor(posCol + n, posRow);

        if (barColumns == 0)
        {
            if (n == 0 && n == barLength - 1)
            {
                uint8_t customChar[8];
                memcpy_P(customChar, Characters::single, sizeof(customChar));

                lcd->createChar(slot, customChar);
                lcd->setCursor(posCol + n, posRow);
                lcd->write(static_cast<uint8_t>(slot));
            }
            else if (n == 0)
            {
                lcd->write(static_cast<uint8_t>(startSlot));
            }
            else if (n == barLength - 1)
            {
                lcd->write(static_cast<uint8_t>(endSlot));
            }
            else
            {
                lcd->write(static_cast<uint8_t>(middleSlot));
            }
        }
        else if (barColumns >= 5 - (n == 0 ? 1 : 0) - (n == barLength - 1 ? 1 : 0))
        {
            barColumns -= 5 - (n == 0 ? 1 : 0) - (n == barLength - 1 ? 1 : 0);

            if (barColumns > 0 && n == barLength - 1)
            { //Overflow
                uint8_t customChar[8];
                memcpy_P(customChar, Characters::overflow, sizeof(customChar));

                lcd->createChar(slot, customChar);
                lcd->setCursor(posCol + n, posRow);
                lcd->write(static_cast<uint8_t>(slot));
            }
            else
            {
                lcd->write(static_cast<uint8_t>(0xFF));
            }
        }
        else if (barColumns < 0 && n == 0)
        { //Underflow
            barColumns = 0;

            uint8_t customChar[8];
            memcpy_P(customChar, Characters::underflow, sizeof(customChar));

            lcd->createChar(slot, customChar);
            lcd->setCursor(posCol + n, posRow);
            lcd->write(static_cast<uint8_t>(slot));
        }
        else
        {
            uint8_t customChar[8];

            if (n == 0 && n == barLength - 1)
            {
                memcpy_P(customChar, Characters::single, sizeof(customChar));
            }
            else if (n == 0)
            {
                memcpy_P(customChar, Characters::start, sizeof(customChar));
            }
            else if (n == barLength - 1)
            {
                memcpy_P(customChar, Characters::end, sizeof(customChar));
            }
            else
            {
                memcpy_P(customChar, Characters::middle, sizeof(customChar));
            }

            for (uint8_t nCharCol{n == 0 ? 1 : 0}; nCharCol < (n == barLength - 1 ? 4 : 5); ++nCharCol)
            {
                if (barColumns <= 0)
                {
                    break;
                }
                for (uint8_t nCharRow{1}; nCharRow < 7; ++nCharRow)
                {
                    bitWrite(customChar[nCharRow], 4 - nCharCol, 1);
                }
                barColumns--;
            }

            lcd->createChar(slot, customChar);
            lcd->setCursor(posCol + n, posRow);
            lcd->write(static_cast<uint8_t>(slot));
        }
    }
}