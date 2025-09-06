#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class HttpResponse {
 public:
  HttpResponse();
  void Init(const std::string& src_dir, const std::string& path,
            bool is_keep_alive, int code);
  bool Serialize(std::vector<char> buff);
  size_t GetFileLen() const;
  char* GetFile() const;

 private:
  static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;

  std::string src_dir_;
  std::string path_;
  bool is_keep_alive_;
  int code_;

  char* mmFile_;
  struct stat mmFileStat_;

  std::string GetFileType();
};