#pragma once
#include "audioeffectx.h"
#include "FmVoice.h"
#include "common.h"
#include <memory>

class Parameter
{
public:
    float value;
    int id;
    Parameter(int id, float value = 0) : value(value), id(id) {}

    const char *getShortName() { return getNameForParam(id, false); }

    const char *getFullName() { return getNameForParam(id, true); }
};


class PresetManager
{
private:
    std::vector<Parameter> &parameterHolder;
    std::string fileName;
    FILE *f = nullptr;
    std::string curProgramName;
    void init();
    void openFile(int rw);
    void closeFile();
    bool readProgram(int number, std::string &name, bool readNameOnly, FILE *copyToTmp = nullptr);

public:
    std::vector<std::string> presetNames;

    PresetManager(std::vector<Parameter> &h, const std::string &file = "FmSynthPresets.dat") : parameterHolder(h), fileName(file)
    {
        init();
    }

    std::string readProgram(int number);

    void saveProgram(int number, const std::string &name);

    void refresh()
    {
        init();
    }

    void setProgramName(const std::string &name) { curProgramName = name; }

    std::string getProgramName() { return curProgramName; }
};

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

    int getParameterIndexById(int id);
    int getParameterIdByIndex(int index);
    PresetManager *getPresetManager() { return &presetManager; }
    void updateParameters(int num = -1);
};
