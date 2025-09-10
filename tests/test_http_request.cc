#include <gtest/gtest.h>

#include "http/http_request.h"

TEST(HttpRequestTest, ParseGetRequest) {
  HttpRequest request;
  request.Clear();
  std::string raw_request_str =
      "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
  std::vector<char> buffer(raw_request_str.begin(), raw_request_str.end());

  ASSERT_TRUE(request.Parse(buffer));
  EXPECT_EQ(request.Path(), "/index.html");
  EXPECT_TRUE(request.IsValid());
}
