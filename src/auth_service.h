#pragma once
#include <string>

class AuthService {
 public:
  static bool VerifyUser(const std::string& name, const std::string& pwd,
                         bool is_login);
};