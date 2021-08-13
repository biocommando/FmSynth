#pragma once
#include "common.h"
#include "aeffguieditor.h"


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
        const auto opts = getNumberOfOptions(paramId);
        if (opts > 0)
        {
            auto v = Util::getSelection(value, opts);
            label->setText(getOptionLabel(paramId, v));
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
