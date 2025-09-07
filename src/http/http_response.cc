#include "http_response.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

HttpResponse::HttpResponse(int code)
    : code_(code), is_keep_alive_(false), mmFile_(nullptr) {
  mmFileStat_ = {};
}

HttpResponse::~HttpResponse() {
  if (mmFile_) {
    munmap(mmFile_, mmFileStat_.st_size);
    mmFile_ = nullptr;
  }
}

void HttpResponse::SetHeader(const std::string& key, const std::string& value) {
  headers_[key] = value;
}

bool HttpResponse::SetContentFromFile(const std::string& path,
                                      const std::string& src_dir) {
  std::string file_path = src_dir + path;
  if (stat(file_path.c_str(), &mmFileStat_) < 0 ||
      S_ISDIR(mmFileStat_.st_mode)) {
    return false;
  }
  if (!(mmFileStat_.st_mode & S_IROTH)) {
    return false;
  }

  int src_fd = open(file_path.c_str(), O_RDONLY);
  if (src_fd < 0) {
    return false;
  }

  mmFile_ =
      (char*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
  close(src_fd);

  SetHeader("Content-Type", GetFileType(path));
  return true;
}

void HttpResponse::Serialize(std::vector<char>& buff) const {
  // Status line
  std::string status_line =
      "HTTP/1.1 " + std::to_string(code_) + " " + status_text_ + "\r\n";
  buff.insert(buff.end(), status_line.begin(), status_line.end());

  // Headers
  for (const auto& header : headers_) {
    std::string header_line = header.first + ": " + header.second + "\r\n";
    buff.insert(buff.end(), header_line.begin(), header_line.end());
  }

  // Connection header
  std::string conn = "Connection: ";
  if (is_keep_alive_) {
    conn += "keep-alive\r\n";
    conn += "keep-alive: max=6, timeout=120\r\n";
  } else {
    conn += "close\r\n";
  }
  buff.insert(buff.end(), conn.begin(), conn.end());

  // Content-Length
  if (mmFile_) {
    std::string content_len =
        "Content-Length: " + std::to_string(mmFileStat_.st_size) + "\r\n\r\n";
    buff.insert(buff.end(), content_len.begin(), content_len.end());
  } else {
    std::string content_len =
        "Content-Length: " + std::to_string(content_.size()) + "\r\n\r\n";
    buff.insert(buff.end(), content_len.begin(), content_len.end());
    buff.insert(buff.end(), content_.begin(), content_.end());
  }
}

size_t HttpResponse::GetFileLen() const {
  return mmFile_ ? mmFileStat_.st_size : 0;
}

char* HttpResponse::GetFile() const { return mmFile_; }

std::string HttpResponse::GetFileType(const std::string& path) const {
  std::string::size_type idx = path.find_last_of('.');
  if (idx == std::string::npos) {
    return "text/plain";
  }
  std::string suffix = path.substr(idx);
  if (SUFFIX_TYPE.count(suffix) == 1) {
    return SUFFIX_TYPE.find(suffix)->second;
  }
  return "text/plain";
}
