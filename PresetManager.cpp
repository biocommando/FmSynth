#include "PresetManager.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

bool PresetManager::readProgram(int number, std::string &name, bool readNameOnly, FILE *copyToTmp)
{
    fseek(f, 0, SEEK_SET);
    char buf[256];
    bool pgFound = false;
    name.assign("");
    bool doLoadProgram = !readNameOnly && !copyToTmp;
    while (!feof(f))
    {
        fgets(buf, sizeof(buf), f);
        char cmd = 0;
        int id = 0;
        float value = 0;
        sscanf(buf, "%c %d %f", &cmd, &id, &value);
        if (cmd == '{' && id == number)
        {
            pgFound = true;
            // Set all parameters to initial value
            if (doLoadProgram && value == 0)
            {
                for (int i = 0; i < total_number_of_parameters; i++)
                {
                    parameterHolder[i].value = 0;
                }
            }
        }
        if (!pgFound)
            continue;

        if (copyToTmp)
            fprintf(copyToTmp, "%s", buf);

        if (cmd == '+' && doLoadProgram)
        {
            for (int i = 0; i < parameterHolder.size(); i++)
            {
                if (parameterHolder[i].getId() == id)
                {
                    parameterHolder[i].value = value;
                    break;
                }
            }
        }
        if (cmd == '$')
        {
            buf[strlen(buf) - 1] = 0;
            name.assign(&buf[2]);
            if (readNameOnly)
                break;
            if (doLoadProgram)
                curProgramName = name;
        }

        if (cmd == '}')
            break;
    }
    return pgFound;
}

void PresetManager::init()
{
    presetNames.clear();
    openFile(0);
    bool pgFound = true;
    for (int i = 0; pgFound; i++)
    {
        std::string name;
        pgFound = readProgram(i, name, true);
        if (pgFound)
            presetNames.push_back(name);
    }
    closeFile();
}
void PresetManager::openFile(int rw)
{
    closeFile();
    std::string presetFileName = Util::getWorkDir() + "\\" + fileName;
    f = fopen(presetFileName.c_str(), rw ? "w" : "r");
}

void PresetManager::closeFile()
{
    if (f)
    {
        fclose(f);
        f = nullptr;
    }
}
std::string PresetManager::readProgram(int number)
{
    std::string s;
    openFile(0);
    readProgram(number, s, false);
    closeFile();
    return s;
}

void PresetManager::saveProgram(int number, const std::string &name)
{
    curProgramName = name;
    std::string presetTmpFileName = Util::getWorkDir() + "\\" + fileName + ".tmp";
    FILE *tmp = fopen(presetTmpFileName.c_str(), "w");

    openFile(0);

    for (int i = 0; i < presetNames.size(); i++)
    {
        if (i != number)
        {
            std::string s;
            readProgram(i, s, false, tmp);
        }
    }

    fprintf(tmp, "{ %d\n$ %s\n", number, name.c_str());
    for (int i = 0; i < total_number_of_parameters; i++)
    {
        auto p = &parameterHolder[i];
        fprintf(tmp, "+ %d %f\n", p->getId(), p->value);
    }
    fprintf(tmp, "}\n");
    fclose(tmp);
    closeFile();

    openFile(1);
    tmp = fopen(presetTmpFileName.c_str(), "r");
    char ch;
    while ((ch = fgetc(tmp)) != EOF)
        fputc(ch, f);
    fclose(tmp);
    closeFile();
    init();
}