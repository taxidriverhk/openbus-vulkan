#pragma once

#include <fstream>
#include <string>

#include "Common/JsonParser.h"

class ConfigReader
{
public:
    template<typename T>
    static bool ReadConfig(const std::string &configFile, T &jsonObject)
    {
        if (!registered)
        {
            RegisterConfigMapping();
            registered = true;
        }

        std::ifstream configFileStream(configFile);
        if (configFileStream.fail())
        {
            return false;
        }

        configFileStream.seekg(0, std::ios::end);
        std::streampos length = configFileStream.tellg();
        configFileStream.seekg(0, std::ios::beg);

        std::vector<char> buffer(length);
        configFileStream.read(buffer.data(), length);

        std::istringstream jsonStringStream(std::string(buffer.begin(), buffer.end()));
        
        return JsonParser::DeserializeJsonString(jsonStringStream, jsonObject);
    }

private:
    static bool registered;
    static void RegisterConfigMapping();
};
