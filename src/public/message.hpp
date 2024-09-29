// commen.hpp
#ifndef COMMEN_H
#define COMMEN_H
#include <string>
#include <vector>
#ifdef __Debug__
#include <iostream>
#define D(x)                         \
    do {                             \
        std::cout << x << std::endl; \
    } while
#else
#define D(x)
#endif

namespace logger {
typedef size_t LogLevel;
typedef size_t RoleId;
class ClientRole;
class Buffer;

struct Message {
    std::string msg;
    size_t timestamp;
    LogLevel level;
    RoleId role_id;
    std::vector<size_t> labelIds;
    Message(){};
    Message(const std::string& msg, size_t timestamp = 0, LogLevel level = 0,
            RoleId role_id = 0, const std::vector<size_t>& labelIds = {})
        : msg(msg),
          timestamp(timestamp),
          level(level),
          role_id(role_id),
          labelIds(labelIds){};
};

}  // namespace logger

#endif  // COMMEN_H