#include "View.h"
#include <fstream>
#include <sstream>

static std::string loadFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string View::render(const std::string& templatePath, const std::map<std::string, std::string>& context) {
    std::string tpl = loadFile(templatePath);
    for (const auto& [key, value] : context) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = tpl.find(placeholder, pos)) != std::string::npos) {
            tpl.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return tpl;
}

// Usage
// void handleHome(Request& req, Response& res) {
//     std::map<std::string, std::string> context = {
//         {"name", "Alice"},
//         {"time", getCurrentTimeString()}
//     };

//     std::string html = View::render("/www/index.html", context);
//     res.send(html, "text/html");
// }
