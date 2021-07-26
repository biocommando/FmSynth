#include <fstream>
#include "FmSynth.h"
#include "FmVoice.h"
#include "GenericDto.h"
#include "build.h"
#include "FmSynthGui.h"
#include <windows.h>

std::string workDir;
void resolveWorkDir()
{
    if (workDir != "")
        return;
    // work out the resource directory
    // first we get the DLL path from windows API
    extern void *hInstance;
    wchar_t workDirWc[1024];
    GetModuleFileName((HMODULE)hInstance, workDirWc, 1024);
    char workDirC[1024];
    wcstombs(workDirC, workDirWc, 1024);

    workDir.assign(workDirC);

    // let's get rid of the DLL file name
    auto posBslash = workDir.find_last_of('\\');
    if (posBslash != std::string::npos)
    {
        workDir = workDir.substr(0, posBslash);
    }

    // Let's find out the actual directory we want to work in
    auto workDirSpec = workDir + "\\FmSynthWorkDir.txt";
    std::ifstream f;
    f.open(workDirSpec);
    std::getline(f, workDirSpec);
    f.close();
    if (workDirSpec != "")
        workDir = workDirSpec;
}

std::ofstream *getLogger()
{
    static bool initDone = false;
    static std::ofstream file;
    if (!initDone)
    {
        file.open("D:\\old pc\\Documents\\Visual Studio 2013\\Projects\\FmSynth\\log.txt");
        initDone = true;
    }
    return &file;
}

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
    return new FmSynth(audioMaster);
}

FmSynth::FmSynth(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 0, total_number_of_parameters), presetManager(parameters)
{
    resolveWorkDir();
    setNumInputs(2);        // stereo in
    setNumOutputs(2);       // stereo out
    setUniqueID(670839338); // identify
    isSynth(true);
    programsAreChunks();

    for (int i = 0; i < total_number_of_parameters; i++)
    {
        parameters.push_back(Parameter(i));
    }
}

FmSynth::~FmSynth()
{
    if (chunk)
    {
        free(chunk);
    }
}

VstInt32 FmSynth::getChunk(void **data, bool isPreset)
{
    if (chunk)
    {
        free(chunk);
        chunk = nullptr;
    }
    auto version = GenericDto::createInt(0, 1000);

    std::string s = version.serialize();
    s += GenericDto::createString(BUILD_DATE, 1001).serialize();
    s += GenericDto::createString(presetManager.getProgramName(), 1002).serialize();
    for (int i = 0; i < parameters.size(); i++)
    {
        auto param = &parameters[i];
        auto dto = GenericDto::createFloat(param->value, param->id);
        s += dto.serialize();
    }
    chunk = (char *)malloc(s.size() + 1);
    memcpy(chunk, s.c_str(), s.size() + 1);
    if (chunk)
    {
        *data = chunk;
        return s.size() + 1;
    }
    return 0;
}

VstInt32 FmSynth::setChunk(void *data, VstInt32 byteSize, bool isPreset)
{
    auto s = std::string((char *)data, byteSize);
    int idx = 0;
    while (idx < byteSize)
    {
        auto dto = GenericDto::deserialize(s.substr(idx));
        if (!dto.isValid() || dto.byteLength <= 0)
        {
            break;
        }
        if (dto.id < 1000)
        {
            for (int i = 0; i < parameters.size(); i++)
            {
                if (parameters[i].id == dto.id)
                {
                    parameters[i].value = dto.fValue;
                    break;
                }
            }
        }
        else if (dto.id == 1002)
        {
            presetManager.setProgramName(dto.sValue);
            
            if (editor)
            {
                ((FmSynthGui*)editor)->notifyCurrentPresetNameChanged();
            }
        }
        idx += dto.byteLength;
    }
    updateParameters();
    return 0;
}

