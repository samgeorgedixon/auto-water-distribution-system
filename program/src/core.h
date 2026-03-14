#pragma once

#include "config.h"

#ifdef DEBUG
    #define LOGf(...) Serial.printf(__VA_ARGS__)
#else
    #define LOGf(...)
#endif
