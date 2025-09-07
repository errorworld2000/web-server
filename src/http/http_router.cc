#include "http_router.h"

#include "auth_service.h"

void HttpRouter::Route(const HttpRequest& req, HttpResponse& resp,
                       const std::string& src_dir) {
  std::string path = req.Path();
  if (req.GetMethod() == "POST") {
    if (path == "/login" || path == "/register") {
      bool is_login = (path == "/login");
      const std::string& user = req.GetPost("username");
      const std::string& pwd = req.GetPost("password");
      if (AuthService::VerifyUser(user, pwd, is_login)) {
        path = "/welcome.html";
      } else {
        path = "/error.html";
      }
    }
  }

  resp.SetStatusCode(200);
  resp.SetStatusText("OK");
  resp.SetKeepAlive(req.IsKeepAlive());
  if (!resp.SetContentFromFile(path, src_dir)) {
    resp.SetStatusCode(404);
    resp.SetStatusText("Not Found");
    if (!resp.SetContentFromFile("/error.html", src_dir)) {
      // If error.html is also not found, set a default error message.
      resp.SetStatusCode(500);
      resp.SetStatusText("Internal Server Error");
      resp.SetContent("<h1>Internal Server Error</h1>");
    }
  }
}
