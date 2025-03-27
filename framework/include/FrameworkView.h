#pragma once
#include <string>
#include <map>

class FrameworkView {
public:
    static std::string render(const std::string& templatePath, const std::map<std::string, std::string>& context);
};
