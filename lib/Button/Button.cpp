#include <Arduino.h>

#include "Button.hpp"

Button::Button( uint8_t pin, uint8_t activeLogic, uint8_t mode, uint32_t debounceInterval )
    : pin{pin}, activeLogic{activeLogic}, mode{mode}, debounceInterval{debounceInterval}
{}

auto Button::init() -> void
{
    pinMode( pin, mode );
    lastState = digitalRead( pin );
}

auto Button::process() -> void
{
    const auto now{millis()};

    const auto currentState{digitalRead( pin )};
    if ( currentState != lastState )
    {
        if ( currentState == activeLogic )
        {
            pressTimer = now;
        }
        else
        {
            for ( auto n{0}; n < presses.size(); ++n )
            {
                if( presses[n].pressed and presses[n].releaseCallback != nullptr )
                {
                    presses[n].releaseCallback();
                }
                presses[n].pressed = false;
            }
        }
        lastState = currentState;
    }

    const auto currentPressInterval{ now - pressTimer };
    for ( auto n{0}; n < presses.size(); ++n )
    {
        if ( presses[n].multiPress == 0 )
        {
            if ( currentState == activeLogic )
            {
                if ( not presses[n].pressed and currentPressInterval > debounceInterval and currentPressInterval > presses[n].pressInterval )
                {
                    presses[n].pressed = true;
                    if ( presses[n].pressCallback != nullptr )
                    {
                        presses[n].pressCallback();
                    }
                }
            }
        }
        else
        {
            if ( currentState == activeLogic )
            {
                if ( not presses[n].pressed and currentPressInterval > debounceInterval and currentPressInterval < presses[n].pressInterval )
                {
                    presses[n].pressed = true;
                    presses[n].pressCount++;
                    if ( presses[n].pressCount == presses[n].multiPress )
                    {
                        presses[n].pressCount = 0;
                        if ( presses[n].pressCallback != nullptr )
                        {
                            presses[n].pressCallback();
                        }
                    }
                }
            }
            else
            {
                if ( presses[n].pressCount > 0 and currentPressInterval > presses[n].pressInterval )
                {
                    presses[n].pressed = false;
                    presses[n].pressCount = 0;
                }
            }
        }
    }
}

auto Button::onPress( std::function<void()> pressCallback, std::function<void()> releaseCallback ) -> void
{
    onPress( 0, 0, pressCallback, releaseCallback );
}

auto Button::onPress( uint32_t pressInterval, std::function<void()> pressCallback, std::function<void()> releaseCallback ) -> void
{
    onPress( pressInterval, 0, pressCallback, releaseCallback );
}

auto Button::onPress( uint32_t pressInterval, uint8_t multiPress, std::function<void()> pressCallback, std::function<void()> releaseCallback ) -> void
{
    presses.emplace_back( Press
    {
        false,
        pressInterval,
        multiPress,
        0,
        0,
        pressCallback,
        releaseCallback
    } );
}