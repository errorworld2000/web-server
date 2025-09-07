#pragma once

#include <string>

#include "http_request.h"
#include "http_response.h"

class HttpRouter {
 public:
  void Route(const HttpRequest& req, HttpResponse& resp,
             const std::string& src_dir);
};
