#pragma once

#include <Arduino.h>

#include <vector>
#include <functional>
#include <cstdint>

class Button
{
    public:
        Button( uint8_t pin, uint8_t activeLogic = LOW, uint8_t mode = INPUT_PULLUP, uint32_t debounceInterval = 30 );
        auto init() -> void;
        auto process() -> void;
        auto onPress( std::function<void()> pressCallback, std::function<void()> releaseCallback = nullptr ) -> void;
        auto onPress( uint32_t pressInterval, std::function<void()> pressCallback, std::function<void()> releaseCallback = nullptr ) -> void;
        auto onPress( uint32_t pressInterval, uint8_t multiPress, std::function<void()> pressCallback, std::function<void()> releaseCallback = nullptr ) -> void;
    private:
        uint8_t pin;
        uint8_t activeLogic;
        uint8_t mode;
        uint32_t debounceInterval;
        uint8_t lastState;

        struct Press
        {
            bool pressed;
            uint32_t pressInterval;
            uint8_t multiPress;
            uint32_t lastPressed;
            uint8_t pressCount;
            std::function<void()> pressCallback;
            std::function<void()> releaseCallback;
        };

        uint32_t pressTimer{0};
        std::vector<Press> presses{};
};