#include "json_utils.h"
#include <stdexcept>

json JsonUtils::parse(const std::string &jsonString)
{
    try
    {
        return json::parse(jsonString);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
    }
}

std::string JsonUtils::serialize(const json &jsonObject)
{
    return jsonObject.dump();
}