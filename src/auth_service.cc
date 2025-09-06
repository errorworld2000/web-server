#include "auth_service.h"

#include <cassert>
#include <iostream>
#include <mysql/mysql.h>

#include "sqlconnRAII.h"
#include "sqlconnpool.h"

bool AuthService::VerifyUser(const std::string& name, const std::string& pwd,
                             bool isLogin) {
  if (name.empty() || pwd.empty()) return false;

  MYSQL* sql;
  SqlConnRAII(&sql, SqlConnPool::Instance());
  assert(sql);

  bool flag = false;
  MYSQL_RES* res = nullptr;

  char order[256];
  snprintf(order, sizeof(order),
           "SELECT username, password FROM user WHERE username='%s' LIMIT 1",
           name.c_str());

  if (mysql_query(sql, order) == 0) {
    res = mysql_store_result(sql);
    while (MYSQL_ROW row = mysql_fetch_row(res)) {
      std::string password(row[1]);
      if (isLogin) {
        flag = (pwd == password);
      } else {
        // 注册行为，但用户已存在
        flag = false;
      }
    }
    mysql_free_result(res);
  }

  if (!isLogin && flag == true) {
    snprintf(order, sizeof(order),
             "INSERT INTO user(username, password) VALUES('%s','%s')",
             name.c_str(), pwd.c_str());
    if (mysql_query(sql, order) == 0) {
      flag = true;
    } else {
      flag = false;
    }
  }

  SqlConnPool::Instance()->FreeConn(sql);
  return flag;
}
