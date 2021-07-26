#pragma once

class DcFilter
{
    float factor;
    float state[2] = {0, 0};

public:
    DcFilter(float sampleRate)
    {
        const float cutFreqHz = 10;
        factor = 1.0f / (2 * 3.14159265 * 1.0f / sampleRate * cutFreqHz + 1);
    }

    float process(float input)
    {
        state[0] = factor * (input + state[0] - state[1]);
        state[1] = input;

        return state[0];
    }
};