// commen.hpp
#ifndef COMMEN_H
#define COMMEN_H
#include <string>
#include <vector>

namespace logger {
typedef size_t LogLevel;
typedef size_t RoleId;
class ClientRole;
class Buffer;

struct Message {
    std::string msg;
    size_t timestamp;
    LogLevel level;
    RoleId role;
    std::vector<size_t> labelIds;
};

}  // namespace logger

#endif  // COMMEN_H