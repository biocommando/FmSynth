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

class AbstractValueFormatter
{
public:
    virtual std::string format(float value, int paramId) = 0;
    virtual ~AbstractValueFormatter() {}
};

class FloatValueFormatter : public AbstractValueFormatter
{
    float max;

public:
    FloatValueFormatter(float max = 1.0f) : max(max) {}
    std::string format(float value, int paramId) { return std::to_string(value * max).substr(0, 5); }
};

class Integer0To100Formatter : public AbstractValueFormatter
{
    bool zeroPrefix;

public:
    Integer0To100Formatter(bool zeroPrefix = true) : zeroPrefix(zeroPrefix) {}
    std::string format(float value, int paramId)
    {
        int vint = value * 100;
        if (!zeroPrefix)
            return std::to_string(vint);
        if (vint == 100)
            return "100";
        else if (vint < 10)
            return "00" + std::to_string(vint);
        else
            return "0" + std::to_string(vint);
    }
};

class SelectionFormatter : public AbstractValueFormatter
{
public:
    std::string format(float value, int paramId)
    {
        const auto opts = getNumberOfOptions(paramId);
        if (opts > 0)
        {
            auto v = Util::getSelection(value, opts);
            return getOptionLabel(paramId, v);
        }
        return "";
    }
};

class Knob : public CKnob
{
public:
    int paramId;
    CTextLabel *label;
    AbstractValueFormatter &formatter;

    Knob(const CRect &size, CControlListener *listener, long tag, CBitmap *background,
         CBitmap *handle, int id, AbstractValueFormatter &formatter)
        : CKnob(size, listener, tag, background, handle), paramId(id),
          formatter(formatter)
    {
    }

    void setValue(float value)
    {
        CKnob::setValue(value);
        setLabel(value);
    }

    void setLabel(float value)
    {
        label->setText(formatter.format(value, paramId).c_str());
    }
};
