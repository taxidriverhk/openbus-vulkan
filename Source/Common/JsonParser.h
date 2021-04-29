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
        struct_mapping::map_json_to_struct(parsedObject, jsonStringStream);
        return true;
    }
    
    template<typename T>
    static bool SerializeJsonString(const T &jsonObject, std::string &serializedString)
    {
        std::ostringstream jsonStringStream;
        struct_mapping::map_struct_to_json(jsonObject, jsonStringStream);
        serializedString = jsonStringStream.str();
        return true;
    }

    template<typename T, typename V>
    static void RegisterMapper(V T:: *field, const std::string &name)
    {
        struct_mapping::reg(field, name);
    }
};
