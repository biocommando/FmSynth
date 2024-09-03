#pragma once

#include "common.h"

class Parameter
{
    int index;
public:
    float value;

    Parameter(int index, float value = 0) : value(value), index(index) {}

    const char *getShortName() { return getNameForParam(index, false); }

    const char *getFullName() { return getNameForParam(index, true); }

    int getId() { return getSaveIdForParam(index); }

    int getIndex() const { return index; }
};