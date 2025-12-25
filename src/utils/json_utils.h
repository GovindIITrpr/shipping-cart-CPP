#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JsonUtils {
public:
    static json parse(const std::string& jsonString);
    static std::string serialize(const json& jsonObject);
};

#endif // JSON_UTILS_H