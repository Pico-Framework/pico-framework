#include "DashboardView.h"

//const char* DashboardView::dashboard_html = dashboard_html;

DashboardView::DashboardView()
    : HtmlTemplateView(dashboard_html) {}  // uses TemplateSource::Inline by default
