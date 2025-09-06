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

HttpResponse::HttpResponse() {}

void HttpResponse::Init(const std::string& src_dir, const std::string& path,
                        bool is_keep_alive, int code) {
  if (mmFile_) {
    munmap(mmFile_, mmFileStat_.st_size);
    mmFile_ = nullptr;
  }
  mmFileStat_ = {};

  src_dir_ = src_dir;
  path_ = path;
  is_keep_alive_ = is_keep_alive;
  code_ = code;
}

bool HttpResponse::Serialize(std::vector<char> buff) {
  std::string status;
  if (stat((src_dir_ + path_).data(), &mmFileStat_) < 0 ||
      S_ISDIR(mmFileStat_.st_mode)) {
    code_ = 404;
    status = "Not Found";
  } else if (!(mmFileStat_.st_mode & S_IROTH)) {
    code_ = 403;
    status = "Forbidden";
  } else if (code_ == -1) {
    code_ = 200;
    status = "OK";
  }
  path_ = "/" + std::to_string(code_) + ".html";

  // add state-line
  buff.emplace_back("HTTP/1.1 " + std::to_string(code_) + " " + status +
                    "\r\n");

  // add header
  buff.emplace_back("Connection: ");
  if (is_keep_alive_) {
    buff.emplace_back("keep-alive\r\n");
    buff.emplace_back("keep-alive: max=6, timeout=120\r\n");
  } else {
    buff.emplace_back("close\r\n");
  }
  buff.emplace_back("Content-type: " + GetFileType() + "\r\n");

  // add content
  int src_fd = open((src_dir_ + path_).data(), O_RDONLY);
  int* mm_ret =
      (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
  mmFile_ = (char*)mm_ret;
  close(src_fd);
  buff.emplace_back("Content-length: " + std::to_string(mmFileStat_.st_size) +
                    "\r\n\r\n");
}

size_t HttpResponse::GetFileLen() const { return mmFileStat_.st_size; }

char* HttpResponse::GetFile() const { return mmFile_; }

std::string HttpResponse::GetFileType() {
  std::string::size_type idx = path_.find_last_of('.');
  if (idx == std::string::npos) {
    return "text/plain";
  }
  std::string suffix = path_.substr(idx);
  if (SUFFIX_TYPE.count(suffix) == 1) {
    return SUFFIX_TYPE.find(suffix)->second;
  }
  return "text/plain";
}