#include <fstream>
#include "FmSynth.h"
#include "FmVoice.h"
#include "GenericDto.h"
#include "build.h"
#include "FmSynthGui.h"

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
    setNumInputs(2);        // stereo in
    setNumOutputs(2);       // stereo out
    setUniqueID(670839338); // identify
    isSynth(true);
    programsAreChunks();

    for (int i = 0; i < total_number_of_parameters; i++)
    {
        parameters.push_back(Parameter(i));
    }

    oversamplingFactor = settings.getNumericSetting("oversampling");
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
    int combined, major = VERSION_MAJOR, minor = VERSION_MINOR;
    versionConvert(&combined, &major, &minor);
    auto version = GenericDto::createInt(combined, reserved_id_version);

    std::string s = version.serialize();
    s += GenericDto::createString(BUILD_DATE, reserved_id_build_date).serialize();
    s += GenericDto::createString(presetManager.getProgramName(), reserved_id_program_name).serialize();
    for (int i = 0; i < parameters.size(); i++)
    {
        auto param = &parameters[i];
        s += GenericDto::serializeParameter(param->value, i);
    }
    chunk = (char *)malloc(s.size() + 1);
    if (chunk)
    {
        memcpy(chunk, s.c_str(), s.size() + 1);
        *data = chunk;
        return s.size() + 1;
    }
    return 0;
}

VstInt32 FmSynth::setChunk(void *data, VstInt32 byteSize, bool isPreset)
{
    auto dtos = GenericDto::deserializeAll(std::string((char *)data, byteSize));
    for (int i = 0; i < dtos.size(); i++)
    {
        const auto dto = &dtos[i];
        if (dto->id == reserved_id_version || dto->id == reserved_id_build_date)
        {
            // reserved ids, pass
        }
        else if (dto->id == reserved_id_program_name)
        {
            presetManager.setProgramName(dto->sValue);

            if (editor)
            {
                ((FmSynthGui *)editor)->notifyCurrentPresetNameChanged();
            }
        }
        else
        {
            for (int i = 0; i < parameters.size(); i++)
            {
                if (parameters[i].getId() == dto->id)
                {
                    if (dto->getType() == 'i')
                        parameters[i].value =
                            Util::getSelectionValue(dto->iValue, getNumberOfOptions(i));
                    else
                        parameters[i].value = dto->fValue;
                    break;
                }
            }
        }
    }
    updateParameters();
    return 0;
}

void FmSynth::updateParameters(int num)
{
    voicesLock.lock();
    for (int i = 0; i < voices.size(); i++)
    {
        if (num == -1)
        {
            for (int j = 0; j < parameters.size(); j++)
            {
                voices[i].updateParameter(parameters[j].index, parameters[j].value);
            }
        }
        else
        {
            voices[i].updateParameter(parameters[num].index, parameters[num].value);
        }
    }
    voicesLock.unlock();
}

void FmSynth::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    voicesLock.lock();
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
    voicesLock.unlock();
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
        strcpy(label, getNameForParam(parameters[index].index, false));
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
    strcpy_s(name, 32, "Complex Kookaburra");
    return true;
}
bool FmSynth::getProductString(char *text)
{
    strcpy_s(text, 64, "Complex Kookaburra - 4 OP non-fixed architecture FM synth");
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
            voicesLock.lock();
            for (int i = 0; i < voices.size(); i++)
            {
                if (voices[i].note == midiMessage[1])
                {
                    voices[i].release();
                }
            }
            voicesLock.unlock();
            voicesChanged = true;
        }
        else if ((midiMessage[0] & 0xF0) == 0b10010000)
        {
            // Note on, key/velocity=midievent->midiData[1]/midievent->midiData[2]
            char key = midiMessage[1];
            char velocity = midiMessage[2];
            auto v = FmVoice(sampleRate, oversamplingFactor, velocity / 127.0f);
            v.note = key;
            v.trigger();
            voicesLock.lock();

            // Remove first pre-existing voice with the same note if there
            // are multiple voices and the limit is exceeded.
            // This is done to save CPU power for sounds with a long release time.
            // Having a relatively high threshold for removing voices makes the
            // optimization rather unnoticeable.
            int first = -1, total = 0;
            for (int i = 0; i < voices.size(); i++)
            {
                if (voices[i].note == key)
                {
                    if (first == -1)
                    {
                        first = i;
                    }
                    total++;
                    if (total == 4)
                    {
                        voices.erase(voices.begin() + first);
                        break;
                    }
                }
            }
            voices.push_back(v);

            voicesLock.unlock();
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
