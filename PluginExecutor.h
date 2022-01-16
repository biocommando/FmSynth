#pragma once
#include "FstAudioEffect.h"
#include "ExternalPluginExecutor.h"
#include <map>

class PluginExecutor : public ExternalPluginExecutor
{
    FstAudioEffect *eff;

public:
    PluginExecutor(const std::string &pluginPath, FstAudioEffect *fsteff)
        : ExternalPluginExecutor(pluginPath), eff(fsteff)
    {
    }

    void execute(std::map<std::string, double> extraParams)
    {
        std::map<std::string, int> paramNameToIdxMap;
        int i = 0;
        std::string name;
        do
        {
            name = eff->getParamName(i);
            addParameter({name, eff->getParameter(i)});
            paramNameToIdxMap[name] = i;
            i++;
        } while (name != "");
        for (const auto &extra : extraParams)
        {
            addParameter({extra.first, extra.second});
        }

        if (!ExternalPluginExecutor::execute())
            return;

        for (const auto &item : getOutput())
        {
            eff->setParameter(paramNameToIdxMap[item.getName()], item.getValue());
        }
    }
};