void FmSynth::updateParameters(int num)
{
    for (int i = 0; i < voices.size(); i++)
    {
        if (num == -1)
        {
            for (int j = 0; j < parameters.size(); j++)
            {
                voices[i].updateParameter(parameters[j].id, parameters[j].value);
            }
        }
        else
        {
            voices[i].updateParameter(parameters[num].id, parameters[num].value);
        }
    }
}

void FmSynth::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    for (int i = 0; i < sampleFrames; i++)
    {
        outputs[0][i] = 0;
        outputs[1][i] = 0;
        for (int j = voices.size() - 1; j >= 0; j--)
        {
            float ch0 = 0, ch1 = 0;
            auto stillAlive = voices[j].process(&ch0, &ch1);
            if (!stillAlive)
            {
                voices.erase(voices.begin() + j);
            }
            outputs[0][i] += ch0;
            outputs[1][i] += ch1;
        }
    }
}

float FmSynth::getParameter(VstInt32 index) { return validParameter(index) ? parameters[index].value : 0; }

void FmSynth::setParameter(VstInt32 index, float value)
{
    if (validParameter(index))
    {
        parameters[index].value = value;
        updateParameters(index);
    }
}

void FmSynth::getParameterName(VstInt32 index, char *label)
{
    if (validParameter(index))
    {
        strcpy(label, getNameForParam(parameters[index].id, false));
    }
    else
    {
        *label = 0;
    }
}

void FmSynth::getParameterDisplay(VstInt32 index, char *text)
{
    if (validParameter(index))
    {
        float2string(parameters[index].value, text, kVstMaxParamStrLen);
    }
    else
    {
        *text = 0;
    }
}

void FmSynth::getParameterLabel(VstInt32 index, char *label)
{
    getParameterName(index, label);
}

bool FmSynth::getEffectName(char *name)
{
    strcpy_s(name, 32, "4 OP Free FM Synth");
    return true;
}
bool FmSynth::getProductString(char *text)
{
    strcpy_s(text, 64, "4 OP Free FM Synth");
    return true;
}
bool FmSynth::getVendorString(char *text)
{
    strcpy_s(text, 64, "(c) 2021 Joonas Salonpaa");
    return true;
}

VstInt32 FmSynth::processEvents(VstEvents *events)
{
    bool voicesChanged = false;
    for (int i = 0; i < events->numEvents; i++)
    {
        if (!(events->events[i]->type & kVstMidiType))
        {
            continue;
        }
        VstMidiEvent *midievent = (VstMidiEvent *)(events->events[i]);
        const char *midiMessage = midievent->midiData;
        if ((midiMessage[0] & 0xF0) == 0b10000000)
        {
            // Note off, key = midievent->midiData[1]
            for (int i = 0; i < voices.size(); i++)
            {
                if (voices[i].note == midiMessage[1])
                {
                    voices[i].release();
                }
            }
            voicesChanged = true;
        }
        else if ((midiMessage[0] & 0xF0) == 0b10010000)
        {
            // Note on, key/velocity=midievent->midiData[1]/midievent->midiData[2]
            char key = midiMessage[1];
            char velocity = midiMessage[2];
            auto v = FmVoice(sampleRate, velocity / 127.0f);
            v.note = key;
            v.trigger();
            voices.push_back(v);
            voicesChanged = true;
        }
    }
    if (voicesChanged)
    {
        updateParameters();
    }
    return 0;
}

void FmSynth::open()
{
    setEditor(new FmSynthGui(this));
}

int FmSynth::getParameterIndexById(int id)
{
    for (int i = 0; i < parameters.size(); i++)
    {
        if (parameters[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

int FmSynth::getParameterIdByIndex(int index)
{
    if (validParameter(index))
    {
        return parameters[index].id;
    }
    return -1;
}

///////////// Preset manager code (should be splitted)

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
                if (parameterHolder[i].id == id)
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
    resolveWorkDir();
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
    std::string presetFileName = workDir + "\\" + fileName;
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
    std::string presetTmpFileName = std::string(workDir) + "\\" + "TranSynPresets.tmp";
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
        fprintf(tmp, "+ %d %f\n", p->id, p->value);
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
