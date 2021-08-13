#pragma once

#include "common.h"
#include <fstream>

struct SynthSetting
{
    std::string name;
    std::string value;
};

class Settings
{
    std::vector<struct SynthSetting> settings;

public:
    Settings(const std::string &fileName = "settings.txt")
    {
        std::ifstream f;
        f.open(Util::getWorkDir() + "\\" + fileName);
        std::string s;
        while (std::getline(f, s))
        {
            auto ss = Util::splitString(s, '=');
            if (ss.size() == 2)
            {
                struct SynthSetting setting = {ss[0], ss[1]};
                settings.push_back(setting);
            }
        }
    }

    std::string getSetting(const std::string &name, const std::string &defaultValue = "")
    {
        for (int i = 0; i < settings.size(); i++)
        {
            if (settings[i].name == name)
                return settings[i].value;
        }
        return defaultValue;
    }

    float getNumericSetting(const std::string &name, float defaultValue = 0)
    {
        auto s = getSetting(name);
        return s != "" ? std::stof(s) : defaultValue;
    }
};