/**
 * @file JsonView.cpp
 * @brief Implementation of JsonView for rendering JSON responses.
 */

//  void App::handleStatus(HttpRequest& req, HttpResponse& res) {
//     auto json = model.toJson();
//     JsonView view(json);
//     res.send(view);  // or res.send(view, {}); if you prefer consistency
// }

#include "framework/JsonView.h"

JsonView::JsonView(const nlohmann::json &payload)
    : payload_(payload) {}

std::string JsonView::render(const std::map<std::string, std::string> &) const
{
    return payload_.dump(2); // pretty-print
}

std::string JsonView::getContentType() const
{
    return "application/json";
}
