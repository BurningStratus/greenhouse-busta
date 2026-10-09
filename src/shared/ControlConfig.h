#pragma once
#include "FreeRTOS.h"

// keep one copy in config_queue, other tasks read it without removing it
struct ControlConfig
{
    // TODO add a verifier to avoid >1500.0f
    std::__atomic_float<float> co2Target = 1000.0f; // initial value in ppm, the user can change it later
};

// keep the values that we need to remember between loop iterations
struct ControlState
{
    bool valveOpen = false;
    bool hasInjected = false;
    TickType_t openAt = 0;
    TickType_t closedAt = 0;

    bool ventilating = false;
    int fanSpeed = -1; // we still don't know the confirmed fan speed
    bool fanFault = false; // no fan fault detected yet
    bool fanCommFault = true; // we still don't have a successful pulse read
    bool fanWriteFault = false; // no speed ControlConfigcommand has failed yet

    unsigned int zeroPulseReads = 0;
    TickType_t lastPulseRead = 0;
};

