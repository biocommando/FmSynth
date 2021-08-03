#pragma once

#include "AdsrEnvelope.h"
#include "common.h"
#include "LowpassFilter.h"
#include <math.h>

constexpr float pi = 3.14159265358979323846f;

class FmOperator
{
public:
    float value = 0;
    float phase = 0;
    float phaseInc = 0;
    float velocityMultiplier = 1;
    float mods[4] = {0, 0, 0, 0};
    int waveform = 0;

    void process(float mod, float env)
    {
        phase += phaseInc;
        if (phase >= pi)
        {
            phase -= pi * 2;
        }
        auto modulatedPhase = phase + mod;
        if (waveform == 1 && modulatedPhase > 0)
        {
            modulatedPhase = 0;
        }
        else if (waveform == 2 && modulatedPhase > 0)
        {
            modulatedPhase = -modulatedPhase;
        }
        else if (waveform == 3)
        {
            if (modulatedPhase > 0)
            {
                modulatedPhase = 0;
            }
            else
            {
                modulatedPhase *= 2;
            }
        }

        value = sin(modulatedPhase) * env * velocityMultiplier;
    }
};

class FmVoice
{
    std::vector<FmOperator> ops;
    std::vector<AdsrEnvelope> env;
    std::vector<Filter> filters;
    float outputVol[4] = {0, 0, 0, 0};
    float ratios[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    float freq;
    float sampleRate;
    float velocity;
    float filterAmount = 0;
    int filterType = 0;

    DcFilter dcBlock;

    float calculatePhaseInc()
    {
        return pow(2, note / 12) * 16.352 / (sampleRate / 2) * pi;
    }

    void updatePhaseIncForOperator(int op)
    {
        float ratio = ratios[0][op] / (ratios[1][op] < 1e-6 ? 1e-6 : ratios[1][op]);
        ops[op].phaseInc = ratio * freq;
    }

public:
    float note;

    FmVoice(int sampleRate, float velocity) : sampleRate(sampleRate),
                                              dcBlock(DcFilter(sampleRate)), velocity(velocity)
    {
        ops.resize(4);
        env.resize(4);
        for (int i = 0; i < 4; i++)
        {
            filters.push_back(Filter(sampleRate));
        }
    }

    bool process(float *channel0, float *channel1)
    {
        bool ended = true;
        *channel0 = 0;
        for (int i = 0; i < 4; i++)
        {
            float fm = 0;
            for (int j = 0; j < 4; j++)
            {
                fm += ops[j].value * ops[i].mods[j];
            }
            env[i].calculateNext();
            ops[i].process(fm, env[i].getEnvelope());
            if (filterType > 0)
            {
                ops[i].value = filterType == 1                               //
                                   ? filters[i].processLowpass(ops[i].value) //
                                   : filters[i].processHighpass(ops[i].value);
            }
            *channel0 += outputVol[i] * ops[i].value;
            ended = ended && env[i].ended();
        }
        *channel0 = dcBlock.process(*channel0);
        *channel1 = *channel0;
        return !ended;
    }

    void trigger()
    {
        freq = calculatePhaseInc();
        for (int i = 0; i < 4; i++)
        {
            env[i].trigger();
        }
    }

    void updateParameter(int idx, float value)
    {
        auto opGroup = (idx - group_osc_1_start) / group_oscillator_length;
        if (opGroup >= 0 && opGroup < 4)
        {
            auto paramInGroup = (idx - group_osc_1_start) % group_oscillator_length;
            if (paramInGroup == idx_osc_1__fm_parameters__ratio_nominator)
            {
                ratios[0][opGroup] = value;
                updatePhaseIncForOperator(opGroup);
            }
            else if (paramInGroup == idx_osc_1__fm_parameters__ratio_divider)
            {
                ratios[1][opGroup] = value;
                updatePhaseIncForOperator(opGroup);
            }
            else if (paramInGroup == idx_osc_1__fm_parameters__velocity_sensitivity)
            {
                value *= value;
                ops[opGroup].velocityMultiplier = (1 - value) + value * velocity;
            }
            else if (paramInGroup == idx_osc_1__fm_parameters__waveform)
            {
                ops[opGroup].waveform = (int)(value * 4 * 0.999);
            }
            else if (paramInGroup == idx_osc_1__modulation_routing__route_to_output)
            {
                outputVol[opGroup] = value;
            }
            else if (paramInGroup >= idx_osc_1__modulation_routing__route_to_osc1 && paramInGroup <= idx_osc_1__modulation_routing__route_to_osc4)
            {
                ops[paramInGroup - idx_osc_1__modulation_routing__route_to_osc1].mods[opGroup] = value * 10;
            }
            else if (paramInGroup == idx_osc_1__envelope__attack)
            {
                env[opGroup].setAttack(sampleRate * 4 * value);
            }
            else if (paramInGroup == idx_osc_1__envelope__decay)
            {
                env[opGroup].setDecay(sampleRate * 4 * value);
            }
            else if (paramInGroup == idx_osc_1__envelope__sustain)
            {
                env[opGroup].setSustain(value);
            }
            else if (paramInGroup == idx_osc_1__envelope__release)
            {
                env[opGroup].setRelease(sampleRate * 4 * value);
            }
        }
        else if (idx == idx_filter_type || idx == idx_filter)
        {
            if (idx == idx_filter_type)
            {
                filterType = (int)(value * 3 * 0.999);
            }
            else
            {
                filterAmount = value * 0.999f;
            }
            if (filterType > 0)
            {
                const float amountSqr = filterAmount * filterAmount;
                const auto factor = filterType == 1 ? 1 - amountSqr : amountSqr;
                const auto filterHz = sampleRate * 0.5f * factor;
                for (int i = 0; i < 4; i++)
                {
                    if (filterType == 1)
                        filters[i].updateLowpass(filterHz);
                    else
                        filters[i].updateHighpass(filterHz);
                }
            }
        }
    }

    void release()
    {
        for (int i = 0; i < 4; i++)
        {
            env[i].release();
        }
    }
};