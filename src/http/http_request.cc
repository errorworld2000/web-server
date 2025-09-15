#include "http_request.h"

#include <algorithm>
#include <assert.h>
#include <cstddef>

#include "auth_service.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "/register", "/login", "/welcome", "/error"};

void HttpRequest::Clear() {
  method_ = path_ = version_ = body_ = "";
  body_len_ = 0;
  state_ = ParseState::NONE;
  header_.clear();
  post_.clear();
}

bool HttpRequest::Parse(std::vector<char>& buff) {
  if (buff.empty()) return false;

  const char* CRLF = "\r\n";
  size_t parsed_len = 0;

  // --- (Phase 1: Parse Request Line and Headers) ---
  while (state_ != ParseState::BODY && state_ != ParseState::FINISH &&
         state_ != ParseState::ERROR) {
    auto line_end_it =
        std::search(buff.begin() + parsed_len, buff.end(), CRLF, CRLF + 2);

    if (line_end_it == buff.end()) {
      return false;
    }

    std::string line(buff.begin() + parsed_len, line_end_it);
    parsed_len = (line_end_it - buff.begin()) + 2;

    switch (state_) {
      case ParseState::NONE:
        ParseRequestLine(line);
        break;
      case ParseState::HEADER:
        ParseHeader(line);
        break;
      default:
        state_ = ParseState::ERROR;
        break;
    }
  }

  // --- (Phase 2: Parse Request Body) ---
  if (state_ == ParseState::BODY) {
    if (buff.size() - parsed_len < body_len_) {
      return false;
    }
    auto body_end = buff.begin() + parsed_len + body_len_;
    ParseBody(std::string(buff.begin() + parsed_len, body_end));
    parsed_len += body_len_;
  }

  // --- 阶段3: 清理缓冲区 (Phase 3: Clean up buffer) ---
  if (parsed_len > 0) {
    buff.erase(buff.begin(), buff.begin() + parsed_len);
  }

  return state_ == ParseState::FINISH;
}

bool HttpRequest::IsKeepAlive() const {
  if (version_ == "1.1") {
    if (header_.count("Connection") && (header_.at("Connection") == "close" ||
                                        header_.at("Connection") == "Close")) {
      return false;
    }
    return true;
  } else {
    return header_.count("Connection") &&
           (header_.at("Connection") == "keep-alive" ||
            header_.at("Connection") == "Keep-Alive");
  }
}

std::string HttpRequest::GetHeader(const std::string& key) const {
  if (header_.count(key)) {
    return header_.at(key);
  }
  return "";
}

std::string HttpRequest::GetPost(const std::string& key) const {
  if (post_.count(key)) {
    return post_.at(key);
  }
  return "";
}

int HttpRequest::ConverHex(char ch) {
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= '0' && ch <= '9') return ch - '0';
  return ch;
}

bool HttpRequest::ParseRequestLine(const std::string& line) {
  size_t method_end = line.find(' ');
  if (method_end == std::string::npos) {
    state_ = ParseState::ERROR;
    return false;
  }
  method_ = line.substr(0, method_end);

  size_t path_start = line.find_first_not_of(' ', method_end);
  if (path_start == std::string::npos) {
    state_ = ParseState::ERROR;
    return false;
  }

  size_t path_end = line.find(' ', path_start);
  if (path_end == std::string::npos) {
    state_ = ParseState::ERROR;
    return false;
  }
  path_ = line.substr(path_start, path_end - path_start);

  size_t version_start = line.find_first_not_of(' ', path_end);
  if (version_start == std::string::npos ||
      line.substr(version_start, 5) != "HTTP/") {
    state_ = ParseState::ERROR;
    return false;
  }
  version_ = line.substr(version_start + 5);

  state_ = ParseState::HEADER;
  if (path_ == "/") {
    path_ = "/index.html";
  } else {
    for (auto& item : DEFAULT_HTML) {
      if (item == path_) {
        path_ += ".html";
        break;
      }
    }
  }
  return true;
}

void HttpRequest::ParseHeader(const std::string& line) {
  if (line.empty()) {
    if (header_.count("Content-Length") &&
        std::stoi(header_["Content-Length"]) > 0) {
      body_len_ = std::stoi(header_["Content-Length"]);
      state_ = ParseState::BODY;
    } else {
      state_ = ParseState::FINISH;
    }
  } else {
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      std::string key = line.substr(0, colon_pos);
      size_t key_end = key.find_last_not_of(" \t");
      if (key_end != std::string::npos) {
        key.resize(key_end + 1);
      }

      std::string value = line.substr(colon_pos + 1);
      size_t value_start = value.find_first_not_of(" \t");
      if (value_start != std::string::npos) {
        value = value.substr(value_start);
      }
      header_[key] = value;
    } else {
      state_ = ParseState::ERROR;
    }
  }
}

void HttpRequest::ParseBody(const std::string& line) {
  body_ = line;
  if (GetHeader("Content-Type") == "application/x-www-form-urlencoded") {
    ParseFromUrlencoded();
  }
  state_ = ParseState::FINISH;
}

void HttpRequest::ParseFromUrlencoded() {
  if (body_.empty()) return;

  std::string key, value;
  int state = 0;  // 0 for key, 1 for value
  std::string current_field;

  for (size_t i = 0; i < body_.size(); ++i) {
    char ch = body_[i];
    if (ch == '=') {
      key = UrlDecode(current_field);  // Decode the extracted key
      current_field.clear();
    } else if (ch == '&') {
      value = UrlDecode(current_field);  // Decode the extracted value
      post_[key] = value;
      current_field.clear();
    } else {
      current_field += ch;
    }
  }
  // Handle the last pair
  if (!current_field.empty()) {
    value = UrlDecode(current_field);
    post_[key] = value;
  }
}

std::string HttpRequest::UrlDecode(const std::string& str) {
  std::string decoded_str;
  char ch;
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '+') {
      decoded_str += ' ';
    } else if (str[i] == '%') {
      assert(i + 2 < str.length());
      int high = ConverHex(str[i + 1]);
      int low = ConverHex(str[i + 2]);
      ch = high * 16 + low;
      decoded_str += ch;
      i += 2;
    } else {
      decoded_str += str[i];
    }
  }
  return decoded_str;
}
