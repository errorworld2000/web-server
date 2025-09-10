#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class ParseState {
  NONE,
  HEADER,
  BODY,
  FINISH,
  ERROR,
};

class HttpRequest {
 public:
  void Clear();
  bool Parse(std::vector<char>& buff);
  bool IsKeepAlive() const;
  std::string Path() const { return path_; }
  std::string GetMethod() const { return method_; }
  std::string GetHeader(const std::string& key) const;
  std::string GetPost(const std::string& key) const;
  bool IsValid() const { return state_ != ParseState::ERROR; }

 private:
  static const std::unordered_set<std::string> DEFAULT_HTML;

  std::string method_, path_, version_, body_;
  int body_len_;
  ParseState state_;
  std::unordered_map<std::string, std::string> header_;
  std::unordered_map<std::string, std::string> post_;

  static int ConverHex(char ch);
  static std::string UrlDecode(const std::string& str);

  bool ParseRequestLine(const std::string& line);
  void ParseHeader(const std::string& line);
  void ParseBody(const std::string& line);
  void ParseFromUrlencoded();
};