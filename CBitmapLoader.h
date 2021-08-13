#pragma once
#include "common.h"
#include "aeffguieditor.h"

class CBitmapLoader
{
public:
    static CBitmap *load(const char *relativePath)
    {
        std::string s = Util::getWorkDir() + "\\" + relativePath;
        std::wstring ws(s.size(), L'#');
        mbstowcs(&ws[0], s.c_str(), s.size());
        auto bmp = Gdiplus::Bitmap::FromFile(ws.c_str(), false);
        auto cbmp = new CBitmap(bmp);
        delete bmp;
        return cbmp;
    }
};