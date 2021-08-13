#pragma once

#include "common.h"
#include <cstdlib>
#include <cstring>

class GenericDto
{
    int type;
public:
    int iValue = 0;
    float fValue = 0;
    std::string sValue;
    int id;
    // This is populated only when deserializing a payload
    int byteLength = -1;

    GenericDto(int id = 0) : type(0), id(id) {}

    static GenericDto createInt(int value, int id = 0)
    {
        GenericDto d(id);
        d.iValue = value;
        d.type = 1;
        return d;
    }

    static GenericDto createFloat(float value, int id = 0)
    {
        GenericDto d(id);
        d.fValue = value;
        d.type = 2;
        return d;
    }

    static GenericDto createString(const std::string &value, int id = 0)
    {
        GenericDto d(id);
        d.sValue = value;
        d.type = 3;
        return d;
    }

    static GenericDto deserialize(const std::string &serialized)
    {
        GenericDto d;
        const auto headerSz = 1 + 2 * sizeof(int);

        if (serialized.size() >= headerSz)
        {
            d.type = serialized[0] - '0';
            int len = 0;
            memcpy(&len, serialized.substr(1).c_str(), sizeof(int));
            d.byteLength = len;
            memcpy(&d.id, serialized.substr(1 + sizeof(int)).c_str(), sizeof(int));
            if (serialized.size() < len)
            {
                d.type = 0;
                return d;
            }

            auto val = serialized.substr(headerSz, len - headerSz);
            if (d.type == 1)
            {
                memcpy(&d.iValue, val.c_str(), sizeof(int));
            }
            else if (d.type == 2)
            {
                memcpy(&d.fValue, val.c_str(), sizeof(float));
            }
            else if (d.type == 3)
            {
                d.sValue = val.substr(0, len - headerSz);
            }
            else
            {
                d.type = 0;
            }
        }

        return d;
    }

    static std::vector<GenericDto> deserializeAll(const std::string &serialized)
    {
        std::string s = serialized;
        std::vector<GenericDto> result;
        while (s.size() > 0)
        {
            auto dto = deserialize(s);
            if (!dto.isValid() || dto.byteLength <= 0)
            {
                break;
            }
            result.push_back(dto);
            s = s.substr(dto.byteLength);
        }
        return result;
    }

    static std::string serializeParameter(float value, int idx)
    {
        const auto saveId = getSaveIdForParam(idx);
        const auto opts = getNumberOfOptions(idx);
        if (opts == 0)
            return createFloat(value, saveId).serialize();
        else
            return createInt(Util::getSelection(value, opts), saveId).serialize();
    }

    bool isValid() { return type >= 1 && type <= 3; }

    std::string serialize()
    {
        if (!isValid())
        {
            return "";
        }
        std::string payload;
        if (type == 1)
        {
            payload = std::string((char *)&iValue, sizeof(int));
        }
        else if (type == 2)
        {
            payload = std::string((char *)&fValue, sizeof(float));
        }
        else
        {
            payload = sValue;
        }

        char headerBuf[1 + 2 * sizeof(int)];
        headerBuf[0] = (char)type + '0';
        int len = sizeof(headerBuf) + payload.size();
        memcpy(&headerBuf[1], &len, sizeof(int));
        memcpy(&headerBuf[1 + sizeof(int)], &id, sizeof(int));
        std::string header(headerBuf, sizeof(headerBuf));
        return header + payload;
    }

    // Human readable type mapping: i = int, f = float, s = string, x = invalid
    char getType()
    {
        if (type == 1)
            return 'i';
        if (type == 2)
            return 'f';
        if (type == 3)
            return 's';
        return 'x';
    }
};