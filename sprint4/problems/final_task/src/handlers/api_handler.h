#pragma once

#include "web/core.h"
#include "app/app.h"

namespace handlers {

class ApiHandler {
  public:
    ApiHandler(app::Application& app);

    bool IsApiRequest(const web::StringRequest& req);

    web::StringResponse HandleApiRequest(web::StringRequest&& req);

  private:
    app::Application& app_;
    std::string uri_prefix_ = "/api";
};

}  // namespace handlers
