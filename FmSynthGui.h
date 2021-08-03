#pragma once
#include "build.h"
#include "aeffguieditor.h"
#include "FmSynth.h"
#include <vector>

constexpr int TOP_MARGIN = 30;
constexpr int LEFT_MARGIN = 10;

constexpr int GRID_SIZE = 60;

constexpr int KNOB_SIZE = 40;

constexpr int TEXT_H = 16;

#define GRID_X(x) LEFT_MARGIN + GRID_SIZE *(x)

#define GRID_Y(y) TOP_MARGIN + GRID_SIZE *(y)

#define GRID_RECT(name, x, y, w, h) CRect name(GRID_X(x), GRID_Y(y), GRID_X(x) + (w), GRID_Y(y) + (h))

#define ADD_TEXT(text, x, y, w, h, aexpr)     \
    do                                        \
    {                                         \
        GRID_RECT(r, x, y, w, h);             \
        auto label = new CTextLabel(r, text); \
        setColors(label, false);              \
        xframe->addView(label);               \
        aexpr;                                \
    } while (0)

extern CColor ice, bggray;

constexpr int tagPresetList = 102;
constexpr int tagPresetActionList = 103;
constexpr int tagParam = 202;
constexpr int tagPresetNameEdit = 301;

class Knob : public CKnob
{
public:
    int paramId;
    CTextLabel *label;

    Knob(const CRect &size, CControlListener *listener, long tag, CBitmap *background,
         CBitmap *handle, int id) : CKnob(size, listener, tag, background, handle), paramId(id)
    {
    }

    void setValue(float value)
    {
        CKnob::setValue(value);
        setLabel(value);
    }

    void setLabel(float value)
    {
        if (paramId == idx_osc_1__fm_parameters__waveform ||
            paramId == idx_osc_2__fm_parameters__waveform ||
            paramId == idx_osc_3__fm_parameters__waveform ||
            paramId == idx_osc_4__fm_parameters__waveform)
        {
            int v = value * 4 * 0.999;
            const char waveTableNames[][8] = {
                "-/\\/\\-",
                "-/\\---",
                "_/\\/\\_",
                "/\\/\\--",
            };
            label->setText(waveTableNames[v]);
            return;
        }
        else if (paramId == idx_filter_type)
        {
            int v = value * 3 * 0.999;
            const char typeNames[][8] = {
                "off", "lo-pass", "hi-pass"
            };
            label->setText(typeNames[v]);
            return;
        }
        char text[5];
        int vint = value * 100;
        if (vint == 100)
            sprintf(text, "100");
        else if (vint < 10)
            sprintf(text, "00%d", vint);
        else
            sprintf(text, "0%d", vint);
        label->setText(text);
    }
};

class FmSynthGui : public AEffGUIEditor, public CControlListener
{
    std::vector<Knob *> knobs;
    COptionMenu *presetList = nullptr;
    COptionMenu *presetActionList = nullptr;
    CBitmap *knobBackground = nullptr;
    CTextEdit *currentPresetNameEdit = nullptr;

    std::string currentPresetName = "<Preset name>";

    int presetNumber = -1;
    bool useRoundedValues = true;

    FmSynth *synth()
    {
        return (FmSynth *)effect;
    }

    CBitmap *loadBitmap(const char *relativePath)
    {
        std::string s = Util::getWorkDir() + "\\" + relativePath;
        std::wstring ws(s.size(), L'#');
        mbstowcs(&ws[0], s.c_str(), s.size());
        auto bmp = Gdiplus::Bitmap::FromFile(ws.c_str(), false);
        auto cbmp = new CBitmap(bmp);
        delete bmp;
        return cbmp;
    }

    Knob *addKnob(CFrame *xframe, int x, int y, int idx, int tag)
    {
        const CColor cBg = kBlackCColor, cFg = ice;
        GRID_RECT(knobRect, x, y, KNOB_SIZE, KNOB_SIZE);
        auto knob = new Knob(knobRect, this, tag, knobBackground, nullptr, idx);
        ADD_TEXT("00", x, y + 0.7, KNOB_SIZE, TEXT_H, knob->label = label);
        xframe->addView(knob);
        knob->setValue(synth()->getParameter(idx));
        knobs.push_back(knob);
        return knob;
    }

    void setColors(CParamDisplay *ctrl, bool setFrameColor = true)
    {
        const CColor cBg = kBlackCColor, cFg = ice;
        ctrl->setBackColor(cBg);
        if (setFrameColor)
            ctrl->setFrameColor(cFg);
        ctrl->setFontColor(cFg);
    }

public:
    FmSynthGui(void *ptr) : AEffGUIEditor(ptr)
    {
    }
    ~FmSynthGui()
    {
    }

