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
};

}  // namespace logger

#endif  // COMMEN_H