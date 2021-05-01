#pragma once

#include <sstream>
#include <string>

#include "struct_mapping/struct_mapping.h"

class JsonParser
{
public:
    template<typename T>
    static bool DeserializeJsonString(std::istringstream &jsonStringStream, T &parsedObject)
    {
        try
        {
            struct_mapping::map_json_to_struct(parsedObject, jsonStringStream);
        }
        catch (const std::exception)
        {
            return false;
        }
        return true;
    }
    
    template<typename T>
    static bool SerializeJsonString(const T &jsonObject, std::string &serializedString)
    {
        std::ostringstream jsonStringStream;
        try
        {
            struct_mapping::map_struct_to_json(jsonObject, jsonStringStream);
        }
        catch (const std::exception)
        {
            return false;
        }
        serializedString = jsonStringStream.str();
        return true;
    }

    template<typename T, typename V>
    static void RegisterMapper(V T:: *field, const std::string &name)
    {
        struct_mapping::reg(field, name);
    }
};
