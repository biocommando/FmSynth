#include "FmSynthGui.h"
#include "PluginExecutor.h"
#include <fstream>
#include <map>

extern std::ofstream *getLogger();

CColor frontColor = {63, 72, 204, 255};
CColor menuBgColor = {11, 11, 11, 255};

constexpr int tagPresetList = 102;
constexpr int tagPresetActionList = 103;
constexpr int tagMacroList = 104;
constexpr int tagParam = 202;
constexpr int tagPresetNameEdit = 301;

constexpr int n_osc_params = group_oscillator_length;

std::map<std::string, std::vector<std::string>> readMacros()
{
    std::map<std::string, std::vector<std::string>> macros;
    const auto dir = Util::getWorkDir();
    const auto file = dir + "\\plugins\\plugin_list.txt";
    std::ifstream ifs;
    ifs.open(file);
    for (std::string s; std::getline(ifs, s);)
    {
        const auto v = Util::splitString(s, ';');
        if (v.size() >= 2)
            macros[v[0]] = v;
    }
    return macros;
}

bool FmSynthGui::open(void *ptr)
{
    CRect frameSize(0, 0, GRID_X(16) + LEFT_MARGIN, GRID_Y(15));
    CColor cBg = kBlackCColor, cFg = frontColor;
    ERect *wSize;
    getRect(&wSize);

    wSize->top = wSize->left = 0;
    wSize->bottom = (VstInt16)frameSize.bottom;
    wSize->right = (VstInt16)frameSize.right;

    auto xframe = new CFrame(frameSize, ptr, this);

    knobBackground = CBitmapLoader::load("FmSynthKnob.bmp");

    xframe->setBackgroundColor(cBg);

    // Image source https://openclipart.org/detail/321966/a-kookaburra-bird
    auto logoBmp = CBitmapLoader::load("FmSynthLogo.bmp");

    CRect logoRect(wSize->right / 2 - logoBmp->getWidth() / 2, 0,
                   wSize->right / 2 + logoBmp->getWidth() / 2, logoBmp->getHeight());
    auto logo = new CView(logoRect);
    logo->setBackground(logoBmp);
    xframe->addView(logo);

    knobs.clear();

    std::string currParent = "";
    int cursX = 0, cursY = 0;

    for (int i = group_osc_1_start; i < group_osc_1_start + group_oscillator_length * 4; i++)
    {
        auto id = i;
        auto split = Util::splitString(std::string(getNameForParam(id, true)), '/');
        auto parent = split[0] + "  -  " + split[1];
        if (parent != currParent)
        {
            ADD_TEXT(parent.c_str(), cursX, cursY, GRID_SIZE * 3, TEXT_H, label->setHoriAlign(kLeftText));
            currParent = parent;
        }
        ADD_TEXT(split[2].c_str(), cursX, cursY + 0.35, GRID_SIZE * 2.8, TEXT_H, label->setHoriAlign(kRightText));
        addKnob(xframe, 3 + cursX, cursY, id, tagParam);

        cursY++;
        if (cursY == n_osc_params)
        {
            cursY = 0;
            cursX += 4;
        }
    }

    ADD_TEXT(getNameForParam(idx_filter, true), 6, n_osc_params + .35, GRID_SIZE, TEXT_H, );
    addKnob(xframe, 7, n_osc_params, idx_filter, tagParam);
    ADD_TEXT(getNameForParam(idx_filter_type, true), 8, n_osc_params + .35, GRID_SIZE, TEXT_H, );
    addKnob(xframe, 9, n_osc_params, idx_filter_type, tagParam);
    ADD_TEXT(getNameForParam(idx_fix_osc, true), 10, n_osc_params + .35, GRID_SIZE, TEXT_H, );
    addKnob(xframe, 11, n_osc_params, idx_fix_osc, tagParam);
    ADD_TEXT(getNameForParam(idx_lfo_rate, true), 12, n_osc_params + .35, GRID_SIZE, TEXT_H, );
    addKnob(xframe, 13, n_osc_params, idx_lfo_rate, tagParam);

    GRID_RECT(presetNameEditRect, 0, n_osc_params, GRID_SIZE * 6, TEXT_H * 1.5);
    currentPresetNameEdit = new CTextEdit(presetNameEditRect, this, tagPresetNameEdit, currentPresetName.c_str());
    setColors(currentPresetNameEdit);
    currentPresetNameEdit->setBackColor(menuBgColor);

    xframe->addView(currentPresetNameEdit);

    ADD_TEXT("Preset", 0, n_osc_params + .5, GRID_SIZE * 1, TEXT_H, label->setHoriAlign(kLeftText));
    GRID_RECT(presetRect, 1, n_osc_params + .5, 2 * GRID_SIZE, TEXT_H);
    presetList = new COptionMenu(presetRect, this, tagPresetList);
    setColors(presetList);

    presetList->addEntry(new CMenuItem("Select preset", 1 << 1));

    auto presetMgr = synth()->getPresetManager();
    for (int i = 0; i < presetMgr->presetNames.size(); i++)
    {
        presetList->addEntry(new CMenuItem(presetMgr->presetNames[i].c_str()));
    }

    xframe->addView(presetList);

    GRID_RECT(presetActRect, 3.5, n_osc_params + .5, 2 * GRID_SIZE, TEXT_H);
    presetActionList = new COptionMenu(presetActRect, this, tagPresetActionList);
    setColors(presetActionList);
    presetActionList->addEntry(new CMenuItem("Preset actions...", 1 << 1));
    presetActionList->addEntry(new CMenuItem("Save preset"));
    presetActionList->addEntry(new CMenuItem("Save preset as new"));
    xframe->addView(presetActionList);

    CRect macroRect (LEFT_MARGIN, 10, 4 * GRID_SIZE, 10 + TEXT_H);
    macroList = new COptionMenu(macroRect, this, tagMacroList);
    setColors(macroList);
    macroList->addEntry(new CMenuItem("Macros...", 1 << 1));
    for (const auto &macro : readMacros())
    {
        macroList->addEntry(new CMenuItem(macro.first.c_str()));
    }
    xframe->addView(macroList);

    ADD_TEXT("v " VERSION_STRING, 15, n_osc_params - .25, GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));
    ADD_TEXT("B " BUILD_DATE, 14, n_osc_params, 2 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));
    ADD_TEXT("(c) Joonas Salonpaa", 14, n_osc_params + .25, 2 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));
    ADD_TEXT("github.com/bio", 14, n_osc_params + .5, 2 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));
    ADD_TEXT("commando/FmSynth", 14, n_osc_params + .75, 2 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));

    /*auto logoBmp = loadBitmap("TranSynLogo.bmp");

        CRect logoRect(wSize->right / 2 - 234, 5, wSize->right + 234, 50);
        auto logo = new CView(logoRect);
        logo->setBackground(logoBmp);
        xframe->addView(logo);*/

    knobBackground->forget();
    logoBmp->forget();

    frame = xframe;

    return true;
}