    bool open(void *ptr)
    {
        CRect frameSize(0, 0, GRID_X(16) + LEFT_MARGIN, GRID_Y(14));
        CColor cBg = kBlackCColor, cFg = ice;
        ERect *wSize;
        getRect(&wSize);

        wSize->top = wSize->left = 0;
        wSize->bottom = (VstInt16)frameSize.bottom;
        wSize->right = (VstInt16)frameSize.right;

        auto xframe = new CFrame(frameSize, ptr, this);

        knobBackground = loadBitmap("FmSynthKnob.bmp");

        xframe->setBackgroundColor(cBg);
        
        ADD_TEXT("Complex Kookaburra -- 4 OP non-fixed architecture FM Synth", 0, -0.375, GRID_SIZE * 14, TEXT_H, );

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
            if (cursY == 13)
            {
                cursY = 0;
                cursX += 4;
            }
        }

        ADD_TEXT(getNameForParam(idx_filter, true), 6, 13.35, GRID_SIZE, TEXT_H, );
        addKnob(xframe, 7, 13, idx_filter, tagParam);
        ADD_TEXT(getNameForParam(idx_filter_type, true), 8, 13.35, GRID_SIZE, TEXT_H, );
        addKnob(xframe, 9, 13, idx_filter_type, tagParam);

        GRID_RECT(presetNameEditRect, 0, 13, GRID_SIZE * 6, TEXT_H * 1.5);
        currentPresetNameEdit = new CTextEdit(presetNameEditRect, this, tagPresetNameEdit, currentPresetName.c_str());
        setColors(currentPresetNameEdit);
        currentPresetNameEdit->setBackColor(bggray);

        xframe->addView(currentPresetNameEdit);

        ADD_TEXT("Preset", 0, 13.5, GRID_SIZE * 1, TEXT_H, label->setHoriAlign(kLeftText));
        GRID_RECT(presetRect, 1, 13.5, 2 * GRID_SIZE, TEXT_H);
        presetList = new COptionMenu(presetRect, this, tagPresetList);
        setColors(presetList);

        presetList->addEntry(new CMenuItem("Select preset", 1 << 1));

        auto presetMgr = synth()->getPresetManager();
        for (int i = 0; i < presetMgr->presetNames.size(); i++)
        {
            presetList->addEntry(new CMenuItem(presetMgr->presetNames[i].c_str()));
        }

        xframe->addView(presetList);

        GRID_RECT(presetActRect, 3.5, 13.5, 2 * GRID_SIZE, TEXT_H);
        presetActionList = new COptionMenu(presetActRect, this, tagPresetActionList);
        setColors(presetActionList);
        presetActionList->addEntry(new CMenuItem("Preset actions...", 1 << 1));
        presetActionList->addEntry(new CMenuItem("Save preset"));
        presetActionList->addEntry(new CMenuItem("Save preset as new"));
        xframe->addView(presetActionList);

        ADD_TEXT("v 1.0 build " BUILD_DATE, 12, 13.25, 4 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));
        ADD_TEXT("(c) 2021 Joonas Salonpaa", 12, 13.5, 4 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));
        ADD_TEXT("github.com/biocommando/FmSynth", 12, 13.75, 4 * GRID_SIZE, TEXT_H, label->setHoriAlign(kRightText));

        /*auto logoBmp = loadBitmap("TranSynLogo.bmp");

        CRect logoRect(wSize->right / 2 - 234, 5, wSize->right + 234, 50);
        auto logo = new CView(logoRect);
        logo->setBackground(logoBmp);
        xframe->addView(logo);*/

        knobBackground->forget();
        //logoBmp->forget();

        frame = xframe;

        return true;
    }
    void close()
    {
        auto xframe = frame;
        frame = 0;
        delete xframe;
    }

    void setParameter(int id, float value)
    {
        if (!frame)
            return;
        for (int i = knobs.size() - 1; i >= 0; i--)
        {
            if (knobs[i]->paramId == id)
            {
                knobs[i]->setValue(value);
                return;
            }
        }
    }

    void valueChanged(CControl *control)
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

    void notifyCurrentPresetNameChanged()
    {
        if (synth()->getPresetManager()->getProgramName() != "")
        {
            currentPresetName = synth()->getPresetManager()->getProgramName();
            if (currentPresetNameEdit)
                currentPresetNameEdit->setText(currentPresetName.c_str());
        }
    }
};