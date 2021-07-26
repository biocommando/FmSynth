#pragma once
#include "common.h"

class AbstractVoice
{
public:
    int note;
    virtual bool process(float *channel0, float *channel1) = 0;
    virtual void updateParameter(int id, float value) = 0;
    virtual void trigger() = 0;
    virtual void release() = 0;

    virtual ~AbstractVoice() {}
};