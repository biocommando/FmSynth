#pragma once

#include "AdsrEnvelope.h"
#include "common.h"
#include "Filter.h"
#include <math.h>

constexpr float pi = 3.14159265358979323846f;
constexpr int lookupTableSize = 8192;

class FmOperator
{
    static inline float calcSin(float phase)
    {
        static float lookup[lookupTableSize];
        static bool init = false;
        if (!init)
        {
            init = true;
            for (int i = 0; i < lookupTableSize; i++)
            {
                lookup[i] = sin(i * 2 * pi / lookupTableSize);
            }
        }
        int pos = (phase / pi + 1) * lookupTableSize / 2;

        if (pos >= lookupTableSize)
            pos = pos % lookupTableSize;
        else if (pos < 0)
            pos = lookupTableSize - ((-pos) % lookupTableSize);
        return lookup[pos];
    }

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

        value = calcSin(modulatedPhase) * env * velocityMultiplier;
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
    int fixedOsc = -1;
    int oversamplingFactor;

    DcFilter dcBlock;
    Filter downSamplingFilter;

    float calculatePhaseInc(float note)
    {
        return pow(2, note / 12) * 16.352 / (sampleRate / 2) * pi;
    }

    void updatePhaseIncForOperator(int op)
    {
        float ratio = ratios[0][op] / (ratios[1][op] < 1e-6 ? 1e-6 : ratios[1][op]);
        if (fixedOsc == op)
            ops[op].phaseInc = ratio * calculatePhaseInc(-24);
        else
            ops[op].phaseInc = ratio * freq;
    }

    bool processOneSample(float *output)
    {
        bool ended = true;
        *output = 0;
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
            *output += outputVol[i] * ops[i].value;
            ended = ended && env[i].ended();
        }
        return !ended;
    }

public:
    float note;

    FmVoice(int sampleRate, int oversamplingFactor, float velocity)
        : sampleRate(sampleRate * oversamplingFactor), oversamplingFactor(oversamplingFactor),
          dcBlock(DcFilter(sampleRate)), velocity(velocity),
          downSamplingFilter(sampleRate * oversamplingFactor)
    {
        ops.resize(4);
        env.resize(4);
        for (int i = 0; i < 4; i++)
        {
            filters.push_back(Filter(sampleRate * oversamplingFactor));
        }
        downSamplingFilter.updateLowpass(sampleRate * 0.5f);
    }

    bool process(float *channel0, float *channel1)
    {
        float v;
        bool ok = true;
        for (int i = 0; i < oversamplingFactor; i++)
        {
            ok = ok && processOneSample(&v);
            v = downSamplingFilter.processLowpass(v);
        }

        v = dcBlock.process(v);
        *channel0 = v;
        *channel1 = *channel0;
        return ok;
    }

    void trigger()
    {
        freq = calculatePhaseInc(note);
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
                ops[opGroup].waveform = Util::getSelection(value, getNumberOfOptions(idx));
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
                filterType = Util::getSelection(value, getNumberOfOptions(idx));
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
        else if (idx == idx_fix_osc)
        {
            if (fixedOsc != -1)
            {
                auto f = fixedOsc;
                fixedOsc = -1;
                updatePhaseIncForOperator(f);
            }
            fixedOsc = Util::getSelection(value, getNumberOfOptions(idx)) - 1;
            if (fixedOsc != -1)
                updatePhaseIncForOperator(fixedOsc);
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