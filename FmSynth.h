#pragma once
#include "audioeffectx.h"
#include "FmVoice.h"
#include "common.h"
#include "Parameter.h"
#include "PresetManager.h"
#include <memory>

class FmSynth : public AudioEffectX
{
private:
    char *chunk = nullptr;
    std::vector<Parameter> parameters;
    std::vector<FmVoice> voices;
    PresetManager presetManager;

    bool validParameter(int idx) { return idx >= 0 && idx < parameters.size(); }

public:
    FmSynth(audioMasterCallback audioMaster);
    ~FmSynth();
    VstInt32 getChunk(void **data, bool isPreset);
    VstInt32 setChunk(void *data, VstInt32 byteSize, bool isPreset);

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
    float getParameter(VstInt32 index);
    void setParameter(VstInt32 index, float value);
    void getParameterName(VstInt32 index, char *label);
    void getParameterDisplay(VstInt32 index, char *text);
    void getParameterLabel(VstInt32 index, char *label);
    bool getEffectName(char *name);
    bool getProductString(char *text);
    bool getVendorString(char *text);
    VstInt32 processEvents(VstEvents *events);
    void open();

    PresetManager *getPresetManager() { return &presetManager; }
    void updateParameters(int num = -1);
};