void FmSynthGui::valueChanged(CControl *control)
{
    if (!frame)
        return;
    auto tag = control->getTag();

    if (tag == tagPresetList)
    {
        auto presetMgr = synth()->getPresetManager();
        presetNumber = presetList->getCurrentIndex() - 1;
        currentPresetName = presetMgr->readProgram(presetNumber);
        currentPresetNameEdit->setText(currentPresetName.c_str());
        presetList->setCurrent(0);
        synth()->updateParameters();
        for (int i = 0; i < knobs.size(); i++)
        {
            knobs[i]->setValue(synth()->getParameter(knobs[i]->paramId));
        }
    }
    else if (tag == tagPresetActionList)
    {
        auto presetMgr = synth()->getPresetManager();
        const auto action = presetActionList->getCurrentIndex();
        presetActionList->setCurrent(0);
        if (action == 1 && presetNumber >= 0) // save / replace
        {
            presetMgr->saveProgram(presetNumber, currentPresetName);
            presetList->getEntry(presetNumber + 1)->setTitle(currentPresetName.c_str());
        }
        else // save as new
        {
            presetMgr->refresh();
            presetMgr->saveProgram(presetMgr->presetNames.size(), currentPresetName);
            presetNumber = presetMgr->presetNames.size();

            presetList->addEntry(new CMenuItem(currentPresetName.c_str()));
        }
    }
    else if (tag == tagMacroList)
    {
        const auto action = macroList->getCurrentIndex();
        const auto title = macroList->getEntry(action)->getTitle();
        macroList->setCurrent(0);
        auto macros = readMacros();
        const auto &pluginDef = macros[title];
        std::map<std::string, double> extraParams;
        for (int i = 2; i < pluginDef.size(); i++)
        {
            const auto kv = Util::splitString(pluginDef[i], '=');
            if (kv.size() == 2)
                extraParams[kv[0]] = std::stod(kv[1]);
        }
        PluginExecutor executor(Util::getWorkDir() + "\\plugins\\" + pluginDef[1], synth());
        executor.execute(extraParams);
    }
    else if (tag == tagParam)
    {
        auto knob = (Knob *)control;
        const auto idx = knob->paramId;

        float value;
        auto paramid = knob->paramId;
        if (useRoundedValues && (paramid == idx_osc_1__fm_parameters__ratio_nominator ||
                                 paramid == idx_osc_1__fm_parameters__ratio_divider ||
                                 paramid == idx_osc_2__fm_parameters__ratio_nominator ||
                                 paramid == idx_osc_2__fm_parameters__ratio_divider ||
                                 paramid == idx_osc_3__fm_parameters__ratio_nominator ||
                                 paramid == idx_osc_3__fm_parameters__ratio_divider ||
                                 paramid == idx_osc_4__fm_parameters__ratio_nominator ||
                                 paramid == idx_osc_4__fm_parameters__ratio_divider))
        {
            value = ((int)(knob->getValue() * 100)) * 0.01;
        }
        else
        {
            value = knob->getValue();
        }

        synth()->setParameterAutomated(idx, value);
        knob->setLabel(value);
    }
    else if (tag == tagPresetNameEdit)
    {
        char name[256];
        currentPresetNameEdit->getText(name);
        currentPresetName.assign(name);
    }
}
