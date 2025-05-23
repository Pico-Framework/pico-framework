#include "DashboardView.h"

//const char* DashboardView::dashboard_html = dashboard_html;

DashboardView::DashboardView()
    : StaticView(dashboard_html) {}  // uses TemplateSource::Inline by default
