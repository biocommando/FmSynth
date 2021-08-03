#pragma once

#include "common.h"
#include "Parameter.h"

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