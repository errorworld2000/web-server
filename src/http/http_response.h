#pragma once

#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class HttpResponse {
 public:
  explicit HttpResponse(int code = -1);
  ~HttpResponse();

  void SetKeepAlive(bool on) { is_keep_alive_ = on; }
  void SetStatusCode(int code) { code_ = code; }
  void SetStatusText(const std::string& text) { status_text_ = text; }
  void SetContent(const std::string& content) { content_ = content; }
  void SetHeader(const std::string& key, const std::string& value);
  bool SetContentFromFile(const std::string& path, const std::string& src_dir);

  void Serialize(std::vector<char>& buff) const;
  size_t GetFileLen() const;
  char* GetFile() const;

 private:
  static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;

  int code_;
  std::string status_text_;
  std::string content_;
  std::unordered_map<std::string, std::string> headers_;
  bool is_keep_alive_;

  char* mmFile_;
  struct stat mmFileStat_;

  std::string GetFileType(const std::string& path) const;
};