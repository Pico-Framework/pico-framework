#include "UserView.h"

UserView::UserView()
    : HtmlTemplateView(login_html) {}  // uses TemplateSource::Inline by default
